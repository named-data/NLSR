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

#include "name-prefix-table.hpp"

#include "logger.hpp"
#include "nlsr.hpp"
#include "routing-table.hpp"

#include <algorithm>
#include <list>
#include <utility>

namespace nlsr {

INIT_LOGGER("NamePrefixTable");

static bool
npteCompare(NamePrefixTableEntry& npte, const ndn::Name& name)
{
  return npte.getNamePrefix() == name;
}

void
NamePrefixTable::addEntry(const ndn::Name& name, RoutingTableEntry& rte)
{
  NptEntryList::iterator it = std::find_if(m_table.begin(),
                                           m_table.end(),
                                           bind(&npteCompare, _1, name));
  if (it == m_table.end()) {
    _LOG_TRACE("Adding origin: " << rte.getDestination() << " to new name prefix: " << name);

    NamePrefixTableEntry entry(name);

    entry.addRoutingTableEntry(rte);

    entry.generateNhlfromRteList();
    entry.getNexthopList().sort();

    m_table.push_back(entry);

    if (rte.getNexthopList().getSize() > 0) {
      _LOG_TRACE("Updating FIB with next hops for " << entry);
      m_nlsr.getFib().update(name, entry.getNexthopList());
    }
  }
  else {
    _LOG_TRACE("Adding origin: " << rte.getDestination() << " to existing prefix: " << *it);

    it->addRoutingTableEntry(rte);

    it->generateNhlfromRteList();
    it->getNexthopList().sort();

    if (it->getNexthopList().getSize() > 0) {
      _LOG_TRACE("Updating FIB with next hops for " << *it);
      m_nlsr.getFib().update(name, it->getNexthopList());
    }
    else {
      // The routing table may recalculate and add a routing table entry with no next hops to
      // replace an existing routing table entry. In this case, the name prefix is no longer
      // reachable through a next hop and should be removed from the FIB. But, the prefix
      // should remain in the Name Prefix Table as a future routing table calculation may
      // add next hops.
      _LOG_TRACE(*it << " has no next hops; removing from FIB");
      m_nlsr.getFib().remove(name);
    }
  }
}

void
NamePrefixTable::removeEntry(const ndn::Name& name, RoutingTableEntry& rte)
{
  NptEntryList::iterator it = std::find_if(m_table.begin(),
                                           m_table.end(),
                                           bind(&npteCompare, _1, name));
  if (it != m_table.end()) {
    _LOG_TRACE("Removing origin: " << rte.getDestination() << " from prefix: " << *it);

    it->removeRoutingTableEntry(rte);

    // If the prefix is a router prefix and it does not have any other routing table entries,
    // the Adjacency/Coordinate LSA associated with that origin router has been removed from
    // the LSDB and so the router prefix should be removed from the Name Prefix Table.
    //
    // If the prefix is an advertised name prefix:
    //   If another router advertises this name prefix, the RteList should have another entry
    //   for that router; the next hops should be recalculated and installed in the FIB.
    //
    //   If no other router advertises this name prefix, the RteList should be empty and the
    //   prefix can be removed from the Name Prefix Table. Once a new Name LSA advertises this
    //   prefix, a new entry for the prefix will be created.
    //
    if (it->getRteListSize() == 0) {
      _LOG_TRACE(*it << " has no routing table entries; removing from table and FIB");
      m_table.erase(it);
      m_nlsr.getFib().remove(name);
    }
    else {
      _LOG_TRACE(*it << " has other routing table entries; updating FIB with next hops");
      it->generateNhlfromRteList();
      it->getNexthopList().sort();

      m_nlsr.getFib().update(name, it->getNexthopList());
    }
  }
}

void
NamePrefixTable::addEntry(const ndn::Name& name, const ndn::Name& destRouter)
{
  _LOG_DEBUG("Adding origin: " << destRouter << " to " << name);

  RoutingTableEntry* rteCheck = m_nlsr.getRoutingTable().findRoutingTableEntry(destRouter);

  if (rteCheck != nullptr) {
    addEntry(name, *rteCheck);
  }
  else {
    RoutingTableEntry rte(destRouter);
    addEntry(name, rte);
  }
}

void
NamePrefixTable::removeEntry(const ndn::Name& name, const ndn::Name& destRouter)
{
  _LOG_DEBUG("Removing origin: " << destRouter << " from " << name);

  RoutingTableEntry* rteCheck = m_nlsr.getRoutingTable().findRoutingTableEntry(destRouter);

  if (rteCheck != nullptr) {
    removeEntry(name, *rteCheck);
  }
  else {
    RoutingTableEntry rte(destRouter);
    removeEntry(name, rte);
  }
}

void
NamePrefixTable::updateWithNewRoute()
{
  _LOG_DEBUG("Updating table with newly calculated routes");

  // Update each name prefix entry in the Name Prefix Table with newly calculated next hops
  for (const NamePrefixTableEntry& prefixEntry : m_table) {
    for (const RoutingTableEntry& routingEntry : prefixEntry.getRteList()) {
      _LOG_TRACE("Updating next hops to origin: " << routingEntry.getDestination()
                                                  << " for prefix: " << prefixEntry);

      RoutingTableEntry* rteCheck =
        m_nlsr.getRoutingTable().findRoutingTableEntry(routingEntry.getDestination());

      if (rteCheck != nullptr) {
        addEntry(prefixEntry.getNamePrefix(), *rteCheck);
      }
      else {
        RoutingTableEntry rte(routingEntry.getDestination());
        addEntry(prefixEntry.getNamePrefix(), rte);
      }
    }
  }
}

void
NamePrefixTable::writeLog()
{
  _LOG_DEBUG(*this);
}

std::ostream&
operator<<(std::ostream& os, const NamePrefixTable& table)
{
  os << "----------------NPT----------------------\n";

  for (const NamePrefixTableEntry& entry : table) {
    os << entry << std::endl;
  }

  return os;
}

} // namespace nlsr
