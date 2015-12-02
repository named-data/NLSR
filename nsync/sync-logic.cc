/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *         Yingdi Yu <yingdi@cs.ucla.edu>
 */

#include "sync-logic.h"
#include "sync-diff-leaf.h"
#include "sync-full-leaf.h"
#include "sync-logging.h"
#include "sync-state.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <vector>

using namespace std;

using ndn::Data;
using ndn::EventId;
using ndn::KeyChain;
using ndn::Face;
using ndn::Name;
using ndn::OnDataValidated;
using ndn::OnDataValidationFailed;
using ndn::Validator;

INIT_LOGGER ("SyncLogic");


#ifdef _DEBUG
#define _LOG_DEBUG_ID(v) _LOG_DEBUG(m_instanceId << " " << v)
#else
#define _LOG_DEBUG_ID(v) _LOG_DEBUG(v)
#endif

#define GET_RANDOM(var) var ()

#define TIME_SECONDS_WITH_JITTER(sec) \
  (ndn::time::seconds(sec) + ndn::time::milliseconds(GET_RANDOM (m_reexpressionJitter)))

#define TIME_MILLISECONDS_WITH_JITTER(ms) \
  (ndn::time::seconds(ms) + ndn::time::milliseconds(GET_RANDOM (m_reexpressionJitter)))

namespace Sync {

using ndn::shared_ptr;

int SyncLogic::m_instanceCounter = 0;

const int SyncLogic::m_syncResponseFreshness = 1000; // MUST BE dividable by 1000!!!
const int SyncLogic::m_syncInterestReexpress = 4; // seconds

SyncLogic::SyncLogic (const Name& syncPrefix,
                      shared_ptr<Validator> validator,
                      shared_ptr<Face> face,
                      LogicUpdateCallback onUpdate,
                      LogicRemoveCallback onRemove)
  : m_state (new FullState)
  , m_syncInterestTable (face->getIoService(), ndn::time::seconds(m_syncInterestReexpress))
  , m_syncPrefix (syncPrefix)
  , m_onUpdate (onUpdate)
  , m_onRemove (onRemove)
  , m_perBranch (false)
  , m_validator(validator)
  , m_keyChain(new KeyChain())
  , m_face(face)
  , m_scheduler(face->getIoService())
  , m_randomGenerator (static_cast<unsigned int> (std::time (0)))
  , m_rangeUniformRandom (m_randomGenerator, boost::uniform_int<> (200,1000))
  , m_reexpressionJitter (m_randomGenerator, boost::uniform_int<> (100,500))
  , m_recoveryRetransmissionInterval (m_defaultRecoveryRetransmitInterval)
{
  m_syncRegisteredPrefixId = m_face->setInterestFilter (m_syncPrefix,
                                                        bind(&SyncLogic::onSyncInterest,
                                                             this, _1, _2),
                                                        bind(&SyncLogic::onSyncRegisterSucceed,
                                                             this, _1),
                                                        bind(&SyncLogic::onSyncRegisterFailed,
                                                             this, _1, _2));


  m_reexpressingInterestId = m_scheduler.scheduleEvent (ndn::time::seconds (0),
                                                        bind (&SyncLogic::sendSyncInterest, this));

  m_instanceId = string("Instance " + boost::lexical_cast<string>(m_instanceCounter++) + " ");
}

SyncLogic::SyncLogic (const Name& syncPrefix,
                      shared_ptr<Validator> validator,
                      shared_ptr<Face> face,
                      LogicPerBranchCallback onUpdateBranch)
  : m_state (new FullState)
  , m_syncInterestTable (face->getIoService(), ndn::time::seconds (m_syncInterestReexpress))
  , m_syncPrefix (syncPrefix)
  , m_onUpdateBranch (onUpdateBranch)
  , m_perBranch(true)
  , m_validator(validator)
  , m_keyChain(new KeyChain())
  , m_face(face)
  , m_scheduler(face->getIoService())
  , m_randomGenerator (static_cast<unsigned int> (std::time (0)))
  , m_rangeUniformRandom (m_randomGenerator, boost::uniform_int<> (200,1000))
  , m_reexpressionJitter (m_randomGenerator, boost::uniform_int<> (100,500))
  , m_recoveryRetransmissionInterval (m_defaultRecoveryRetransmitInterval)
{
  m_syncRegisteredPrefixId = m_face->setInterestFilter (m_syncPrefix,
                                                        bind(&SyncLogic::onSyncInterest,
                                                             this, _1, _2),
                                                        bind(&SyncLogic::onSyncRegisterSucceed,
                                                             this, _1),
                                                        bind(&SyncLogic::onSyncRegisterFailed,
                                                             this, _1, _2));

  m_reexpressingInterestId = m_scheduler.scheduleEvent (ndn::time::seconds (0),
                                                        bind (&SyncLogic::sendSyncInterest, this));
}

SyncLogic::~SyncLogic ()
{
  m_face->unsetInterestFilter(m_syncRegisteredPrefixId);
  m_scheduler.cancelEvent (m_reexpressingInterestId);
  m_scheduler.cancelEvent (m_delayedInterestProcessingId);
}

/**
 * Two types of intersts
 *
 * Normal name:    .../<hash>
 * Recovery name:  .../recovery/<hash>
 */
tuple<DigestConstPtr, std::string>
SyncLogic::convertNameToDigestAndType (const Name &name)
{
  BOOST_ASSERT (m_syncPrefix.isPrefixOf(name));

  int nameLengthDiff = name.size() - m_syncPrefix.size();
  BOOST_ASSERT (nameLengthDiff > 0);
  BOOST_ASSERT (nameLengthDiff < 3);

  string hash = name.get(-1).toUri();
  string interestType;

  if(nameLengthDiff == 1)
    interestType = "normal";
  else
    interestType = name.get(-2).toUri();

  _LOG_DEBUG_ID (hash << ", " << interestType);

  DigestPtr digest = make_shared<Digest> ();
  istringstream is (hash);
  is >> *digest;

  return make_tuple(digest, interestType);
}

void
SyncLogic::onSyncInterest (const Name& prefix, const ndn::Interest& interest)
{
  Name name = interest.getName();

  _LOG_DEBUG_ID("respondSyncInterest: " << name);

  try
    {
      _LOG_DEBUG_ID ("<< I " << name);

      DigestConstPtr digest;
      string type;
      tie (digest, type) = convertNameToDigestAndType (name);

      if (type == "normal") // kind of ineffective...
        {
          processSyncInterest (name, digest);
        }
      else if (type == "recovery")
        {
          processSyncRecoveryInterest (name, digest);
        }
    }
  catch (Error::DigestCalculationError &e)
    {
      _LOG_DEBUG_ID ("Something fishy happened...");
      // log error. ignoring it for now, later we should log it
      return ;
    }
}

void
SyncLogic::onSyncRegisterSucceed(const Name& prefix)
{
  _LOG_DEBUG_ID("Sync prefix registration succeeded! " << prefix);
}

void
SyncLogic::onSyncRegisterFailed(const Name& prefix, const string& msg)
{
  _LOG_DEBUG_ID("Sync prefix registration failed! " << msg);
}

void
SyncLogic::onSyncData(const ndn::Interest& interest, Data& data)
{
  OnDataValidated onValidated = bind(&SyncLogic::onSyncDataValidated, this, _1);
  OnDataValidationFailed onValidationFailed = bind(&SyncLogic::onSyncDataValidationFailed, this, _1);
  m_validator->validate(data, onValidated, onValidationFailed);
}

void
SyncLogic::onSyncTimeout(const ndn::Interest& interest)
{
  // It is OK. Others will handle the time out situation.
}

void
SyncLogic::onSyncDataValidationFailed(const shared_ptr<const Data>& data)
{
  _LOG_DEBUG_ID("Sync data cannot be verified!");
}

void
SyncLogic::onSyncDataValidated(const shared_ptr<const Data>& data)
{
  Name name = data->getName();
  const char* wireData = (const char*)data->getContent().value();
  size_t len = data->getContent().value_size();

  try
    {
      _LOG_DEBUG_ID ("<< D " << name);

      DigestConstPtr digest;
      string type;
      tie (digest, type) = convertNameToDigestAndType (name);

      if (type == "normal")
        {
          processSyncData (name, digest, wireData, len);
        }
      else
        {
          // timer is always restarted when we schedule recovery
          m_scheduler.cancelEvent (m_reexpressingRecoveryInterestId);
          processSyncData (name, digest, wireData, len);
        }
    }
  catch (Error::DigestCalculationError &e)
    {
      _LOG_DEBUG_ID ("Something fishy happened...");
      // log error. ignoring it for now, later we should log it
      return;
    }
}

void
SyncLogic::processSyncInterest (const Name &name, DigestConstPtr digest,
                                bool timedProcessing/*=false*/)
{
  _LOG_DEBUG_ID("processSyncInterest");
  DigestConstPtr rootDigest;
  {
    rootDigest = m_state->getDigest();
  }

  // Special case when state is not empty and we have received request with zero-root digest
  if (digest->isZero () && !rootDigest->isZero ())
    {

      SyncStateMsg ssm;
      {
        ssm << (*m_state);
      }
      sendSyncData (name, digest, ssm);
      return;
    }

  if (*rootDigest == *digest)
    {
      _LOG_DEBUG_ID ("processSyncInterest (): Same state. Adding to PIT");
      m_syncInterestTable.insert (digest, name.toUri(), false);
      return;
    }

  DiffStateContainer::iterator stateInDiffLog = m_log.find (digest);

  if (stateInDiffLog != m_log.end ())
  {
    DiffStateConstPtr stateDiff = (*stateInDiffLog)->diff ();

    sendSyncData (name, digest, stateDiff);
    return;
  }

  if (!timedProcessing)
    {
      bool exists = m_syncInterestTable.insert (digest, name.toUri(), true);
      if (exists) // somebody else replied, so restart random-game timer
        {
          _LOG_DEBUG_ID("Unknown digest, but somebody may have already replied, " <<
                        "so restart our timer");
          m_scheduler.cancelEvent (m_delayedInterestProcessingId);
        }

      uint32_t waitDelay = GET_RANDOM (m_rangeUniformRandom);
      _LOG_DEBUG_ID("Digest is not in the log. Schedule processing after small delay: " <<
                    ndn::time::milliseconds (waitDelay));

      m_delayedInterestProcessingId =
        m_scheduler.scheduleEvent(ndn::time::milliseconds (waitDelay),
                                  bind (&SyncLogic::processSyncInterest, this, name, digest, true));
    }
  else
    {
      _LOG_DEBUG_ID("                                                      (timed processing)");

      m_recoveryRetransmissionInterval = m_defaultRecoveryRetransmitInterval;
      sendSyncRecoveryInterests (digest);
    }
}

void
SyncLogic::processSyncData (const Name &name, DigestConstPtr digest,
                            const char *wireData, size_t len)
{
  DiffStatePtr diffLog = make_shared<DiffState> ();
  bool ownInterestSatisfied = false;

  try
    {

      m_syncInterestTable.remove (name.toUri()); // Remove satisfied interest from PIT

      ownInterestSatisfied = (name == m_outstandingInterestName);

      DiffState diff;
      SyncStateMsg msg;
      if (!msg.ParseFromArray(wireData, len) || !msg.IsInitialized())
      {
        //Throw
        BOOST_THROW_EXCEPTION (Error::SyncStateMsgDecodingFailure () );
      }
      msg >> diff;

      vector<MissingDataInfo> v;
      BOOST_FOREACH (LeafConstPtr leaf, diff.getLeaves().get<ordered>())
        {
          DiffLeafConstPtr diffLeaf = dynamic_pointer_cast<const DiffLeaf> (leaf);
          BOOST_ASSERT (diffLeaf != 0);

          NameInfoConstPtr info = diffLeaf->getInfo();
          if (diffLeaf->getOperation() == UPDATE)
            {
              SeqNo seq = diffLeaf->getSeq();

              bool inserted = false;
              bool updated = false;
              SeqNo oldSeq;
              {
                tie (inserted, updated, oldSeq) = m_state->update (info, seq);
              }

              if (inserted || updated)
                {
                  diffLog->update (info, seq);
                  if (!oldSeq.isValid())
                  {
                    oldSeq = SeqNo(seq.getSession(), 0);
                  }
                  else
                  {
                    ++oldSeq;
                  }
                  // there is no need for application to process update on forwarder node
                  if (info->toString() != forwarderPrefix)
                  {
                    MissingDataInfo mdi = {info->toString(), oldSeq, seq};
                    {
                      ostringstream interestName;
                      interestName << mdi.prefix <<
                        "/" << mdi.high.getSession() <<
                        "/" << mdi.high.getSeq();
                      _LOG_DEBUG_ID("+++++++++++++++ " + interestName.str());
                    }
                    if (m_perBranch)
                    {
                       ostringstream interestName;
                       interestName << mdi.prefix <<
                         "/" << mdi.high.getSession() <<
                         "/" << mdi.high.getSeq();
                       m_onUpdateBranch(interestName.str());
                    }
                    else
                    {
                      v.push_back(mdi);
                    }
                  }
                }
            }
          else if (diffLeaf->getOperation() == REMOVE)
            {
              if (m_state->remove (info))
                {
                  diffLog->remove (info);
                  if (!m_perBranch)
                  {
                    m_onRemove (info->toString ());
                  }
                }
            }
          else
            {
            }
        }

      if (!v.empty())
      {
        if (!m_perBranch)
        {
           m_onUpdate(v);
        }
      }

      insertToDiffLog (diffLog);
    }
  catch (Error::SyncStateMsgDecodingFailure &e)
    {
      _LOG_DEBUG_ID ("Something really fishy happened during state decoding " <<
                  diagnostic_information (e));
      diffLog.reset ();
      // don't do anything
    }

  if ((diffLog != 0 && diffLog->getLeaves ().size () > 0) ||
      ownInterestSatisfied)
    {
      _LOG_DEBUG_ID(" +++++++++++++++ state changed!!!");
      // Do it only if everything went fine and state changed

      // this is kind of wrong
      // satisfyPendingSyncInterests (diffLog); // if there are interests in PIT,
      //                                        // there is a point to satisfy them using new state

      // if state has changed, then it is safe to express a new interest
      ndn::time::system_clock::Duration after =
        ndn::time::milliseconds(GET_RANDOM (m_reexpressionJitter));
      // cout << "------------ reexpress interest after: " << after << endl;
      EventId eventId = m_scheduler.scheduleEvent (after,
                                                   bind (&SyncLogic::sendSyncInterest, this));

      m_scheduler.cancelEvent (m_reexpressingInterestId);
      m_reexpressingInterestId = eventId;
    }
}

void
SyncLogic::processSyncRecoveryInterest (const Name &name, DigestConstPtr digest)
{
  _LOG_DEBUG_ID("processSyncRecoveryInterest");
  DiffStateContainer::iterator stateInDiffLog = m_log.find (digest);

  if (stateInDiffLog == m_log.end ())
    {
      _LOG_DEBUG_ID ("Could not find " << *digest << " in digest log");
      return;
    }

  SyncStateMsg ssm;
  {
    ssm << (*m_state);
  }
  sendSyncData (name, digest, ssm);
}

void
SyncLogic::satisfyPendingSyncInterests (DiffStateConstPtr diffLog)
{
  DiffStatePtr fullStateLog = make_shared<DiffState> ();
  {
    BOOST_FOREACH (LeafConstPtr leaf, m_state->getLeaves ()/*.get<timed> ()*/)
      {
        fullStateLog->update (leaf->getInfo (), leaf->getSeq ());
        /// @todo Impose limit on how many state info should be send out
      }
  }

  try
    {
      uint32_t counter = 0;
      while (m_syncInterestTable.size () > 0)
        {
          Sync::Interest interest = m_syncInterestTable.pop ();

          if (!interest.m_unknown)
            {
              _LOG_DEBUG_ID (">> D " << interest.m_name);
              sendSyncData (interest.m_name, interest.m_digest, diffLog);
            }
          else
            {
              _LOG_DEBUG_ID (">> D (unknown)" << interest.m_name);
              sendSyncData (interest.m_name, interest.m_digest, fullStateLog);
            }
          counter ++;
        }
      _LOG_DEBUG_ID ("Satisfied " << counter << " pending interests");
    }
  catch (Error::InterestTableIsEmpty &e)
    {
      // ok. not really an error
    }
}

void
SyncLogic::insertToDiffLog (DiffStatePtr diffLog)
{
  diffLog->setDigest (m_state->getDigest());
  if (m_log.size () > 0)
    {
      m_log.get<sequenced> ().front ()->setNext (diffLog);
    }
  m_log.erase (m_state->getDigest()); // remove diff state with the same digest.
                                      // next pointers are still valid
  /// @todo Optimization
  m_log.get<sequenced> ().push_front (diffLog);
}

void
SyncLogic::addLocalNames (const Name &prefix, uint64_t session, uint64_t seq)
{
  DiffStatePtr diff;
  {
    //cout << "Add local names" <<endl;
    NameInfoConstPtr info = StdNameInfo::FindOrCreate(prefix.toUri());

    _LOG_DEBUG_ID ("addLocalNames (): old state " << *m_state->getDigest ());

    SeqNo seqN (session, seq);
    m_state->update(info, seqN);

    _LOG_DEBUG_ID ("addLocalNames (): new state " << *m_state->getDigest ());

    diff = make_shared<DiffState>();
    diff->update(info, seqN);
    insertToDiffLog (diff);
  }

  // _LOG_DEBUG_ID ("PIT size: " << m_syncInterestTable.size ());
  satisfyPendingSyncInterests (diff);
}

void
SyncLogic::remove(const Name &prefix)
{
  DiffStatePtr diff;
  {
    NameInfoConstPtr info = StdNameInfo::FindOrCreate(prefix.toUri());
    m_state->remove(info);

    // increment the sequence number for the forwarder node
    NameInfoConstPtr forwarderInfo = StdNameInfo::FindOrCreate(forwarderPrefix);

    LeafContainer::iterator item = m_state->getLeaves ().find (forwarderInfo);
    SeqNo seqNo (0);
    if (item != m_state->getLeaves ().end ())
      {
        seqNo = (*item)->getSeq ();
        ++seqNo;
      }
    m_state->update (forwarderInfo, seqNo);

    diff = make_shared<DiffState>();
    diff->remove(info);
    diff->update(forwarderInfo, seqNo);

    insertToDiffLog (diff);
  }

  satisfyPendingSyncInterests (diff);
}

void
SyncLogic::sendSyncInterest ()
{
  _LOG_DEBUG_ID("sendSyncInterest");

  {
    m_outstandingInterestName = m_syncPrefix;
    ostringstream os;
    os << *m_state->getDigest();
    m_outstandingInterestName.append(os.str());
    _LOG_DEBUG_ID (">> I " << m_outstandingInterestName);
  }

  _LOG_DEBUG_ID("sendSyncInterest: " << m_outstandingInterestName);

  EventId eventId =
    m_scheduler.scheduleEvent(ndn::time::seconds(m_syncInterestReexpress) +
                              ndn::time::milliseconds(GET_RANDOM(m_reexpressionJitter)),
                              bind (&SyncLogic::sendSyncInterest, this));
  m_scheduler.cancelEvent (m_reexpressingInterestId);
  m_reexpressingInterestId = eventId;

  ndn::Interest interest(m_outstandingInterestName);
  interest.setMustBeFresh(true);

  m_face->expressInterest(interest,
                          bind(&SyncLogic::onSyncData, this, _1, _2),
                          bind(&SyncLogic::onSyncTimeout, this, _1));
}

void
SyncLogic::sendSyncRecoveryInterests (DigestConstPtr digest)
{
  ostringstream os;
  os << *digest;

  Name interestName = m_syncPrefix;
  interestName.append("recovery").append(os.str());

  ndn::time::system_clock::Duration nextRetransmission =
    ndn::time::milliseconds(m_recoveryRetransmissionInterval + GET_RANDOM (m_reexpressionJitter));

  m_recoveryRetransmissionInterval <<= 1;

  m_scheduler.cancelEvent (m_reexpressingRecoveryInterestId);
  if (m_recoveryRetransmissionInterval < 100*1000) // <100 seconds
    m_reexpressingRecoveryInterestId =
      m_scheduler.scheduleEvent (nextRetransmission,
                                 bind (&SyncLogic::sendSyncRecoveryInterests, this, digest));

  ndn::Interest interest(interestName);
  interest.setMustBeFresh(true);

  m_face->expressInterest(interest,
                          bind(&SyncLogic::onSyncData, this, _1, _2),
                          bind(&SyncLogic::onSyncTimeout, this, _1));
}


void
SyncLogic::sendSyncData (const Name &name, DigestConstPtr digest, StateConstPtr state)
{
  SyncStateMsg msg;
  msg << (*state);
  sendSyncData(name, digest, msg);
}

// pass in state msg instead of state, so that there is no need to lock the state until
// this function returns
void
SyncLogic::sendSyncData (const Name &name, DigestConstPtr digest, SyncStateMsg &ssm)
{
  _LOG_DEBUG_ID (">> D " << name);
  int size = ssm.ByteSize();
  char *wireData = new char[size];
  ssm.SerializeToArray(wireData, size);

  //Data syncData(name);
  ndn::shared_ptr<ndn::Data> syncData = ndn::make_shared<ndn::Data>();
  syncData->setName(name);
  syncData->setContent(reinterpret_cast<const uint8_t*>(wireData), size);
  syncData->setFreshnessPeriod(ndn::time::seconds(m_syncResponseFreshness));

  m_keyChain->sign(*syncData);

  m_face->put(*syncData);

  delete []wireData;

  // checking if our own interest got satisfied
  bool satisfiedOwnInterest = false;
  {
    satisfiedOwnInterest = (m_outstandingInterestName == name);
  }

  if (satisfiedOwnInterest)
    {
      _LOG_DEBUG_ID ("Satisfied our own Interest. Re-expressing (hopefully with a new digest)");

      ndn::time::system_clock::Duration after =
        ndn::time::milliseconds(GET_RANDOM(m_reexpressionJitter));
      // cout << "------------ reexpress interest after: " << after << endl;
      EventId eventId = m_scheduler.scheduleEvent (after,
                                                   bind (&SyncLogic::sendSyncInterest, this));
      m_scheduler.cancelEvent (m_reexpressingInterestId);
      m_reexpressingInterestId = eventId;
    }
}

string
SyncLogic::getRootDigest()
{
  ostringstream os;
  os << *m_state->getDigest();
  return os.str();
}

size_t
SyncLogic::getNumberOfBranches () const
{
  return m_state->getLeaves ().size ();
}

void
SyncLogic::printState () const
{
  BOOST_FOREACH (const shared_ptr<Sync::Leaf> leaf, m_state->getLeaves ())
    {
      std::cout << *leaf << std::endl;
    }
}

std::map<std::string, bool>
SyncLogic::getBranchPrefixes() const
{
  std::map<std::string, bool> m;

  BOOST_FOREACH (const shared_ptr<Sync::Leaf> leaf, m_state->getLeaves ())
    {
      std::string prefix = leaf->getInfo()->toString();
      // do not return forwarder prefix
      if (prefix != forwarderPrefix)
      {
        m.insert(pair<std::string, bool>(prefix, false));
      }
    }

  return m;
}

}
