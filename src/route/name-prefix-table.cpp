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

#include <list>
#include <utility>
#include <algorithm>

#include "nlsr.hpp"
#include "name-prefix-table.hpp"
#include "name-prefix-table-entry.hpp"
#include "routing-table.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("NamePrefixTable");

using namespace std;

static bool
npteCompare(NamePrefixTableEntry& npte, const ndn::Name& name)
{
  return npte.getNamePrefix() == name;
}



void
NamePrefixTable::addEntry(const ndn::Name& name, RoutingTableEntry& rte)
{
  std::list<NamePrefixTableEntry>::iterator it = std::find_if(m_table.begin(),
                                                              m_table.end(),
                                                              ndn::bind(&npteCompare, _1, name));
  if (it == m_table.end()) {
    NamePrefixTableEntry newEntry(name);
    newEntry.addRoutingTableEntry(rte);
    newEntry.generateNhlfromRteList();
    newEntry.getNexthopList().sort();
    m_table.push_back(newEntry);
    if (rte.getNexthopList().getSize() > 0) {
      m_nlsr.getFib().update(name, newEntry.getNexthopList());
    }
  }
  else {
    if (rte.getNexthopList().getSize() > 0) {
      (*it).addRoutingTableEntry(rte);
      (*it).generateNhlfromRteList();
      (*it).getNexthopList().sort();
      m_nlsr.getFib().update(name, (*it).getNexthopList());
    }
    else {
      (*it).resetRteListNextHop();
      (*it).getNexthopList().reset();
      m_nlsr.getFib().remove(name);
    }
  }
}

void
NamePrefixTable::removeEntry(const ndn::Name& name, RoutingTableEntry& rte)
{
  std::list<NamePrefixTableEntry>::iterator it = std::find_if(m_table.begin(),
                                                              m_table.end(),
                                                              ndn::bind(&npteCompare, _1, name));
  if (it != m_table.end()) {
    ndn::Name destRouter = rte.getDestination();
    (*it).removeRoutingTableEntry(rte);
    if (((*it).getRteListSize() == 0) &&
        (!m_nlsr.getLsdb().doesLsaExist(destRouter.append("/" + NameLsa::TYPE_STRING),
                                        (NameLsa::TYPE_STRING))) &&
        (!m_nlsr.getLsdb().doesLsaExist(destRouter.append("/" + AdjLsa::TYPE_STRING),
                                        (AdjLsa::TYPE_STRING))) &&
        (!m_nlsr.getLsdb().doesLsaExist(destRouter.append("/" + CoordinateLsa::TYPE_STRING),
                                        (CoordinateLsa::TYPE_STRING)))) {
      m_table.erase(it);
      m_nlsr.getFib().remove(name);
    }
    else {
      (*it).generateNhlfromRteList();
      m_nlsr.getFib().update(name, (*it).getNexthopList());
    }
  }
}


void
NamePrefixTable::addEntry(const ndn::Name& name, const ndn::Name& destRouter)
{
  //
  RoutingTableEntry* rteCheck =
    m_nlsr.getRoutingTable().findRoutingTableEntry(destRouter);
  if (rteCheck != 0) {
    addEntry(name, *(rteCheck));
  }
  else {
    RoutingTableEntry rte(destRouter);
    addEntry(name, rte);
  }
}

void
NamePrefixTable::removeEntry(const ndn::Name& name, const ndn::Name& destRouter)
{
  //
  RoutingTableEntry* rteCheck =
    m_nlsr.getRoutingTable().findRoutingTableEntry(destRouter);
  if (rteCheck != 0) {
    removeEntry(name, *(rteCheck));
  }
  else {
    RoutingTableEntry rte(destRouter);
    removeEntry(name, rte);
  }
}

void
NamePrefixTable::updateWithNewRoute()
{
  for (std::list<NamePrefixTableEntry>::iterator it = m_table.begin();
       it != m_table.end(); ++it) {
    std::list<RoutingTableEntry> rteList = (*it).getRteList();
    for (std::list<RoutingTableEntry>::iterator rteit = rteList.begin();
         rteit != rteList.end(); ++rteit) {
      RoutingTableEntry* rteCheck =
        m_nlsr.getRoutingTable().findRoutingTableEntry((*rteit).getDestination());
      if (rteCheck != 0) {
        addEntry((*it).getNamePrefix(), *(rteCheck));
      }
      else {
        RoutingTableEntry rte((*rteit).getDestination());
        addEntry((*it).getNamePrefix(), rte);
      }
    }
  }
}

void
NamePrefixTable::writeLog()
{
  _LOG_DEBUG("----------------NPT----------------------");
  for (std::list<NamePrefixTableEntry>::iterator it = m_table.begin();
       it != m_table.end();
       ++it) {
    (*it).writeLog();
  }
}

} //namespace nlsr
