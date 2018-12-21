/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
 *                           Regents of the University of California
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

#ifndef NLSR_FIB_ENTRY_HPP
#define NLSR_FIB_ENTRY_HPP

#include "nexthop-list.hpp"

#include <ndn-cxx/util/scheduler.hpp>

namespace nlsr {

class FibEntry
{
public:
  FibEntry()
    : m_name()
    , m_seqNo(1)
    , m_nexthopList()
  {
  }

  FibEntry(const ndn::Name& name)
    : m_seqNo(1)
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

  void
  setRefreshEventId(ndn::EventId id)
  {
    m_refreshEventId = id;
  }

  ndn::EventId
  getRefreshEventId() const
  {
    return m_refreshEventId;
  }

  void
  setSeqNo(int32_t fsn)
  {
    m_seqNo = fsn;
  }

  int32_t
  getSeqNo() const
  {
    return m_seqNo;
  }

  void
  writeLog() const;

  typedef NexthopList::const_iterator const_iterator;

  const_iterator
  begin() const;

  const_iterator
  end() const;

private:
  ndn::Name m_name;
  ndn::EventId m_refreshEventId;
  int32_t m_seqNo;
  NexthopList m_nexthopList;
};

inline FibEntry::const_iterator
FibEntry::begin() const
{
  return m_nexthopList.cbegin();
}

inline FibEntry::const_iterator
FibEntry::end() const
{
  return m_nexthopList.cend();
}

} // namespace nlsr

#endif // NLSR_FIB_ENTRY_HPP
