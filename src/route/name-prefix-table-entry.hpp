/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
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
 */

#ifndef NLSR_NAME_PREFIX_TABLE_ENTRY_HPP
#define NLSR_NAME_PREFIX_TABLE_ENTRY_HPP

#include "routing-table-pool-entry.hpp"
#include "test-access-control.hpp"
#include "nexthop.hpp"

#include <list>
#include <utility>

namespace nlsr {

class NamePrefixTableEntry
{
public:
  NamePrefixTableEntry()
  {
  }

  NamePrefixTableEntry(const ndn::Name& namePrefix)
    : m_namePrefix(namePrefix)
    , m_nexthopList()
  {
  }

  const ndn::Name&
  getNamePrefix() const
  {
    return m_namePrefix;
  }

  const std::list<std::shared_ptr<RoutingTablePoolEntry>>&
  getRteList() const
  {
    return m_rteList;
  }

  /*! \brief Resets the next hop lists of all routing table entries
   * that advertise this name prefix.
   */
  void
  resetRteListNextHop()
  {
    if (m_rteList.size() > 0) {
      for (auto it = m_rteList.begin(); it != m_rteList.end(); ++it) {
        (*it)->getNexthopList().clear();
      }
    }
  }

  size_t
  getRteListSize()
  {
    return m_rteList.size();
  }

  const NexthopList&
  getNexthopList() const
  {
    return m_nexthopList;
  }

  NexthopList&
  getNexthopListForModification()
  {
    return m_nexthopList;
  }

  /*! \brief Collect all next-hops that are advertised by this entry's
   * routing entries.
   */
  void
  generateNhlfromRteList();

  /*! \brief Removes a routing entry from this NPT entry.
   * \return The number of NPTs using the just-removed routing entry.
   */
  uint64_t
  removeRoutingTableEntry(std::shared_ptr<RoutingTablePoolEntry> rtpePtr);

  /*! \brief Adds a routing entry to this NPT entry.
   * \param rtpePtr The routing entry.
   *
   * Adds a routing table pool entry to this NPT entry's list
   * (reminder: each RTPE has a next-hop list). They are used to
   * calculate this entry's overall next-hop list.
   */
  void
  addRoutingTableEntry(std::shared_ptr<RoutingTablePoolEntry> rtpePtr);

  void
  writeLog();

private:
  ndn::Name m_namePrefix;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::list<std::shared_ptr<RoutingTablePoolEntry>> m_rteList;
  NexthopList m_nexthopList;
};

bool
operator==(const NamePrefixTableEntry& lhs, const NamePrefixTableEntry& rhs);

bool
operator==(const NamePrefixTableEntry& lhs, const ndn::Name& rhs);

std::ostream&
operator<<(std::ostream& os, const NamePrefixTableEntry& entry);

} // namespace nlsr

#endif // NLSR_NAME_PREFIX_TABLE_ENTRY_HPP
