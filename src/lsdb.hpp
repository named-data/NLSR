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

#ifndef NLSR_LSDB_HPP
#define NLSR_LSDB_HPP

#include "conf-parameter.hpp"
#include "lsa.hpp"
#include "sequencing-manager.hpp"
#include "test-access-control.hpp"
#include "communication/sync-logic-handler.hpp"
#include "statistics.hpp"

#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/signal.hpp>
#include <ndn-cxx/util/time.hpp>
#include <utility>
#include <boost/cstdint.hpp>

namespace nlsr {

class Nlsr;

class Lsdb
{
public:
  Lsdb(Nlsr& nlsr, ndn::Scheduler& scheduler);

  SyncLogicHandler&
  getSyncLogicHandler()
  {
    return m_sync;
  }

  bool
  isLsaNew(const ndn::Name& routerName, const Lsa::Type& lsaType, const uint64_t& sequenceNumber);

  bool
  doesLsaExist(const ndn::Name& key, const Lsa::Type& lsType);

  /*! \brief Builds a name LSA for this router and then installs it
      into the LSDB.
  */
  bool
  buildAndInstallOwnNameLsa();

  /*! \brief Returns the name LSA with the given key.
    \param key The name of the router that the desired LSA comes from.
  */
  NameLsa*
  findNameLsa(const ndn::Name& key);

  /*! \brief Installs a name LSA into the LSDB
    \param nlsa The name LSA to install into the LSDB.
  */
  bool
  installNameLsa(NameLsa& nlsa);

  /*! \brief Remove a name LSA from the LSDB.
    \param key The name of the router that published the LSA to remove.

    This function will remove a name LSA from the LSDB by finding an
    LSA whose name matches key. This removal also causes the NPT to
    remove those name prefixes if no more LSAs advertise them.
   */
  bool
  removeNameLsa(const ndn::Name& key);

  /*! Returns whether a seq. no. from a certain router signals a new LSA.
    \param key The name of the originating router.
    \param seqNo The sequence number to check.
  */
  bool
  isNameLsaNew(const ndn::Name& key, uint64_t seqNo);

  void
  writeNameLsdbLog();

  const std::list<NameLsa>&
  getNameLsdb() const;

  /*! \brief Builds a cor. LSA for this router and installs it into the LSDB. */
  bool
  buildAndInstallOwnCoordinateLsa();

  /*! \brief Finds a cor. LSA in the LSDB.
    \param key The name of the originating router that published the LSA.
  */
  CoordinateLsa*
  findCoordinateLsa(const ndn::Name& key);

  /*! \brief Installs a cor. LSA into the LSDB.
    \param clsa The cor. LSA to install.
  */
  bool
  installCoordinateLsa(CoordinateLsa& clsa);

  /*! \brief Removes a cor. LSA from the LSDB.
    \param key The name of the router that published the LSA to remove.

    Removes the coordinate LSA whose origin router name matches that
    given by key. Additionally, ask the NPT to remove the prefix,
    which will occur if no other LSAs point there.
  */
  bool
  removeCoordinateLsa(const ndn::Name& key);

  /*! \brief Returns whether a cor. LSA from a router is new or not.
    \param key The name prefix of the originating router.
    \param seqNo The sequence number of the candidate LSA.
  */
  bool
  isCoordinateLsaNew(const ndn::Name& key, uint64_t seqNo);

  void
  writeCorLsdbLog();

  const std::list<CoordinateLsa>&
  getCoordinateLsdb() const;

  //function related to Adj LSDB

  /*! \brief Schedules a build of this router's LSA. */
  void
  scheduleAdjLsaBuild();

  /*! \brief Wrapper event to build and install an adj. LSA for this router. */
  bool
  buildAndInstallOwnAdjLsa();

  /*! \brief Removes an adj. LSA from the LSDB.
    \param key The name of the publishing router whose LSA to remove.
  */
  bool
  removeAdjLsa(const ndn::Name& key);

  /*! \brief Returns whether an LSA is new.
    \param key The name of the publishing router.
    \param seqNo The seq. no. of the candidate LSA.

    This function determines whether the LSA with the name key and
    seq. no. seqNo would be new to this LSDB.
  */
  bool
  isAdjLsaNew(const ndn::Name& key, uint64_t seqNo);

  /*! \brief Installs an adj. LSA into the LSDB.
    \param alsa The adj. LSA to add to the LSDB.
  */
  bool
  installAdjLsa(AdjLsa& alsa);

  /*! \brief Finds an adj. LSA in the LSDB.
    \param key The name of the publishing router whose LSA to find.
   */
  AdjLsa*
  findAdjLsa(const ndn::Name& key);

  const std::list<AdjLsa>&
  getAdjLsdb() const;

  void
  setAdjLsaBuildInterval(uint32_t interval)
  {
    m_adjLsaBuildInterval = ndn::time::seconds(interval);
  }

  const ndn::time::seconds&
  getAdjLsaBuildInterval() const
  {
    return m_adjLsaBuildInterval;
  }

  SequencingManager&
  getSequencingManager()
  {
    return m_sequencingManager;
  }

  void
  writeAdjLsdbLog();

  void
  setLsaRefreshTime(const ndn::time::seconds& lsaRefreshTime);

  void
  setThisRouterPrefix(std::string trp);

  void
  expressInterest(const ndn::Name& interestName, uint32_t timeoutCount,
                  ndn::time::steady_clock::TimePoint deadline = DEFAULT_LSA_RETRIEVAL_DEADLINE);

  void
  processInterest(const ndn::Name& name, const ndn::Interest& interest);

private:
  /* \brief Add a name LSA to the LSDB if it isn't already there.
     \param nlsa The candidade name LSA.
  */
  bool
  addNameLsa(NameLsa& nlsa);

  /*! \brief Returns whether the LSDB contains some LSA.
    \param key The name of the publishing router whose LSA to check for.
   */
  bool
  doesNameLsaExist(const ndn::Name& key);

  /*! \brief Adds a cor. LSA to the LSDB if it isn't already there.
    \param clsa The candidate cor. LSA.
  */
  bool
  addCoordinateLsa(CoordinateLsa& clsa);

  /*! \brief Returns whether a cor. LSA is in the LSDB.
    \param key The name of the router that published the queried LSA.
   */
  bool
  doesCoordinateLsaExist(const ndn::Name& key);

  /*! \brief Attempts to construct an adj. LSA.

    This function will attempt to construct an adjacency LSA. An LSA
    can only be built when the status of all of the router's neighbors
    is known. That is, when we are not currently trying to contact any
    neighbor.
   */
  void
  buildAdjLsa();

  /*! \brief Adds an adj. LSA to the LSDB if it isn't already there.
    \param alsa The candidate adj. LSA to add to the LSDB.
  */
  bool
  addAdjLsa(AdjLsa& alsa);

  /*! \brief Returns whether the LSDB contains an LSA.
    \param key The name of a router whose LSA to check for in the LSDB.
  */
  bool
  doesAdjLsaExist(const ndn::Name& key);

  /*! \brief Schedules a refresh/expire event in the scheduler.
    \param key The name of the router that published the LSA.
    \param seqNo The seq. no. associated with the LSA.
    \param expTime How many seconds to wait before triggering the event.
   */
  ndn::EventId
  scheduleNameLsaExpiration(const ndn::Name& key, int seqNo,
                            const ndn::time::seconds& expTime);

  /*! \brief Either allow to expire, or refresh a name LSA.
    \param lsaKey The name of the router that published the LSA.
    \param seqNo The seq. no. of the LSA to check.
  */
  void
  expireOrRefreshNameLsa(const ndn::Name& lsaKey, uint64_t seqNo);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  /*! \brief Schedules an expire/refresh event in the LSA.
    \param key The name of the router whose LSA is in question.
    \param seqNo The sequence number of the LSA to check.
    \param expTime The number of seconds to wait before triggering the event.
  */
  ndn::EventId
  scheduleAdjLsaExpiration(const ndn::Name& key, int seqNo,
                           const ndn::time::seconds& expTime);

private:

  void
  expireOrRefreshAdjLsa(const ndn::Name& lsaKey, uint64_t seqNo);

  ndn::EventId
  scheduleCoordinateLsaExpiration(const ndn::Name& key, int seqNo,
                                  const ndn::time::seconds& expTime);

  void
  expireOrRefreshCoordinateLsa(const ndn::Name& lsaKey,
                                uint64_t seqNo);

private:

  void
  putLsaData(const ndn::Interest& interest, const std::string& content);

  void
  processInterestForNameLsa(const ndn::Interest& interest,
                            const ndn::Name& lsaKey,
                            uint64_t seqNo);

  void
  processInterestForAdjacencyLsa(const ndn::Interest& interest,
                                 const ndn::Name& lsaKey,
                                 uint64_t seqNo);

  void
  processInterestForCoordinateLsa(const ndn::Interest& interest,
                                  const ndn::Name& lsaKey,
                                  uint64_t seqNo);
  void
  onContentValidated(const std::shared_ptr<const ndn::Data>& data);

  void
  processContentNameLsa(const ndn::Name& lsaKey,
                        uint64_t lsSeqNo, std::string& dataContent);

  void
  processContentAdjacencyLsa(const ndn::Name& lsaKey,
                             uint64_t lsSeqNo, std::string& dataContent);

  void
  processContentCoordinateLsa(const ndn::Name& lsaKey,
                              uint64_t lsSeqNo, std::string& dataContent);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  /*!
     \brief Error callback when SegmentFetcher fails to return an LSA

     In all error cases, a reattempt to fetch the LSA will be made.

     Segment validation can fail either because the packet does not have a
     valid signature (fatal) or because some of the certificates in the trust chain
     could not be fetched (non-fatal).

     Currently, the library does not provide clear indication (besides a plain-text message
     in the error callback) of the reason for the failure nor the segment that failed
     to be validated, thus we will continue to try to fetch the LSA until the deadline
     is reached.
   */
  void
  onFetchLsaError(uint32_t errorCode,
                  const std::string& msg,
                  ndn::Name& interestName,
                  uint32_t retransmitNo,
                  const ndn::time::steady_clock::TimePoint& deadline,
                  ndn::Name lsaName,
                  uint64_t seqNo);

  /*!
     \brief Success callback when SegmentFetcher returns a valid LSA

     \param interestName The base Interest used to fetch the LSA in the format:
            /<network>/NLSR/LSA/<site>/%C1.Router/<router>/<lsa-type>/<seqNo>
   */
  void
  afterFetchLsa(const ndn::ConstBufferPtr& data, ndn::Name& interestName);

private:
  ndn::time::system_clock::TimePoint
  getLsaExpirationTimePoint();

  /*! \brief Cancels an event in the event scheduler. */
  void
  cancelScheduleLsaExpiringEvent(ndn::EventId eid);

public:
  static const ndn::Name::Component NAME_COMPONENT;

  ndn::util::signal::Signal<Lsdb, Statistics::PacketType> lsaIncrementSignal;

private:
  Nlsr& m_nlsr;
  ndn::Scheduler& m_scheduler;
  SyncLogicHandler m_sync;

  std::list<NameLsa> m_nameLsdb;
  std::list<AdjLsa> m_adjLsdb;
  std::list<CoordinateLsa> m_corLsdb;

  ndn::time::seconds m_lsaRefreshTime;
  std::string m_thisRouterPrefix;

  typedef std::map<ndn::Name, uint64_t> SequenceNumberMap;

  // Maps the name of an LSA to its highest known sequence number from sync;
  // Used to stop NLSR from trying to fetch outdated LSAs
  SequenceNumberMap m_highestSeqNo;

  static const ndn::time::seconds GRACE_PERIOD;
  static const ndn::time::steady_clock::TimePoint DEFAULT_LSA_RETRIEVAL_DEADLINE;

  ndn::time::seconds m_adjLsaBuildInterval;

  SequencingManager m_sequencingManager;

  ndn::util::signal::ScopedConnection m_onNewLsaConnection;

};

} // namespace nlsr

#endif // NLSR_LSDB_HPP
