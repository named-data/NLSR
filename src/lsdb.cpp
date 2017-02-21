/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
 *
 * This file is part of NLSR (Named-data Link State Routing).
 * See AUTHORS.md for complete list of NLSR authors and contributors.
 *
 * NLSR is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "lsdb.hpp"

#include "logger.hpp"
#include "nlsr.hpp"
#include "publisher/segment-publisher.hpp"
#include "utility/name-helper.hpp"

#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/util/segment-fetcher.hpp>

#include <string>

namespace nlsr {

INIT_LOGGER("Lsdb");

class LsaContentPublisher : public SegmentPublisher<ndn::Face>
{
public:
  LsaContentPublisher(ndn::Face& face,
                      ndn::KeyChain& keyChain,
                      const ndn::time::milliseconds& freshnessPeriod,
                      const std::string& content)
    : SegmentPublisher(face, keyChain, freshnessPeriod)
    , m_content(content)
  {
  }

  virtual size_t
  generate(ndn::EncodingBuffer& outBuffer) {
    size_t totalLength = 0;
    totalLength += outBuffer.prependByteArray(reinterpret_cast<const uint8_t*>(m_content.c_str()),
                                              m_content.size());
    return totalLength;
  }

private:
  const std::string m_content;
};

const ndn::Name::Component Lsdb::NAME_COMPONENT = ndn::Name::Component("lsdb");
const ndn::time::seconds Lsdb::GRACE_PERIOD = ndn::time::seconds(10);
const steady_clock::TimePoint Lsdb::DEFAULT_LSA_RETRIEVAL_DEADLINE = steady_clock::TimePoint::min();

Lsdb::Lsdb(Nlsr& nlsr, ndn::Scheduler& scheduler, SyncLogicHandler& sync)
  : m_nlsr(nlsr)
  , m_scheduler(scheduler)
  , m_sync(sync)
  , m_lsaRefreshTime(0)
  , m_adjLsaBuildInterval(ADJ_LSA_BUILD_INTERVAL_DEFAULT)
{
}

void
Lsdb::onFetchLsaError(uint32_t errorCode,
                      const std::string& msg,
                      ndn::Name& interestName,
                      uint32_t retransmitNo,
                      const ndn::time::steady_clock::TimePoint& deadline,
                      ndn::Name lsaName,
                      uint64_t seqNo)
{
  _LOG_DEBUG("Failed to fetch LSA: " << lsaName << ", Error code: " << errorCode
                                                << ", Message: " << msg);

  if (ndn::time::steady_clock::now() < deadline) {
    SequenceNumberMap::const_iterator it = m_highestSeqNo.find(lsaName);

    if (it != m_highestSeqNo.end() && it->second == seqNo) {
      // If the SegmentFetcher failed due to an Interest timeout, it is safe to re-express
      // immediately since at the least the LSA Interest lifetime has elapsed.
      // Otherwise, it is necessary to delay the Interest re-expression to prevent
      // the potential for constant Interest flooding.
      ndn::time::seconds delay = m_nlsr.getConfParameter().getLsaInterestLifetime();

      if (errorCode == ndn::util::SegmentFetcher::ErrorCode::INTEREST_TIMEOUT) {
        delay = ndn::time::seconds(0);
      }

      m_scheduler.scheduleEvent(delay, std::bind(&Lsdb::expressInterest, this,
                                                 interestName, retransmitNo + 1, deadline));
    }
  }
}

void
Lsdb::afterFetchLsa(const ndn::ConstBufferPtr& bufferPtr, ndn::Name& interestName)
{
  std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>(ndn::Name(interestName));
  data->setContent(bufferPtr);

  _LOG_DEBUG("Received data for LSA(name): " << data->getName());

  ndn::Name lsaName = interestName.getSubName(0, interestName.size()-1);
  uint64_t seqNo = interestName[-1].toNumber();

  if (m_highestSeqNo.find(lsaName) == m_highestSeqNo.end()) {
    m_highestSeqNo[lsaName] = seqNo;
  }
  else if (seqNo > m_highestSeqNo[lsaName]) {
    m_highestSeqNo[lsaName] = seqNo;
    _LOG_TRACE("SeqNo for LSA(name): " << data->getName() << "  updated");
  }
  else if (seqNo < m_highestSeqNo[lsaName]) {
    return;
  }

  onContentValidated(data);
}

void
Lsdb::cancelScheduleLsaExpiringEvent(ndn::EventId eid)
{
  m_scheduler.cancelEvent(eid);
}

  /*! \brief Compares if a name LSA is the same as the one specified by key

    \param nlsa1 A name LSA object
    \param key A key of an originating router to compare to nlsa1
   */
static bool
nameLsaCompareByKey(const NameLsa& nlsa1, const ndn::Name& key)
{
  return nlsa1.getKey() == key;
}

bool
Lsdb::buildAndInstallOwnNameLsa()
{
  NameLsa nameLsa(m_nlsr.getConfParameter().getRouterPrefix(),
                  m_nlsr.getSequencingManager().getNameLsaSeq() + 1,
                  getLsaExpirationTimePoint(),
                  m_nlsr.getNamePrefixList());
  m_nlsr.getSequencingManager().increaseNameLsaSeq();
  return installNameLsa(nameLsa);
}

NameLsa*
Lsdb::findNameLsa(const ndn::Name& key)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 std::bind(nameLsaCompareByKey, _1, key));
  if (it != m_nameLsdb.end()) {
    return &(*it);
  }
  return 0;
}

bool
Lsdb::isNameLsaNew(const ndn::Name& key, uint64_t seqNo)
{
  NameLsa* nameLsaCheck = findNameLsa(key);
  // Is the name in the LSDB
  if (nameLsaCheck != 0) {
    // And the supplied seq no is the highest so far
    if (nameLsaCheck->getLsSeqNo() < seqNo) {
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

ndn::EventId
Lsdb::scheduleNameLsaExpiration(const ndn::Name& key, int seqNo,
                                const ndn::time::seconds& expTime)
{
  return m_scheduler.scheduleEvent(expTime + GRACE_PERIOD,
                                   std::bind(&Lsdb::exprireOrRefreshNameLsa, this, key, seqNo));
}

bool
Lsdb::installNameLsa(NameLsa& nlsa)
{
  ndn::time::seconds timeToExpire = m_lsaRefreshTime;
  NameLsa* chkNameLsa = findNameLsa(nlsa.getKey());
  // Determines if the name LSA is new or not.
  if (chkNameLsa == 0) {
    addNameLsa(nlsa);
    _LOG_DEBUG("New Name LSA");
    _LOG_DEBUG("Adding Name Lsa");
    nlsa.writeLog();

    if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      // If this name LSA is from another router, add the advertised
      // prefixes to the NPT.
      m_nlsr.getNamePrefixTable().addEntry(nlsa.getOrigRouter(),
                                           nlsa.getOrigRouter());
      std::list<ndn::Name> nameList = nlsa.getNpl().getNameList();
      for (std::list<ndn::Name>::iterator it = nameList.begin(); it != nameList.end();
           it++) {
        if ((*it) != m_nlsr.getConfParameter().getRouterPrefix()) {
          m_nlsr.getNamePrefixTable().addEntry((*it), nlsa.getOrigRouter());
        }
      }
    }
    if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      ndn::time::system_clock::Duration duration = nlsa.getExpirationTimePoint() -
                                                   ndn::time::system_clock::now();
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }
    nlsa.setExpiringEventId(scheduleNameLsaExpiration(nlsa.getKey(),
                                                      nlsa.getLsSeqNo(),
                                                      timeToExpire));
  }
  // Else this is a known name LSA, so we are updating it.
  else {
    if (chkNameLsa->getLsSeqNo() < nlsa.getLsSeqNo()) {
      _LOG_DEBUG("Updated Name LSA. Updating LSDB");
      _LOG_DEBUG("Deleting Name Lsa");
      chkNameLsa->writeLog();
      chkNameLsa->setLsSeqNo(nlsa.getLsSeqNo());
      chkNameLsa->setExpirationTimePoint(nlsa.getExpirationTimePoint());
      chkNameLsa->getNpl().sort();
      nlsa.getNpl().sort();
      // Obtain the set difference of the current and the incoming
      // name prefix sets, and add those.
      std::list<ndn::Name> nameToAdd;
      std::set_difference(nlsa.getNpl().getNameList().begin(),
                          nlsa.getNpl().getNameList().end(),
                          chkNameLsa->getNpl().getNameList().begin(),
                          chkNameLsa->getNpl().getNameList().end(),
                          std::inserter(nameToAdd, nameToAdd.begin()));
      for (std::list<ndn::Name>::iterator it = nameToAdd.begin();
           it != nameToAdd.end(); ++it) {
        chkNameLsa->addName((*it));
        if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
          if ((*it) != m_nlsr.getConfParameter().getRouterPrefix()) {
            m_nlsr.getNamePrefixTable().addEntry((*it), nlsa.getOrigRouter());
          }
        }
      }

      chkNameLsa->getNpl().sort();

      // Also remove any names that are no longer being advertised.
      std::list<ndn::Name> nameToRemove;
      std::set_difference(chkNameLsa->getNpl().getNameList().begin(),
                          chkNameLsa->getNpl().getNameList().end(),
                          nlsa.getNpl().getNameList().begin(),
                          nlsa.getNpl().getNameList().end(),
                          std::inserter(nameToRemove, nameToRemove.begin()));
      for (std::list<ndn::Name>::iterator it = nameToRemove.begin();
           it != nameToRemove.end(); ++it) {
        _LOG_DEBUG("Removing name LSA no longer advertised: " << (*it).toUri());
        chkNameLsa->removeName((*it));
        if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
          if ((*it) != m_nlsr.getConfParameter().getRouterPrefix()) {
            m_nlsr.getNamePrefixTable().removeEntry((*it), nlsa.getOrigRouter());
          }
        }
      }

      if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
        ndn::time::system_clock::Duration duration = nlsa.getExpirationTimePoint() -
                                                     ndn::time::system_clock::now();
        timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
      }
      cancelScheduleLsaExpiringEvent(chkNameLsa->getExpiringEventId());
      chkNameLsa->setExpiringEventId(scheduleNameLsaExpiration(nlsa.getKey(),
                                                               nlsa.getLsSeqNo(),
                                                               timeToExpire));
      _LOG_DEBUG("Adding Name Lsa");
      chkNameLsa->writeLog();
    }
  }
  return true;
}

bool
Lsdb::addNameLsa(NameLsa& nlsa)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 std::bind(nameLsaCompareByKey, _1,
                                                      nlsa.getKey()));
  if (it == m_nameLsdb.end()) {
    m_nameLsdb.push_back(nlsa);
    return true;
  }
  return false;
}

bool
Lsdb::removeNameLsa(const ndn::Name& key)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 std::bind(nameLsaCompareByKey, _1, key));
  if (it != m_nameLsdb.end()) {
    _LOG_DEBUG("Deleting Name Lsa");
    (*it).writeLog();
    // If the requested name LSA is not ours, we also need to remove
    // its entries from the NPT.
    if ((*it).getOrigRouter() !=
        m_nlsr.getConfParameter().getRouterPrefix()) {
      m_nlsr.getNamePrefixTable().removeEntry((*it).getOrigRouter(),
                                              (*it).getOrigRouter());
      for (std::list<ndn::Name>::iterator nit = (*it).getNpl().getNameList().begin();
           nit != (*it).getNpl().getNameList().end(); ++nit) {
        if ((*nit) != m_nlsr.getConfParameter().getRouterPrefix()) {
          m_nlsr.getNamePrefixTable().removeEntry((*nit), (*it).getOrigRouter());
        }
      }
    }
    m_nameLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesNameLsaExist(const ndn::Name& key)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 std::bind(nameLsaCompareByKey, _1, key));
  if (it == m_nameLsdb.end()) {
    return false;
  }
  return true;
}

void
Lsdb::writeNameLsdbLog()
{
  _LOG_DEBUG("---------------Name LSDB-------------------");
  for (std::list<NameLsa>::iterator it = m_nameLsdb.begin();
       it != m_nameLsdb.end() ; it++) {
    (*it).writeLog();
  }
}

const std::list<NameLsa>&
Lsdb::getNameLsdb()
{
  return m_nameLsdb;
}

// Cor LSA and LSDB related Functions start here

/*! \brief Compares whether an LSA object is the same as a key.
  \param clsa The cor. LSA to check the identity of.
  \param key The key of the publishing router to check against.
*/
static bool
corLsaCompareByKey(const CoordinateLsa& clsa, const ndn::Name& key)
{
  return clsa.getKey() == key;
}

bool
Lsdb::buildAndInstallOwnCoordinateLsa()
{
  CoordinateLsa corLsa(m_nlsr.getConfParameter().getRouterPrefix(),
                       m_nlsr.getSequencingManager().getCorLsaSeq() + 1,
                       getLsaExpirationTimePoint(),
                       m_nlsr.getConfParameter().getCorR(),
                       m_nlsr.getConfParameter().getCorTheta());

  // Sync coordinate LSAs if using HR or HR dry run.
  if (m_nlsr.getConfParameter().getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
    m_nlsr.getSequencingManager().increaseCorLsaSeq();
    m_sync.publishRoutingUpdate();
  }

  installCoordinateLsa(corLsa);

  return true;
}

CoordinateLsa*
Lsdb::findCoordinateLsa(const ndn::Name& key)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       std::bind(corLsaCompareByKey, _1, key));
  if (it != m_corLsdb.end()) {
    return &(*it);
  }
  return 0;
}

bool
Lsdb::isCoordinateLsaNew(const ndn::Name& key, uint64_t seqNo)
{
  CoordinateLsa* clsa = findCoordinateLsa(key);
  // Is the coordinate LSA in the LSDB already
  if (clsa != 0) {
    // And the seq no is newer (higher) than the current one
    if (clsa->getLsSeqNo() < seqNo) {
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

  // Schedules a refresh/expire event in the scheduler.
  // \param key The name of the router that published the LSA.
  // \param seqNo the seq. no. associated with the LSA to check.
  // \param expTime How long to wait before triggering the event.
ndn::EventId
Lsdb::scheduleCoordinateLsaExpiration(const ndn::Name& key, int seqNo,
                                      const ndn::time::seconds& expTime)
{
  return m_scheduler.scheduleEvent(expTime + GRACE_PERIOD,
                                   std::bind(&Lsdb::exprireOrRefreshCoordinateLsa,
                                             this, key, seqNo));
}

bool
Lsdb::installCoordinateLsa(CoordinateLsa& clsa)
{
  ndn::time::seconds timeToExpire = m_lsaRefreshTime;
  CoordinateLsa* chkCorLsa = findCoordinateLsa(clsa.getKey());
  // Checking whether the LSA is new or not.
  if (chkCorLsa == 0) {
    _LOG_DEBUG("New Coordinate LSA. Adding to LSDB");
    _LOG_DEBUG("Adding Coordinate Lsa");
    clsa.writeLog();
    addCoordinateLsa(clsa);

    // Register the LSA's origin router prefix
    if (clsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      m_nlsr.getNamePrefixTable().addEntry(clsa.getOrigRouter(),
                                           clsa.getOrigRouter());
    }
    if (m_nlsr.getConfParameter().getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
      m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
    }
    // Set the expiration time for the new LSA.
    if (clsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      ndn::time::system_clock::Duration duration = clsa.getExpirationTimePoint() -
                                                   ndn::time::system_clock::now();
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }
    scheduleCoordinateLsaExpiration(clsa.getKey(),
                                    clsa.getLsSeqNo(), timeToExpire);
  }
  // We are just updating this LSA.
  else {
    if (chkCorLsa->getLsSeqNo() < clsa.getLsSeqNo()) {
      _LOG_DEBUG("Updated Coordinate LSA. Updating LSDB");
      _LOG_DEBUG("Deleting Coordinate Lsa");
      chkCorLsa->writeLog();
      chkCorLsa->setLsSeqNo(clsa.getLsSeqNo());
      chkCorLsa->setExpirationTimePoint(clsa.getExpirationTimePoint());
      // If the new LSA contains new routing information, update the LSDB with it.
      if (!chkCorLsa->isEqualContent(clsa)) {
        chkCorLsa->setCorRadius(clsa.getCorRadius());
        chkCorLsa->setCorTheta(clsa.getCorTheta());
        if (m_nlsr.getConfParameter().getHyperbolicState() >= HYPERBOLIC_STATE_ON) {
          m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
        }
      }
      // If this is an LSA from another router, refresh its expiration time.
      if (clsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
        ndn::time::system_clock::Duration duration = clsa.getExpirationTimePoint() -
                                                     ndn::time::system_clock::now();
        timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
      }
      cancelScheduleLsaExpiringEvent(chkCorLsa->getExpiringEventId());
      chkCorLsa->setExpiringEventId(scheduleCoordinateLsaExpiration(clsa.getKey(),
                                                                    clsa.getLsSeqNo(),
                                                                    timeToExpire));
      _LOG_DEBUG("Adding Coordinate Lsa");
      chkCorLsa->writeLog();
    }
  }
  return true;
}

bool
Lsdb::addCoordinateLsa(CoordinateLsa& clsa)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       std::bind(corLsaCompareByKey, _1,
                                                                 clsa.getKey()));
  if (it == m_corLsdb.end()) {
    m_corLsdb.push_back(clsa);
    return true;
  }
  return false;
}

bool
Lsdb::removeCoordinateLsa(const ndn::Name& key)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       std::bind(corLsaCompareByKey,
                                                                 _1, key));
  if (it != m_corLsdb.end()) {
    _LOG_DEBUG("Deleting Coordinate Lsa");
    it->writeLog();

    if (it->getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      m_nlsr.getNamePrefixTable().removeEntry(it->getOrigRouter(), it->getOrigRouter());
    }

    m_corLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesCoordinateLsaExist(const ndn::Name& key)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       std::bind(corLsaCompareByKey,
                                                                 _1, key));
  if (it == m_corLsdb.end()) {
    return false;
  }
  return true;
}

void
Lsdb::writeCorLsdbLog()
{
  _LOG_DEBUG("---------------Cor LSDB-------------------");
  for (std::list<CoordinateLsa>::iterator it = m_corLsdb.begin();
       it != m_corLsdb.end() ; it++) {
    (*it).writeLog();
  }
}

const std::list<CoordinateLsa>&
Lsdb::getCoordinateLsdb()
{
  return m_corLsdb;
}

// Adj LSA and LSDB related function starts here

  /*! \brief Returns whether an adj. LSA object is from some router.
    \param alsa The adj. LSA object.
    \param key The router name that you want to compare the LSA with.
   */
static bool
adjLsaCompareByKey(AdjLsa& alsa, const ndn::Name& key)
{
  return alsa.getKey() == key;
}

void
Lsdb::scheduleAdjLsaBuild()
{
  m_nlsr.incrementAdjBuildCount();

  if (m_nlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_ON) {
    // Don't build adjacency LSAs in hyperbolic routing
    _LOG_DEBUG("Adjacency LSA not built. Currently in hyperbolic routing state.");
    return;
  }

  if (m_nlsr.getIsBuildAdjLsaSheduled() == false) {
    _LOG_DEBUG("Scheduling Adjacency LSA build in " << m_adjLsaBuildInterval);

    m_scheduler.scheduleEvent(m_adjLsaBuildInterval, std::bind(&Lsdb::buildAdjLsa, this));
    m_nlsr.setIsBuildAdjLsaSheduled(true);
  }
}

void
Lsdb::buildAdjLsa()
{
  _LOG_TRACE("Lsdb::buildAdjLsa called");

  m_nlsr.setIsBuildAdjLsaSheduled(false);

  if (m_nlsr.getAdjacencyList().isAdjLsaBuildable(m_nlsr.getConfParameter().getInterestRetryNumber())) {

    int adjBuildCount = m_nlsr.getAdjBuildCount();
    // Only do the adjLsa build if there's one scheduled
    if (adjBuildCount > 0) {
      // It only makes sense to do the adjLsa build if we have neighbors
      if (m_nlsr.getAdjacencyList().getNumOfActiveNeighbor() > 0) {
        _LOG_DEBUG("Building and installing own Adj LSA");
        buildAndInstallOwnAdjLsa();
      }
      // We have no active neighbors, meaning no one can route through
      // us.  So delete our entry in the LSDB. This prevents this
      // router from refreshing the LSA, eventually causing other
      // routers to delete it, too.
      else {
        _LOG_DEBUG("Removing own Adj LSA; no ACTIVE neighbors");
        // Get this router's key
        ndn::Name key = m_nlsr.getConfParameter().getRouterPrefix();
        key.append(AdjLsa::TYPE_STRING);

        removeAdjLsa(key);
        // Recompute routing table after removal
        m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
      }
      // In the case that during building the adj LSA, the FIB has to
      // wait on an Interest response, the number of scheduled adj LSA
      // builds could change, so we shouldn't just set it to 0.
      m_nlsr.setAdjBuildCount(m_nlsr.getAdjBuildCount() - adjBuildCount);
    }
  }
  // We are still waiting to know the adjacency status of some
  // neighbor, so schedule a build for later (when all that has
  // hopefully finished)
  else {
    m_nlsr.setIsBuildAdjLsaSheduled(true);
    int schedulingTime = m_nlsr.getConfParameter().getInterestRetryNumber() *
                         m_nlsr.getConfParameter().getInterestResendTime();
    m_scheduler.scheduleEvent(ndn::time::seconds(schedulingTime),
                              std::bind(&Lsdb::buildAdjLsa, this));
  }
}

bool
Lsdb::addAdjLsa(AdjLsa& alsa)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                std::bind(adjLsaCompareByKey, _1,
                                                     alsa.getKey()));
  if (it == m_adjLsdb.end()) {
    m_adjLsdb.push_back(alsa);
    return true;
  }
  return false;
}

AdjLsa*
Lsdb::findAdjLsa(const ndn::Name& key)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                std::bind(adjLsaCompareByKey, _1, key));
  if (it != m_adjLsdb.end()) {
    return &(*it);
  }
  return 0;
}

bool
Lsdb::isAdjLsaNew(const ndn::Name& key, uint64_t seqNo)
{
  AdjLsa*  adjLsaCheck = findAdjLsa(key);
  // If it is in the LSDB
  if (adjLsaCheck != 0) {
    // And the supplied seq no is newer (higher) than the current one.
    if (adjLsaCheck->getLsSeqNo() < seqNo) {
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

ndn::EventId
Lsdb::scheduleAdjLsaExpiration(const ndn::Name& key, int seqNo,
                               const ndn::time::seconds& expTime)
{
  return m_scheduler.scheduleEvent(expTime + GRACE_PERIOD,
                                   std::bind(&Lsdb::exprireOrRefreshAdjLsa, this, key, seqNo));
}

bool
Lsdb::installAdjLsa(AdjLsa& alsa)
{
  ndn::time::seconds timeToExpire = m_lsaRefreshTime;
  AdjLsa* chkAdjLsa = findAdjLsa(alsa.getKey());
  // If this adj. LSA is not in the LSDB already
  if (chkAdjLsa == 0) {
    _LOG_DEBUG("New Adj LSA. Adding to LSDB");
    _LOG_DEBUG("Adding Adj Lsa");
    alsa.writeLog();
    addAdjLsa(alsa);
    // Add any new name prefixes to the NPT
    alsa.addNptEntries(m_nlsr);
    m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
    if (alsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      ndn::time::system_clock::Duration duration = alsa.getExpirationTimePoint() -
                                                   ndn::time::system_clock::now();
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }
    scheduleAdjLsaExpiration(alsa.getKey(),
                             alsa.getLsSeqNo(), timeToExpire);
  }
  else {
    if (chkAdjLsa->getLsSeqNo() < alsa.getLsSeqNo()) {
      _LOG_DEBUG("Updated Adj LSA. Updating LSDB");
      _LOG_DEBUG("Deleting Adj Lsa");
      chkAdjLsa->writeLog();
      chkAdjLsa->setLsSeqNo(alsa.getLsSeqNo());
      chkAdjLsa->setExpirationTimePoint(alsa.getExpirationTimePoint());
      // If the new adj LSA has new content, update the contents of
      // the LSDB entry. Additionally, since we've changed the
      // contents of the LSDB, we have to schedule a routing
      // calculation.
      if (!chkAdjLsa->isEqualContent(alsa)) {
        chkAdjLsa->getAdl().reset();
        chkAdjLsa->getAdl().addAdjacents(alsa.getAdl());
        m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
      }
      if (alsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
        ndn::time::system_clock::Duration duration = alsa.getExpirationTimePoint() -
                                                     ndn::time::system_clock::now();
        timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
      }
      cancelScheduleLsaExpiringEvent(chkAdjLsa->getExpiringEventId());
      chkAdjLsa->setExpiringEventId(scheduleAdjLsaExpiration(alsa.getKey(),
                                                             alsa.getLsSeqNo(),
                                                             timeToExpire));
      _LOG_DEBUG("Adding Adj Lsa");
      chkAdjLsa->writeLog();
    }
  }
  return true;
}

bool
Lsdb::buildAndInstallOwnAdjLsa()
{
  AdjLsa adjLsa(m_nlsr.getConfParameter().getRouterPrefix(),
                m_nlsr.getSequencingManager().getAdjLsaSeq() + 1,
                getLsaExpirationTimePoint(),
                m_nlsr.getAdjacencyList().getNumOfActiveNeighbor(),
                m_nlsr.getAdjacencyList());

  //Sync adjacency LSAs if link-state or dry-run HR is enabled.
  if (m_nlsr.getConfParameter().getHyperbolicState() != HYPERBOLIC_STATE_ON) {
    m_nlsr.getSequencingManager().increaseAdjLsaSeq();
    m_sync.publishRoutingUpdate();
  }

  return installAdjLsa(adjLsa);
}

bool
Lsdb::removeAdjLsa(const ndn::Name& key)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                std::bind(adjLsaCompareByKey, _1, key));
  if (it != m_adjLsdb.end()) {
    _LOG_DEBUG("Deleting Adj Lsa");
    (*it).writeLog();
    (*it).removeNptEntries(m_nlsr);
    m_adjLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesAdjLsaExist(const ndn::Name& key)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                std::bind(adjLsaCompareByKey, _1, key));
  if (it == m_adjLsdb.end()) {
    return false;
  }
  return true;
}

const std::list<AdjLsa>&
Lsdb::getAdjLsdb()
{
  return m_adjLsdb;
}

void
Lsdb::setLsaRefreshTime(const seconds& lsaRefreshTime)
{
  m_lsaRefreshTime = lsaRefreshTime;
}

void
Lsdb::setThisRouterPrefix(std::string trp)
{
  m_thisRouterPrefix = trp;
}

  // This function determines whether a name LSA should be refreshed
  // or expired. The conditions for getting refreshed are: it is still
  // in the LSDB, it hasn't been updated by something else already (as
  // evidenced by its seq. no.), and this is the originating router for
  // the LSA. Is it let expire in all other cases.
  // lsaKey is the key of the LSA's publishing router.
  // seqNo is the seq. no. of the candidate LSA.
void
Lsdb::exprireOrRefreshNameLsa(const ndn::Name& lsaKey, uint64_t seqNo)
{
  _LOG_DEBUG("Lsdb::exprireOrRefreshNameLsa Called");
  _LOG_DEBUG("LSA Key : " << lsaKey << " Seq No: " << seqNo);
  NameLsa* chkNameLsa = findNameLsa(lsaKey);
  // If this name LSA exists in the LSDB
  if (chkNameLsa != 0) {
    _LOG_DEBUG("LSA Exists with seq no: " << chkNameLsa->getLsSeqNo());
    // If its seq no is the one we are expecting.
    if (chkNameLsa->getLsSeqNo() == seqNo) {
      if (chkNameLsa->getOrigRouter() == m_thisRouterPrefix) {
        _LOG_DEBUG("Own Name LSA, so refreshing it");
        _LOG_DEBUG("Deleting Name Lsa");
        chkNameLsa->writeLog();
        chkNameLsa->setLsSeqNo(chkNameLsa->getLsSeqNo() + 1);
        m_nlsr.getSequencingManager().setNameLsaSeq(chkNameLsa->getLsSeqNo());
        chkNameLsa->setExpirationTimePoint(getLsaExpirationTimePoint());
        _LOG_DEBUG("Adding Name Lsa");
        chkNameLsa->writeLog();
        // schedule refreshing event again
        chkNameLsa->setExpiringEventId(scheduleNameLsaExpiration(chkNameLsa->getKey(),
                                                                 chkNameLsa->getLsSeqNo(),
                                                                 m_lsaRefreshTime));
        m_sync.publishRoutingUpdate();
      }
      // Since we cannot refresh other router's LSAs, our only choice is to expire.
      else {
        _LOG_DEBUG("Other's Name LSA, so removing from LSDB");
        removeNameLsa(lsaKey);
      }
    }
  }
}

  // This function determines whether an adj. LSA should be refreshed
  // or expired. The conditions for getting refreshed are: it is still
  // in the LSDB, it hasn't been updated by something else already (as
  // evidenced by its seq. no.), and this is the originating router for
  // the LSA. Is it let expire in all other cases.
  // lsaKey is the key of the LSA's publishing router.
  // seqNo is the seq. no. of the candidate LSA.
void
Lsdb::exprireOrRefreshAdjLsa(const ndn::Name& lsaKey, uint64_t seqNo)
{
  _LOG_DEBUG("Lsdb::exprireOrRefreshAdjLsa Called");
  _LOG_DEBUG("LSA Key: " << lsaKey << " Seq No: " << seqNo);
  AdjLsa* chkAdjLsa = findAdjLsa(lsaKey);
  // If this is a valid LSA
  if (chkAdjLsa != 0) {
    _LOG_DEBUG("LSA Exists with seq no: " << chkAdjLsa->getLsSeqNo());
    // And if it hasn't been updated for some other reason
    if (chkAdjLsa->getLsSeqNo() == seqNo) {
      // If it is our own LSA
      if (chkAdjLsa->getOrigRouter() == m_thisRouterPrefix) {
        _LOG_DEBUG("Own Adj LSA, so refreshing it");
        _LOG_DEBUG("Deleting Adj Lsa");
        chkAdjLsa->writeLog();
        chkAdjLsa->setLsSeqNo(chkAdjLsa->getLsSeqNo() + 1);
        m_nlsr.getSequencingManager().setAdjLsaSeq(chkAdjLsa->getLsSeqNo());
        chkAdjLsa->setExpirationTimePoint(getLsaExpirationTimePoint());
        _LOG_DEBUG("Adding Adj Lsa");
        chkAdjLsa->writeLog();
        // schedule refreshing event again
        chkAdjLsa->setExpiringEventId(scheduleAdjLsaExpiration(chkAdjLsa->getKey(),
                                                               chkAdjLsa->getLsSeqNo(),
                                                               m_lsaRefreshTime));
        m_sync.publishRoutingUpdate();
      }
      // An LSA from another router is expiring
      else {
        _LOG_DEBUG("Other's Adj LSA, so removing from LSDB");
        removeAdjLsa(lsaKey);
      }
      // We have changed the contents of the LSDB, so we have to
      // schedule a routing calculation
      m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
    }
  }
}

  // This function determines whether an adj. LSA should be refreshed
  // or expired. The conditions for getting refreshed are: it is still
  // in the LSDB, it hasn't been updated by something else already (as
  // evidenced by its seq. no.), and this is the originating router for
  // the LSA. It is let expire in all other cases.
  // lsaKey is the key of the LSA's publishing router.
  // seqNo is the seq. no. of the candidate LSA.
void
Lsdb::exprireOrRefreshCoordinateLsa(const ndn::Name& lsaKey,
                                    uint64_t seqNo)
{
  _LOG_DEBUG("Lsdb::exprireOrRefreshCorLsa Called ");
  _LOG_DEBUG("LSA Key : " << lsaKey << " Seq No: " << seqNo);
  CoordinateLsa* chkCorLsa = findCoordinateLsa(lsaKey);
  // Whether the LSA is in the LSDB or not.
  if (chkCorLsa != 0) {
    _LOG_DEBUG("LSA Exists with seq no: " << chkCorLsa->getLsSeqNo());
    // Whether the LSA has been updated without our knowledge.
    if (chkCorLsa->getLsSeqNo() == seqNo) {
      if (chkCorLsa->getOrigRouter() == m_thisRouterPrefix) {
        _LOG_DEBUG("Own Cor LSA, so refreshing it");
        _LOG_DEBUG("Deleting Coordinate Lsa");
        chkCorLsa->writeLog();
        chkCorLsa->setLsSeqNo(chkCorLsa->getLsSeqNo() + 1);
        if (m_nlsr.getConfParameter().getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
          m_nlsr.getSequencingManager().setCorLsaSeq(chkCorLsa->getLsSeqNo());
        }

        chkCorLsa->setExpirationTimePoint(getLsaExpirationTimePoint());
        _LOG_DEBUG("Adding Coordinate Lsa");
        chkCorLsa->writeLog();
        // schedule refreshing event again
        chkCorLsa->setExpiringEventId(scheduleCoordinateLsaExpiration(
                                        chkCorLsa->getKey(),
                                        chkCorLsa->getLsSeqNo(),
                                        m_lsaRefreshTime));
        // Only sync coordinate LSAs if link-state routing is disabled
        if (m_nlsr.getConfParameter().getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
          m_sync.publishRoutingUpdate();
        }
      }
      // We can't refresh other router's LSAs, so we remove it.
      else {
        _LOG_DEBUG("Other's Cor LSA, so removing from LSDB");
        removeCoordinateLsa(lsaKey);
      }
      if (m_nlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_ON) {
        m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
      }
    }
  }
}

void
Lsdb::expressInterest(const ndn::Name& interestName, uint32_t timeoutCount,
                      steady_clock::TimePoint deadline)
{
  if (deadline == DEFAULT_LSA_RETRIEVAL_DEADLINE) {
    deadline = steady_clock::now() + ndn::time::seconds(static_cast<int>(LSA_REFRESH_TIME_MAX));
  }
  // The first component of the interest is the name.
  ndn::Name lsaName = interestName.getSubName(0, interestName.size()-1);
  // The seq no is the last
  uint64_t seqNo = interestName[-1].toNumber();

  // If the LSA is not found in the list currently.
  if (m_highestSeqNo.find(lsaName) == m_highestSeqNo.end()) {
    m_highestSeqNo[lsaName] = seqNo;
  }
  // If the new seq no is higher, that means the LSA is valid
  else if (seqNo > m_highestSeqNo[lsaName]) {
    m_highestSeqNo[lsaName] = seqNo;
  }
  // Otherwise, its an old/invalid LSA
  else if (seqNo < m_highestSeqNo[lsaName]) {
    return;
  }

  ndn::Interest interest(interestName);
  interest.setInterestLifetime(m_nlsr.getConfParameter().getLsaInterestLifetime());

  _LOG_DEBUG("Fetching Data for LSA: " << interestName << " Seq number: " << seqNo);
  ndn::util::SegmentFetcher::fetch(m_nlsr.getNlsrFace(), interest,
                                   m_nlsr.getValidator(),
                                   std::bind(&Lsdb::afterFetchLsa, this, _1, interestName),
                                   std::bind(&Lsdb::onFetchLsaError, this, _1, _2, interestName,
                                             timeoutCount, deadline, lsaName, seqNo));
}

void
Lsdb::processInterest(const ndn::Name& name, const ndn::Interest& interest)
{
  const ndn::Name& interestName(interest.getName());
  _LOG_DEBUG("Interest received for LSA: " << interestName);

  std::string chkString("LSA");
  int32_t lsaPosition = util::getNameComponentPosition(interest.getName(), chkString);

  if (lsaPosition >= 0) {

    // Forms the name of the router that the Interest packet came from.
    ndn::Name originRouter = m_nlsr.getConfParameter().getNetwork();
    originRouter.append(interestName.getSubName(lsaPosition + 1,
                                                interest.getName().size() - lsaPosition - 3));

    uint64_t seqNo = interestName[-1].toNumber();
    _LOG_DEBUG("LSA sequence number from interest: " << seqNo);

    std::string interestedLsType = interestName[-2].toUri();

    // Passes the Interest off to the appropriate subprocessor
    if (interestedLsType == NameLsa::TYPE_STRING) {
      processInterestForNameLsa(interest, originRouter.append(interestedLsType), seqNo);
    }
    else if (interestedLsType == AdjLsa::TYPE_STRING) {
      processInterestForAdjacencyLsa(interest, originRouter.append(interestedLsType), seqNo);
    }
    else if (interestedLsType == CoordinateLsa::TYPE_STRING) {
      processInterestForCoordinateLsa(interest, originRouter.append(interestedLsType), seqNo);
    }
    else {
      _LOG_WARN("Received unrecognized LSA type: " << interestedLsType);
    }
  }
}

  // \brief Sends LSA data.
  // \param interest The Interest that warranted the data.
  // \param content The data that the Interest was seeking.
void
Lsdb::putLsaData(const ndn::Interest& interest, const std::string& content)
{
  LsaContentPublisher publisher(m_nlsr.getNlsrFace(),
                                m_nlsr.getKeyChain(),
                                m_lsaRefreshTime,
                                content);
  _LOG_DEBUG("Sending requested data ( " << content << ")  for interest (" << interest
             << ") to be published and added to face.");
  publisher.publish(interest.getName(),
                    ndn::security::signingByCertificate(m_nlsr.getDefaultCertName()));
}

  // \brief Finds and sends a requested name LSA.
  // \param interest The interest that seeks the name LSA.
  // \param lsaKey The LSA that the Interest is seeking.
  // \param seqNo A sequence number to ensure that we are sending the
  // version that was requested.
void
Lsdb::processInterestForNameLsa(const ndn::Interest& interest,
                                const ndn::Name& lsaKey,
                                uint64_t seqNo)
{

  _LOG_DEBUG("nameLsa interest " << interest << " received");
  NameLsa*  nameLsa = m_nlsr.getLsdb().findNameLsa(lsaKey);
  if (nameLsa != 0) {
    if (nameLsa->getLsSeqNo() == seqNo) {
      std::string content = nameLsa->getData();
      putLsaData(interest,content);
    }
    else {
      _LOG_TRACE("SeqNo for nameLsa does not match");
    }
  }
  else {
    _LOG_TRACE(interest << "  was not found in this lsdb");
  }
}

  // \brief Finds and sends a requested adj. LSA.
  // \param interest The interest that seeks the adj. LSA.
  // \param lsaKey The LSA that the Interest is seeking.
  // \param seqNo A sequence number to ensure that we are sending the
  // version that was requested.
void
Lsdb::processInterestForAdjacencyLsa(const ndn::Interest& interest,
                                     const ndn::Name& lsaKey,
                                     uint64_t seqNo)
{
  if (m_nlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_ON) {
    _LOG_ERROR("Received interest for an adjacency LSA when hyperbolic routing is enabled");
  }

  _LOG_DEBUG("AdjLsa interest " << interest << " received");
  AdjLsa* adjLsa = m_nlsr.getLsdb().findAdjLsa(lsaKey);
  if (adjLsa != 0) {
    if (adjLsa->getLsSeqNo() == seqNo) {
      std::string content = adjLsa->getData();
      putLsaData(interest,content);
    }
    else {
      _LOG_TRACE("SeqNo for AdjLsa does not match");
    }
  }
  else {
    _LOG_TRACE(interest << "  was not found in this lsdb");
  }
}

  // \brief Finds and sends a requested cor. LSA.
  // \param interest The interest that seeks the cor. LSA.
  // \param lsaKey The LSA that the Interest is seeking.
  // \param seqNo A sequence number to ensure that we are sending the
  // version that was requested.
void
Lsdb::processInterestForCoordinateLsa(const ndn::Interest& interest,
                                      const ndn::Name& lsaKey,
                                      uint64_t seqNo)
{
  if (m_nlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_OFF) {
    _LOG_ERROR("Received Interest for a coordinate LSA when link-state routing is enabled");
  }

  _LOG_DEBUG("CoordinateLsa interest " << interest << " received");
  CoordinateLsa* corLsa = m_nlsr.getLsdb().findCoordinateLsa(lsaKey);
  if (corLsa != 0) {
    if (corLsa->getLsSeqNo() == seqNo) {
      std::string content = corLsa->getData();
      putLsaData(interest,content);
    }
    else {
      _LOG_TRACE("SeqNo for CoordinateLsa does not match");
    }
  }
  else {
    _LOG_TRACE(interest << "  was not found in this lsdb");
  }
}

void
Lsdb::onContentValidated(const std::shared_ptr<const ndn::Data>& data)
{
  const ndn::Name& dataName = data->getName();
  _LOG_DEBUG("Data validation successful for LSA: " << dataName);

  std::string chkString("LSA");
  int32_t lsaPosition = util::getNameComponentPosition(dataName, chkString);

  if (lsaPosition >= 0) {

    // Extracts the prefix of the originating router from the data.
    ndn::Name originRouter = m_nlsr.getConfParameter().getNetwork();
    originRouter.append(dataName.getSubName(lsaPosition + 1, dataName.size() - lsaPosition - 3));

    uint64_t seqNo = dataName[-1].toNumber();
    std::string dataContent(reinterpret_cast<const char*>(data->getContent().value()));

    std::string interestedLsType  = dataName[-2].toUri();

    if (interestedLsType == NameLsa::TYPE_STRING) {
      processContentNameLsa(originRouter.append(interestedLsType), seqNo, dataContent);
    }
    else if (interestedLsType == AdjLsa::TYPE_STRING) {
      processContentAdjacencyLsa(originRouter.append(interestedLsType), seqNo, dataContent);
    }
    else if (interestedLsType == CoordinateLsa::TYPE_STRING) {
      processContentCoordinateLsa(originRouter.append(interestedLsType), seqNo, dataContent);
    }
    else {
      _LOG_WARN("Received unrecognized LSA Type: " << interestedLsType);
    }
  }
}

void
Lsdb::processContentNameLsa(const ndn::Name& lsaKey,
                            uint64_t lsSeqNo, std::string& dataContent)
{
  if (isNameLsaNew(lsaKey, lsSeqNo)) {
    NameLsa nameLsa;
    if (nameLsa.initializeFromContent(dataContent)) {
      installNameLsa(nameLsa);
    }
    else {
      _LOG_DEBUG("LSA data decoding error :(");
    }
  }
}

void
Lsdb::processContentAdjacencyLsa(const ndn::Name& lsaKey,
                                 uint64_t lsSeqNo, std::string& dataContent)
{
  if (isAdjLsaNew(lsaKey, lsSeqNo)) {
    AdjLsa adjLsa;
    if (adjLsa.initializeFromContent(dataContent)) {
      installAdjLsa(adjLsa);
    }
    else {
      _LOG_DEBUG("LSA data decoding error :(");
    }
  }
}

void
Lsdb::processContentCoordinateLsa(const ndn::Name& lsaKey,
                                  uint64_t lsSeqNo, std::string& dataContent)
{
  if (isCoordinateLsaNew(lsaKey, lsSeqNo)) {
    CoordinateLsa corLsa;
    if (corLsa.initializeFromContent(dataContent)) {
      installCoordinateLsa(corLsa);
    }
    else {
      _LOG_DEBUG("LSA data decoding error :(");
    }
  }
}

ndn::time::system_clock::TimePoint
Lsdb::getLsaExpirationTimePoint()
{
  ndn::time::system_clock::TimePoint expirationTimePoint = ndn::time::system_clock::now();
  expirationTimePoint = expirationTimePoint +
                        ndn::time::seconds(m_nlsr.getConfParameter().getRouterDeadInterval());
  return expirationTimePoint;
}

void
Lsdb::writeAdjLsdbLog()
{
  _LOG_DEBUG("---------------Adj LSDB-------------------");
  for (std::list<AdjLsa>::iterator it = m_adjLsdb.begin();
       it != m_adjLsdb.end() ; it++) {
    (*it).writeLog();
  }
}

//-----utility function -----
bool
Lsdb::doesLsaExist(const ndn::Name& key, const std::string& lsType)
{
  if (lsType == NameLsa::TYPE_STRING) {
    return doesNameLsaExist(key);
  }
  else if (lsType == AdjLsa::TYPE_STRING) {
    return doesAdjLsaExist(key);
  }
  else if (lsType == CoordinateLsa::TYPE_STRING) {
    return doesCoordinateLsaExist(key);
  }
  return false;
}

} // namespace nlsr
