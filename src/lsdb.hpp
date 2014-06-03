/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#ifndef NLSR_LSDB_HPP
#define NLSR_LSDB_HPP

#include <utility>
#include <boost/cstdint.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/time.hpp>

#include "lsa.hpp"

namespace nlsr {
class Nlsr;

class Lsdb
{
public:
  Lsdb(Nlsr& nlsr)
    : m_nlsr(nlsr)
    , m_lsaRefreshTime(0)
  {
  }


  bool
  doesLsaExist(const ndn::Name& key, const std::string& lsType);
  // function related to Name LSDB

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

  //function related to Cor LSDB
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

  //function related to Adj LSDB
  void
  scheduledAdjLsaBuild();

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

  std::list<AdjLsa>&
  getAdjLsdb();

  void
  writeAdjLsdbLog();

  void
  setLsaRefreshTime(int lrt);

  void
  setThisRouterPrefix(std::string trp);

private:
  bool
  addNameLsa(NameLsa& nlsa);

  bool
  doesNameLsaExist(const ndn::Name& key);


  bool
  addCoordinateLsa(CoordinateLsa& clsa);

  bool
  doesCoordinateLsaExist(const ndn::Name& key);

  bool
  addAdjLsa(AdjLsa& alsa);

  bool
  doesAdjLsaExist(const ndn::Name& key);

  ndn::EventId
  scheduleNameLsaExpiration(const ndn::Name& key, int seqNo,
                            const ndn::time::seconds& expTime);

  void
  exprireOrRefreshNameLsa(const ndn::Name& lsaKey, uint64_t seqNo);

  ndn::EventId
  scheduleAdjLsaExpiration(const ndn::Name& key, int seqNo,
                           const ndn::time::seconds& expTime);

  void
  exprireOrRefreshAdjLsa(const ndn::Name& lsaKey, uint64_t seqNo);

  ndn::EventId
  scheduleCoordinateLsaExpiration(const ndn::Name& key, int seqNo,
                                  const ndn::time::seconds& expTime);

  void
  exprireOrRefreshCoordinateLsa(const ndn::Name& lsaKey,
                                uint64_t seqNo);
public:
  void
  expressInterest(const ndn::Name& interestName, uint32_t interestLifeTime,
                  uint32_t timeoutCount);

  void
  processInterest(const ndn::Name& name, const ndn::Interest& interest);

private:
  void
  processInterestForNameLsa(const ndn::Interest& interest,
                            const ndn::Name& lsaKey,
                            uint32_t interestedlsSeqNo);

  void
  processInterestForAdjacencyLsa(const ndn::Interest& interest,
                                 const ndn::Name& lsaKey,
                                 uint32_t interestedlsSeqNo);

  void
  processInterestForCoordinateLsa(const ndn::Interest& interest,
                                  const ndn::Name& lsaKey,
                                  uint32_t interestedlsSeqNo);

  void
  onContent(const ndn::Interest& interest, const ndn::Data& data);

  void
  onContentValidated(const ndn::shared_ptr<const ndn::Data>& data);

  void
  onContentValidationFailed(const ndn::shared_ptr<const ndn::Data>& data, const std::string& msg);

  void
  processContentNameLsa(const ndn::Name& lsaKey,
                        uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentAdjacencyLsa(const ndn::Name& lsaKey,
                             uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentCoordinateLsa(const ndn::Name& lsaKey,
                              uint32_t lsSeqNo, std::string& dataContent);

  void
  processInterestTimedOut(const ndn::Interest& interest, uint32_t timeoutCount);

  ndn::time::system_clock::TimePoint
  getLsaExpirationTimePoint();

private:
  void
  cancelScheduleLsaExpiringEvent(ndn::EventId eid);

  Nlsr& m_nlsr;
  std::list<NameLsa> m_nameLsdb;
  std::list<AdjLsa> m_adjLsdb;
  std::list<CoordinateLsa> m_corLsdb;

  int m_lsaRefreshTime;
  std::string m_thisRouterPrefix;

};

}//namespace nlsr

#endif //NLSR_LSDB_HPP
