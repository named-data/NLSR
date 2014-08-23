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
#ifndef NLSR_FIB_ENTRY_HPP
#define NLSR_FIB_ENTRY_HPP

#include <list>
#include <iostream>
#include <boost/cstdint.hpp>

#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/time.hpp>

#include "nexthop.hpp"
#include "nexthop-list.hpp"

namespace nlsr {

class FibEntry
{
public:
  FibEntry()
    : m_name()
    , m_expirationTimePoint()
    , m_seqNo(0)
    , m_nexthopList()
  {
  }

  FibEntry(const ndn::Name& name)
    : m_expirationTimePoint()
    , m_seqNo(0)
    , m_nexthopList()
  {
    m_name = name;
  }

  const ndn::Name&
  getName() const
  {
    return m_name;
  }

  NexthopList&
  getNexthopList()
  {
    return m_nexthopList;
  }

  const ndn::time::system_clock::TimePoint&
  getExpirationTimePoint() const
  {
    return m_expirationTimePoint;
  }

  void
  setExpirationTimePoint(const ndn::time::system_clock::TimePoint& ttr)
  {
    m_expirationTimePoint = ttr;
  }

  void
  setExpiringEventId(ndn::EventId feid)
  {
    m_expiringEventId = feid;
  }

  ndn::EventId
  getExpiringEventId() const
  {
    return m_expiringEventId;
  }

  void
  setSeqNo(int32_t fsn)
  {
    m_seqNo = fsn;
  }

  int32_t
  getSeqNo()
  {
    return m_seqNo;
  }

  void
  writeLog();

private:
  ndn::Name m_name;
  ndn::time::system_clock::TimePoint m_expirationTimePoint;
  ndn::EventId m_expiringEventId;
  int32_t m_seqNo;
  NexthopList m_nexthopList;
};

} //namespace nlsr

#endif //NLSR_FIB_ENTRY_HPP
