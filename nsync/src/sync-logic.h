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

#ifndef SYNC_LOGIC_H
#define SYNC_LOGIC_H

#include <boost/random.hpp>
#include <memory>
#include <map>

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/validator.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "sync-interest-table.h"
#include "sync-diff-state.h"
#include "sync-full-state.h"
#include "sync-std-name-info.h"

#include "sync-diff-state-container.h"

#ifdef _DEBUG
#ifdef HAVE_LOG4CXX
#include <log4cxx/logger.h>
#endif
#endif

namespace Sync {

struct MissingDataInfo {
  std::string prefix;
  SeqNo low;
  SeqNo high;
};

/**
 * \ingroup sync
 * @brief A wrapper for SyncApp, which handles ccnx related things (process
 * interests and data)
 */

class SyncLogic
{
public:
  //typedef boost::function< void ( const std::string &/*prefix*/, const SeqNo &/*newSeq*/, const SeqNo &/*oldSeq*/ ) > LogicUpdateCallback;
  typedef boost::function< void (const std::vector<MissingDataInfo> & ) > LogicUpdateCallback;
  typedef boost::function< void (const std::string &/*prefix*/ ) > LogicRemoveCallback;
  typedef boost::function< void (const std::string &)> LogicPerBranchCallback;

  /**
   * @brief Constructor
   * @param syncPrefix the name prefix to use for the Sync Interest
   * @param onUpdate function that will be called when new state is detected
   * @param onRemove function that will be called when state is removed
   * @param ccnxHandle ccnx handle
   * the app data when new remote names are learned
   */
  SyncLogic (const ndn::Name& syncPrefix,
             ndn::shared_ptr<ndn::Validator> validator,
             ndn::shared_ptr<ndn::Face> face,
             LogicUpdateCallback onUpdate,
             LogicRemoveCallback onRemove);

  SyncLogic (const ndn::Name& syncPrefix,
             ndn::shared_ptr<ndn::Validator> validator,
             ndn::shared_ptr<ndn::Face> face,
             LogicPerBranchCallback onUpdateBranch);

  ~SyncLogic ();

  /**
   * a wrapper for the same func in SyncApp
   */
  void addLocalNames (const ndn::Name &prefix, uint64_t session, uint64_t seq);

  /**
   * @brief remove a participant's subtree from the sync tree
   * @param prefix the name prefix for the participant
   */
  void remove (const ndn::Name &prefix);

  std::string
  getRootDigest();

#ifdef _DEBUG
  ndn::Scheduler &
  getScheduler () { return m_scheduler; }
#endif

  void
  printState () const;

  std::map<std::string, bool>
  getBranchPrefixes() const;

private: 
  void
  delayedChecksLoop ();

  void
  onSyncInterest (const ndn::Name& prefix, const ndn::Interest& interest);

  void
  onSyncRegisterFailed(const ndn::Name& prefix, const std::string& msg);

  void
  onSyncData(const ndn::Interest& interest, ndn::Data& data);

  void
  onSyncTimeout(const ndn::Interest& interest);

  void
  onSyncDataValidationFailed(const ndn::shared_ptr<const ndn::Data>& data);

  void
  onSyncDataValidated(const ndn::shared_ptr<const ndn::Data>& data);

  void
  processSyncInterest (const ndn::Name &name,
                       DigestConstPtr digest, bool timedProcessing=false);

  void
  processSyncData (const ndn::Name &name,
                   DigestConstPtr digest, const char *wireData, size_t len);
  
  void
  processSyncRecoveryInterest (const ndn::Name &name,
                               DigestConstPtr digest);
  
  void 
  insertToDiffLog (DiffStatePtr diff);

  void
  satisfyPendingSyncInterests (DiffStateConstPtr diff);

  boost::tuple<DigestConstPtr, std::string>
  convertNameToDigestAndType (const ndn::Name &name);

  void
  sendSyncInterest ();

  void
  sendSyncRecoveryInterests (DigestConstPtr digest);

  void
  sendSyncData (const ndn::Name &name,
                DigestConstPtr digest, StateConstPtr state);

  void
  sendSyncData (const ndn::Name &name,
                DigestConstPtr digest, SyncStateMsg &msg);

  size_t
  getNumberOfBranches () const;
  
private:
  FullStatePtr m_state;
  DiffStateContainer m_log;

  ndn::Name m_outstandingInterestName;
  SyncInterestTable m_syncInterestTable;

  ndn::Name m_syncPrefix;
  LogicUpdateCallback m_onUpdate;
  LogicRemoveCallback m_onRemove;
  LogicPerBranchCallback m_onUpdateBranch;
  bool m_perBranch;
  ndn::ptr_lib::shared_ptr<ndn::Validator> m_validator;
  ndn::ptr_lib::shared_ptr<ndn::KeyChain> m_keyChain;
  ndn::ptr_lib::shared_ptr<ndn::Face> m_face;
  const ndn::RegisteredPrefixId* m_syncRegisteredPrefixId;

  ndn::Scheduler m_scheduler;

  boost::mt19937 m_randomGenerator;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > m_rangeUniformRandom;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > m_reexpressionJitter;

  static const int m_unknownDigestStoreTime = 10; // seconds
  static const int m_syncResponseFreshness = 1000; // MUST BE dividable by 1000!!!
  static const int m_syncInterestReexpress = 4; // seconds

  static const int m_defaultRecoveryRetransmitInterval = 200; // milliseconds
  uint32_t m_recoveryRetransmissionInterval; // milliseconds
  
  ndn::EventId m_delayedInterestProcessingId;
  ndn::EventId m_reexpressingInterestId;
  ndn::EventId m_reexpressingRecoveryInterestId;
  
  std::string m_instanceId;
  static int m_instanceCounter;
};


} // Sync

#endif // SYNC_APP_WRAPPER_H
