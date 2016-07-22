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

#include "name-prefix-table-entry.hpp"

#include "common.hpp"
#include "nexthop.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("NamePrefixTableEntry");

void
NamePrefixTableEntry::generateNhlfromRteList()
{
  m_nexthopList.reset();
  for (auto rtpeItr = m_rteList.begin(); rtpeItr != m_rteList.end(); ++rtpeItr) {
    for (auto nhItr = (*rtpeItr)->getNexthopList().getNextHops().begin();
         nhItr != (*rtpeItr)->getNexthopList().getNextHops().end();
         ++nhItr) {
      m_nexthopList.addNextHop((*nhItr));
    }
  }
}

uint64_t
NamePrefixTableEntry::removeRoutingTableEntry(shared_ptr<RoutingTablePoolEntry>
                                              rtpePtr)
{
  auto rtpeItr = std::find(m_rteList.begin(), m_rteList.end(), rtpePtr);

  if (rtpeItr != m_rteList.end()) {
    (*rtpeItr)->decrementUseCount();
    m_rteList.erase(rtpeItr);
  }
  else {
    _LOG_ERROR("Routing entry for: " << rtpePtr->getDestination()
               << " not found in NPT entry: " << getNamePrefix());
  }
  return (*rtpeItr)->getUseCount();
}

void
NamePrefixTableEntry::addRoutingTableEntry(shared_ptr<RoutingTablePoolEntry>
                                           rtpePtr)
{
  auto rtpeItr = std::find(m_rteList.begin(), m_rteList.end(), rtpePtr);

  // Ensure that this is a new entry
  if (rtpeItr == m_rteList.end()) {
    // Adding a new routing entry to the NPT entry
    rtpePtr->incrementUseCount();
    m_rteList.push_back(rtpePtr);
  }
  // Note: we don't need to update in the else case because these are
  // pointers, and they are centrally-located in the NPT and will all
  // be updated there.
}

void
NamePrefixTableEntry::writeLog()
{
  _LOG_DEBUG("Name: " << m_namePrefix);
  for (auto it = m_rteList.begin(); it != m_rteList.end(); ++it) {
    _LOG_DEBUG("Destination: " << (*it)->getDestination());
    _LOG_DEBUG("Nexthops: ");
    (*it)->getNexthopList().writeLog();
  }
  m_nexthopList.writeLog();
}

bool
operator==(const NamePrefixTableEntry& lhs, const NamePrefixTableEntry& rhs)
{
  return (lhs.getNamePrefix() == rhs.getNamePrefix());
}

bool
operator==(const NamePrefixTableEntry& lhs, const ndn::Name& rhs)
{
  return (lhs.getNamePrefix() == rhs);
}

std::ostream&
operator<<(std::ostream& os, const NamePrefixTableEntry& entry)
{
  os << "Name: " << entry.getNamePrefix() << "\n";

  for (const shared_ptr<RoutingTablePoolEntry> rtpePtr : entry.getRteList()) {
    os << "Destination: " << rtpePtr->getDestination() << "\n";
  }

  return os;
}

} // namespace nlsr
