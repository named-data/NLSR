/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  The University of Memphis,
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

#include "name-prefix-table-entry.hpp"

#include "common.hpp"
#include "nexthop.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER(route.NamePrefixTableEntry);

void
NamePrefixTableEntry::generateNhlfromRteList()
{
  m_nexthopList.clear();
  for (auto iterator = m_rteList.begin(); iterator != m_rteList.end(); ++iterator) {
    for (auto nhItr = (*iterator)->getNexthopList().getNextHops().begin();
         nhItr != (*iterator)->getNexthopList().getNextHops().end();
         ++nhItr) {
      m_nexthopList.addNextHop((*nhItr));
    }
  }
}

uint64_t
NamePrefixTableEntry::removeRoutingTableEntry(std::shared_ptr<RoutingTablePoolEntry> entryPtr)
{
  auto iterator = std::find(m_rteList.begin(), m_rteList.end(), entryPtr);

  if (iterator != m_rteList.end()) {
    (*iterator)->decrementUseCount();
    // Remove this NamePrefixEntry from the RoutingTablePoolEntry
    (*iterator)->namePrefixTableEntries.erase(getNamePrefix());
    m_rteList.erase(iterator);
  }
  else {
    NLSR_LOG_ERROR("Routing entry for: " << entryPtr->getDestination()
               << " not found in NPT entry: " << getNamePrefix());
  }
  return entryPtr->getUseCount();
}

void
NamePrefixTableEntry::addRoutingTableEntry(std::shared_ptr<RoutingTablePoolEntry> entryPtr)
{
  auto iterator = std::find(m_rteList.begin(), m_rteList.end(), entryPtr);

  // Ensure that this is a new entry
  if (iterator == m_rteList.end()) {
    // Adding a new routing entry to the NPT entry
    entryPtr->incrementUseCount();
    m_rteList.push_back(entryPtr);
  }
  // Note: we don't need to update in the else case because these are
  // pointers, and they are centrally-located in the NPT and will all
  // be updated there.
}

bool
operator==(const NamePrefixTableEntry& lhs, const NamePrefixTableEntry& rhs)
{
  return lhs.getNamePrefix() == rhs.getNamePrefix();
}

bool
operator==(const NamePrefixTableEntry& lhs, const ndn::Name& rhs)
{
  return lhs.getNamePrefix() == rhs;
}

std::ostream&
operator<<(std::ostream& os, const NamePrefixTableEntry& entry)
{
  os << "Name: " << entry.getNamePrefix() << "\n";

  for (const auto& entryPtr : entry.getRteList()) {
    os << "  Destination: " << entryPtr->getDestination() << "\n";
    os << entryPtr->getNexthopList();
  }
  return os;
}

} // namespace nlsr
