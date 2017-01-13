/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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

bool
npteCompare(NamePrefixTableEntry& npte, const ndn::Name& name)
{
  return npte.getNamePrefix() == name;
}

void
NamePrefixTable::addEntry(const ndn::Name& name, const ndn::Name& destRouter)
{

  // Check if the advertised name prefix is in the table already.
  NptEntryList::iterator nameItr = std::find(m_table.begin(),
                                           m_table.end(),
                                           name);

  // Attempt to find a routing table pool entry (RTPE) we can use.
  RtpEntryMap::iterator rtpeItr = m_rtpool.find(destRouter);

  // These declarations just to make the compiler happy...
  RoutingTablePoolEntry rtpe;
  std::shared_ptr<RoutingTablePoolEntry> rtpePtr(nullptr);

  // There isn't currently a routing table entry in the pool for this name
  if (rtpeItr == m_rtpool.end()) {
    // See if there is a routing table entry available we could use
    RoutingTableEntry* routeEntryPtr = m_nlsr.getRoutingTable()
                                        .findRoutingTableEntry(destRouter);

    // We have to create a new routing table entry
    if (routeEntryPtr == nullptr) {
      rtpe = RoutingTablePoolEntry(destRouter, 0);
    }
    // There was already a usable one in the routing table
    else {
      rtpe = RoutingTablePoolEntry(*routeEntryPtr, 0);
    }

    // Add the new pool object to the pool.
    rtpePtr = addRtpeToPool(rtpe);
  }
  // There was one already, so just fetch that one.
  else {
    rtpePtr = (*rtpeItr).second;
  }

  // Either we have to make a new NPT entry or there already was one.
  if (nameItr == m_table.end()) {
    _LOG_DEBUG("Adding origin: " << rtpePtr->getDestination()
               << " to a new name prefix: " << name);
    NamePrefixTableEntry npte(name);
    npte.addRoutingTableEntry(rtpePtr);
    npte.generateNhlfromRteList();
    m_table.push_back(npte);

    // If this entry has next hops, we need to inform the FIB
    if (npte.getNexthopList().getSize() > 0) {
      _LOG_TRACE("Updating FIB with next hops for " << npte);
      m_nlsr.getFib().update(name, npte.getNexthopList());
    }
    // The routing table may recalculate and add a routing table entry
    // with no next hops to replace an existing routing table entry. In
    // this case, the name prefix is no longer reachable through a next
    // hop and should be removed from the FIB. But, the prefix should
    // remain in the Name Prefix Table as a future routing table
    // calculation may add next hops.
    else {
      _LOG_TRACE(npte << " has no next hops; removing from FIB");
      m_nlsr.getFib().remove(name);
    }
  }
  else {
    _LOG_TRACE("Adding origin: " << rtpePtr->getDestination()
               << " to existing prefix: " << *nameItr);
    nameItr->addRoutingTableEntry(rtpePtr);
    nameItr->generateNhlfromRteList();

    if (nameItr->getNexthopList().getSize() > 0) {
      _LOG_TRACE("Updating FIB with next hops for " << (*nameItr));
      m_nlsr.getFib().update(name, nameItr->getNexthopList());
    }
    else {
      _LOG_TRACE((*nameItr) << " has no next hops; removing from FIB");
      m_nlsr.getFib().remove(name);
    }
  }
}

void
NamePrefixTable::removeEntry(const ndn::Name& name, const ndn::Name& destRouter)
{
  _LOG_DEBUG("Removing origin: " << destRouter << " from " << name);

  // Fetch an iterator to the appropriate pair object in the pool.
  RtpEntryMap::iterator rtpeItr = m_rtpool.find(destRouter);

  // Simple error checking to prevent any unusual behavior in the case
  // that we try to remove an entry that isn't there.
  if (rtpeItr == m_rtpool.end()) {
    _LOG_DEBUG("No entry for origin: " << destRouter
               << " found, so it cannot be removed from prefix: "
               << name);
    return;
  }
  std::shared_ptr<RoutingTablePoolEntry> rtpePtr = rtpeItr->second;

  // Ensure that the entry exists
  NptEntryList::iterator nameItr = std::find_if(m_table.begin(),
                                           m_table.end(),
                                           bind(&npteCompare, _1, name));
  if (nameItr != m_table.end()) {
    _LOG_TRACE("Removing origin: " << rtpePtr->getDestination()
               << " from prefix: " << *nameItr);

    // Rather than iterating through the whole list periodically, just
    // delete them here if they have no references.
    if ((*nameItr).removeRoutingTableEntry(rtpePtr) == 0) {
      deleteRtpeFromPool(rtpePtr);
    }

    // If the prefix is a router prefix and it does not have any other
    // routing table entries, the Adjacency/Coordinate LSA associated
    // with that origin router has been removed from the LSDB and so
    // the router prefix should be removed from the Name Prefix Table.
    //
    // If the prefix is an advertised name prefix: If another router
    //   advertises this name prefix, the RteList should have another
    //   entry for that router; the next hops should be recalculated
    //   and installed in the FIB.
    //
    //   If no other router advertises this name prefix, the RteList
    //   should be empty and the prefix can be removed from the Name
    //   Prefix Table. Once a new Name LSA advertises this prefix, a
    //   new entry for the prefix will be created.
    //
    if ((*nameItr).getRteListSize() == 0) {
      _LOG_TRACE(*nameItr << " has no routing table entries;"
                 << " removing from table and FIB");
      m_table.erase(nameItr);
      m_nlsr.getFib().remove(name);
    }
    else {
      _LOG_TRACE(*nameItr << " has other routing table entries;"
                 << " updating FIB with next hops");
      (*nameItr).generateNhlfromRteList();
      m_nlsr.getFib().update(name, (*nameItr).getNexthopList());
    }
  }
  else {
    _LOG_DEBUG("Attempted to remove origin: " << rtpePtr->getDestination()
               << " from non-existant prefix: " << name);
  }
}

void
NamePrefixTable::updateWithNewRoute()
{
  _LOG_DEBUG("Updating table with newly calculated routes");

  // Update each name prefix table entry in the NPT with the
  // newly calculated next hops.
  for (auto&& npte : m_table) {
    // For each routing table pool entry in this NPT entry.
    for (auto&& rtpe : npte.getRteList()) {
      _LOG_TRACE("Updating next hops to origin: " << rtpe->getDestination()
                 << " for prefix: " << npte);
      RoutingTableEntry* rteCheck =
        m_nlsr.getRoutingTable().findRoutingTableEntry(rtpe->getDestination());

      // If there is a routing table entry for this prefix, update the NPT with it.
      if (rteCheck != nullptr) {
        rtpe->setNexthopList(rteCheck->getNexthopList());
      }
      else {
        rtpe->getNexthopList().reset();
      }
      addEntry(npte.getNamePrefix(), rtpe->getDestination());
    }
  }
}

  // Inserts the routing table pool entry into the NPT's RTE storage
  // pool.  This cannot fail, so the pool is guaranteed to contain the
  // item after this occurs.
std::shared_ptr<RoutingTablePoolEntry>
NamePrefixTable::addRtpeToPool(RoutingTablePoolEntry& rtpe)
{
  RtpEntryMap::iterator poolItr =
    m_rtpool.insert(std::make_pair(rtpe.getDestination(),
                                   std::make_shared<RoutingTablePoolEntry>
                                   (rtpe)))
    .first;
  //| There's gotta be a more efficient way to do this
  //std::shared_ptr<RoutingTablePoolEntry> poolPtr = &(poolItr->second);
  return poolItr->second;
}

  // Removes the routing table pool entry from the storage pool. The
  // postconditions of this function are guaranteed to include that
  // the storage pool does not contain such an item. Additionally,
  // this function cannot fail, but nonetheless debug information is
  // given in the case that this function is called with an entry that
  // isn't in the pool.
void
NamePrefixTable::deleteRtpeFromPool(std::shared_ptr<RoutingTablePoolEntry> rtpePtr)
{
  if (m_rtpool.erase(rtpePtr->getDestination()) != 1) {
    _LOG_DEBUG("Attempted to delete non-existant origin: "
               << rtpePtr->getDestination()
               << " from NPT routing table entry storage pool.");
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
