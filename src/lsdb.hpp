/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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

#include <utility>
#include <boost/cstdint.hpp>

#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/time.hpp>

#include "conf-parameter.hpp"
#include "lsa.hpp"
#include "test-access-control.hpp"

namespace nlsr {

using namespace ndn::time;

class Nlsr;
class SyncLogicHandler;

class Lsdb
{
public:
  Lsdb(Nlsr& nlsr, ndn::Scheduler& scheduler, SyncLogicHandler& sync);

  bool
  doesLsaExist(const ndn::Name& key, const std::string& lsType);

  // functions related to Name LSDB
  bool
  buildAndInstallOwnNameLsa();

  NameLsa*
  findNameLsa(const ndn::Name& key);

  bool
  installNameLsa(NameLsa& nlsa);

  bool
  removeNameLsa(const ndn::Name& key);

  bool
  isNameLsaNew(const ndn::Name& key, uint64_t seqNo);

  void
  writeNameLsdbLog();

  const std::list<NameLsa>&
  getNameLsdb();

  // functions related to Cor LSDB
  bool
  buildAndInstallOwnCoordinateLsa();

  CoordinateLsa*
  findCoordinateLsa(const ndn::Name& key);

  bool
  installCoordinateLsa(CoordinateLsa& clsa);

  bool
  removeCoordinateLsa(const ndn::Name& key);

  bool
  isCoordinateLsaNew(const ndn::Name& key, uint64_t seqNo);

  void
  writeCorLsdbLog();

  const std::list<CoordinateLsa>&
  getCoordinateLsdb();

  // functions related to Adj LSDB
  void
  scheduleAdjLsaBuild();

  bool
  buildAndInstallOwnAdjLsa();

  bool
  removeAdjLsa(const ndn::Name& key);

  bool
  isAdjLsaNew(const ndn::Name& key, uint64_t seqNo);

  bool
  installAdjLsa(AdjLsa& alsa);

  AdjLsa*
  findAdjLsa(const ndn::Name& key);

  const std::list<AdjLsa>&
  getAdjLsdb();

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

  void
  writeAdjLsdbLog();

  void
  setLsaRefreshTime(const seconds& lsaRefreshTime);

  void
  setThisRouterPrefix(std::string trp);

  void
  expressInterest(const ndn::Name& interestName, uint32_t timeoutCount,
                  steady_clock::TimePoint deadline = DEFAULT_LSA_RETRIEVAL_DEADLINE);

  void
  processInterest(const ndn::Name& name, const ndn::Interest& interest);

private:
  bool
  addNameLsa(NameLsa& nlsa);

  bool
  doesNameLsaExist(const ndn::Name& key);


  bool
  addCoordinateLsa(CoordinateLsa& clsa);

  bool
  doesCoordinateLsaExist(const ndn::Name& key);

  void
  buildAdjLsa();

  bool
  addAdjLsa(AdjLsa& alsa);

  bool
  doesAdjLsaExist(const ndn::Name& key);

  ndn::EventId
  scheduleNameLsaExpiration(const ndn::Name& key, int seqNo,
                            const seconds& expTime);

  void
  exprireOrRefreshNameLsa(const ndn::Name& lsaKey, uint64_t seqNo);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  ndn::EventId
  scheduleAdjLsaExpiration(const ndn::Name& key, int seqNo,
                           const seconds& expTime);

private:
  void
  exprireOrRefreshAdjLsa(const ndn::Name& lsaKey, uint64_t seqNo);

  ndn::EventId
  scheduleCoordinateLsaExpiration(const ndn::Name& key, int seqNo,
                                  const seconds& expTime);

  void
  exprireOrRefreshCoordinateLsa(const ndn::Name& lsaKey,
                                uint64_t seqNo);

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
  onContentValidated(const ndn::shared_ptr<const ndn::Data>& data);

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
  /**
   * @brief Error callback when SegmentFetcher fails to return an LSA
   *
   * In all error cases, a reattempt to fetch the LSA will be made.
   *
   * Segment validation can fail either because the packet does not have a
   * valid signature (fatal) or because some of the certificates in the trust chain
   * could not be fetched (non-fatal).
   *
   * Currently, the library does not provide clear indication (besides a plain-text message
   * in the error callback) of the reason for the failure nor the segment that failed
   * to be validated, thus we will continue to try to fetch the LSA until the deadline
   * is reached.
   */
  void
  onFetchLsaError(uint32_t errorCode,
                  const std::string& msg,
                  ndn::Name& interestName,
                  uint32_t retransmitNo,
                  const ndn::time::steady_clock::TimePoint& deadline,
                  ndn::Name lsaName,
                  uint64_t seqNo);

  /**
   * @brief Success callback when SegmentFetcher returns a valid LSA
   *
   * \param The base Interest used to fetch the LSA in the format:
   *        /<network>/NLSR/LSA/<site>/%C1.Router/<router>/<lsa-type>/<seqNo>
   */
  void
  afterFetchLsa(const ndn::ConstBufferPtr& data, ndn::Name& interestName);

private:
  system_clock::TimePoint
  getLsaExpirationTimePoint();

  void
  cancelScheduleLsaExpiringEvent(ndn::EventId eid);

public:
  static const ndn::Name::Component NAME_COMPONENT;

private:
  Nlsr& m_nlsr;
  ndn::Scheduler& m_scheduler;
  SyncLogicHandler& m_sync;

  std::list<NameLsa> m_nameLsdb;
  std::list<AdjLsa> m_adjLsdb;
  std::list<CoordinateLsa> m_corLsdb;

  seconds m_lsaRefreshTime;
  std::string m_thisRouterPrefix;

  typedef std::map<ndn::Name, uint64_t> SequenceNumberMap;

  // Maps the name of an LSA to its highest known sequence number from sync;
  // Used to stop NLSR from trying to fetch outdated LSAs
  SequenceNumberMap m_highestSeqNo;

  static const ndn::time::seconds GRACE_PERIOD;
  static const steady_clock::TimePoint DEFAULT_LSA_RETRIEVAL_DEADLINE;

  ndn::time::seconds m_adjLsaBuildInterval;
};

} // namespace nlsr

#endif //NLSR_LSDB_HPP
