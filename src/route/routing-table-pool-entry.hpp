/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022,  The University of Memphis,
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
 */

#ifndef NLSR_ROUTING_TABLE_POOL_ENTRY_HPP
#define NLSR_ROUTING_TABLE_POOL_ENTRY_HPP

#include "routing-table-entry.hpp"
#include "nexthop-list.hpp"

#include <ndn-cxx/name.hpp>
#include <unordered_map>

namespace nlsr {

/*! \brief A deduplication system for the NamePrefixTable
 *
 * The NamePrefixTable associates name prefixes to a router. To do
 * this, it needs to know if certain routers are reachable. This in
 * turn requires access to entries from the RoutingTable, which are
 * associated with name prefixes. Doing this naively copies the entry
 * from the RoutingTable each time, which is costly. This class
 * provides a deduplication system where the NamePrefixTable can
 * maintain a collection of RoutingTablePoolEntries. Then, this new
 * class can be associated with the name prefixes instead of the
 * original entries, which provides a minimal memory solution.
 *
 * \sa NamePrefixTable
 */
class NamePrefixTableEntry;

class RoutingTablePoolEntry : public RoutingTableEntry
{
public:
  RoutingTablePoolEntry() = default;

  RoutingTablePoolEntry(const ndn::Name& dest)
  {
    m_destination = dest;
    m_useCount = 1;
  }

  RoutingTablePoolEntry(RoutingTableEntry& rte, uint64_t useCount)
  {
    m_destination = rte.getDestination();
    m_nexthopList = rte.getNexthopList();
    m_useCount = useCount;
  }

  RoutingTablePoolEntry(const ndn::Name& dest, uint64_t useCount)
  {
    m_destination = dest;
    m_useCount = useCount;
  }

  uint64_t
  getUseCount() const
  {
    return m_useCount;
  }

  uint64_t
  incrementUseCount()
  {
    return ++m_useCount;
  }

  uint64_t
  decrementUseCount()
  {
    if (m_useCount != 0) {
      return --m_useCount;
    }
    return 0;
  }

  void
  setNexthopList(NexthopList nhl)
  {
    m_nexthopList = std::move(nhl);
  }

public:
  std::unordered_map<ndn::Name, std::weak_ptr<NamePrefixTableEntry>> namePrefixTableEntries;

private:
  uint64_t m_useCount;
};

bool
operator==(const RoutingTablePoolEntry& lhs, const RoutingTablePoolEntry& rhs);

std::ostream&
operator<<(std::ostream& os, RoutingTablePoolEntry& rtpe);

} // namespace nlsr

#endif // NLSR_ROUTING_TABLE_POOL_ENTRY_HPP
