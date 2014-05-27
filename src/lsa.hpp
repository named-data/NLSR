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
#ifndef NLSR_LSA_HPP
#define NLSR_LSA_HPP

#include <boost/cstdint.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/time.hpp>

#include "adjacent.hpp"
#include "name-prefix-list.hpp"
#include "adjacency-list.hpp"

namespace nlsr {

class Lsa
{
public:
  Lsa()
    : m_origRouter()
    , m_lsSeqNo()
    , m_expirationTimePoint()
    , m_expiringEventId()
  {
  }


  void
  setLsType(const std::string& lst)
  {
    m_lsType = lst;
  }

  const std::string&
  getLsType() const
  {
    return m_lsType;
  }

  void
  setLsSeqNo(uint32_t lsn)
  {
    m_lsSeqNo = lsn;
  }

  uint32_t
  getLsSeqNo() const
  {
    return m_lsSeqNo;
  }

  const ndn::Name&
  getOrigRouter() const
  {
    return m_origRouter;
  }

  void
  setOrigRouter(const ndn::Name& org)
  {
    m_origRouter = org;
  }

  const ndn::time::system_clock::TimePoint&
  getExpirationTimePoint() const
  {
    return m_expirationTimePoint;
  }

  void
  setExpirationTimePoint(const ndn::time::system_clock::TimePoint& lt)
  {
    m_expirationTimePoint = lt;
  }

  void
  setExpiringEventId(const ndn::EventId leei)
  {
    m_expiringEventId = leei;
  }

  ndn::EventId
  getExpiringEventId() const
  {
    return m_expiringEventId;
  }

protected:
  ndn::Name m_origRouter;
  std::string m_lsType;
  uint32_t m_lsSeqNo;
  ndn::time::system_clock::TimePoint m_expirationTimePoint;
  ndn::EventId m_expiringEventId;
};

class NameLsa: public Lsa
{
public:
  NameLsa()
    : Lsa()
    , m_npl()
  {
    setLsType("name");
  }

  NameLsa(const ndn::Name& origR, const std::string& lst, uint32_t lsn,
          const ndn::time::system_clock::TimePoint& lt,
          NamePrefixList& npl);

  NamePrefixList&
  getNpl()
  {
    return m_npl;
  }

  void
  addName(const ndn::Name& name)
  {
    m_npl.insert(name);
  }

  void
  removeName(const ndn::Name& name)
  {
    m_npl.remove(name);
  }

  const ndn::Name
  getKey() const;

  std::string
  getData();

  bool
  initializeFromContent(const std::string& content);

  void
  writeLog();

private:
  NamePrefixList m_npl;

};

std::ostream&
operator<<(std::ostream& os, NameLsa& nLsa);

class AdjLsa: public Lsa
{
public:
  AdjLsa()
    : Lsa()
    , m_adl()
  {
    setLsType("adjacency");
  }

  AdjLsa(const ndn::Name& origR, const std::string& lst, uint32_t lsn,
         const ndn::time::system_clock::TimePoint& lt,
         uint32_t nl , AdjacencyList& adl);

  AdjacencyList&
  getAdl()
  {
    return m_adl;
  }

  void
  addAdjacent(Adjacent adj)
  {
    m_adl.insert(adj);
  }

  const ndn::Name
  getKey() const;

  std::string
  getData();

  bool
  initializeFromContent(const std::string& content);

  uint32_t
  getNoLink()
  {
    return m_noLink;
  }

  bool
  isEqualContent(AdjLsa& alsa);

  void
  addNptEntries(Nlsr& pnlsr);

  void
  removeNptEntries(Nlsr& pnlsr);

  void
  writeLog();

private:
  uint32_t m_noLink;
  AdjacencyList m_adl;
};

std::ostream&
operator<<(std::ostream& os, AdjLsa& aLsa);

class CoordinateLsa: public Lsa
{
public:
  CoordinateLsa()
    : Lsa()
    , m_corRad(0)
    , m_corTheta(0)
  {
    setLsType("coordinate");
  }

  CoordinateLsa(const ndn::Name& origR, const std::string lst, uint32_t lsn,
                const ndn::time::system_clock::TimePoint& lt,
                double r, double theta);

  const ndn::Name
  getKey() const;

  std::string
  getData();

  bool
  initializeFromContent(const std::string& content);

  double
  getCorRadius() const
  {
      return m_corRad;
  }

  void
  setCorRadius(double cr)
  {
      m_corRad = cr;
  }

  double
  getCorTheta() const
  {
    return m_corTheta;
  }

  void
  setCorTheta(double ct)
  {
    m_corTheta = ct;
  }

  bool
  isEqualContent(const CoordinateLsa& clsa);

  void
  writeLog();

private:
  double m_corRad;
  double m_corTheta;

};

std::ostream&
operator<<(std::ostream& os, const CoordinateLsa& cLsa);


}//namespace nlsr

#endif //NLSR_LSA_HPP
