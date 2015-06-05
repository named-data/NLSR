/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
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
  for (std::list<RoutingTableEntry>::iterator it = m_rteList.begin();
       it != m_rteList.end(); ++it)
  {
    for (std::list<NextHop>::iterator nhit =
           (*it).getNexthopList().getNextHops().begin();
         nhit != (*it).getNexthopList().getNextHops().end(); ++nhit)
    {
      m_nexthopList.addNextHop((*nhit));
    }
  }
}



static bool
rteCompare(RoutingTableEntry& rte, ndn::Name& destRouter)
{
  return rte.getDestination() == destRouter;
}

void
NamePrefixTableEntry::removeRoutingTableEntry(RoutingTableEntry& rte)
{
  std::list<RoutingTableEntry>::iterator it = std::find_if(m_rteList.begin(),
                                                           m_rteList.end(),
                                                           bind(&rteCompare, _1, rte.getDestination()));
  if (it != m_rteList.end())
  {
    m_rteList.erase(it);
  }
}

void
NamePrefixTableEntry::addRoutingTableEntry(RoutingTableEntry& rte)
{
  std::list<RoutingTableEntry>::iterator it = std::find_if(m_rteList.begin(),
                                                           m_rteList.end(),
                                                           bind(&rteCompare, _1, rte.getDestination()));
  if (it == m_rteList.end())
  {
    m_rteList.push_back(rte);
  }
  else
  {
    (*it).getNexthopList().reset(); // reseting existing routing table's next hop
    for (std::list<NextHop>::iterator nhit =
           rte.getNexthopList().getNextHops().begin();
         nhit != rte.getNexthopList().getNextHops().end(); ++nhit) {
      (*it).getNexthopList().addNextHop((*nhit));
    }
  }
}

void
NamePrefixTableEntry::writeLog()
{
  _LOG_DEBUG("Name: " << m_namePrefix);
  for (std::list<RoutingTableEntry>::iterator it = m_rteList.begin();
       it != m_rteList.end(); ++it) {
    _LOG_DEBUG("Destination: " << (*it).getDestination());
    _LOG_DEBUG("Nexthops: ");
    (*it).getNexthopList().writeLog();
  }
  m_nexthopList.writeLog();
}

std::ostream&
operator<<(std::ostream& os, const NamePrefixTableEntry& entry)
{
  os << "Name: " << entry.getNamePrefix() << "\n";

  for (const RoutingTableEntry& rte : entry.getRteList()) {
    os << "Destination: " << rte.getDestination() << "\n";
  }

  return os;
}

} // namespace nlsr
