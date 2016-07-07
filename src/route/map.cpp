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
#include <iostream>
#include <list>

#include "nlsr.hpp"
#include "adjacent.hpp"
#include "lsa.hpp"
#include "lsdb.hpp"
#include "map.hpp"
#include "logger.hpp"
namespace nlsr {

INIT_LOGGER("Map");

using namespace std;

static bool
mapEntryCompareByRouter(MapEntry& mpe1, const ndn::Name& rtrName)
{
  return mpe1.getRouter() == rtrName;
}

static bool
mapEntryCompareByMappingNo(MapEntry& mpe1, int32_t mappingNo)
{
  return mpe1.getMappingNumber() == mappingNo;
}

void
Map::addEntry(const ndn::Name& rtrName)
{
  MapEntry me(rtrName, m_mappingIndex);
  if (addEntry(me)) {
    m_mappingIndex++;
  }
}

bool
Map::addEntry(MapEntry& mpe)
{
  //cout << mpe;
  std::list<MapEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  ndn::bind(&mapEntryCompareByRouter,
                                                            _1, mpe.getRouter()));
  if (it == m_table.end()) {
    m_table.push_back(mpe);
    return true;
  }
  return false;
}

const ndn::Name
Map::getRouterNameByMappingNo(int32_t mn)
{
  std::list<MapEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  ndn::bind(&mapEntryCompareByMappingNo,
                                                            _1, mn));
  if (it != m_table.end()) {
    return (*it).getRouter();
  }
  return ndn::Name();
}

int32_t
Map::getMappingNoByRouterName(const ndn::Name& rName)
{
  std::list<MapEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  ndn::bind(&mapEntryCompareByRouter,
                                                            _1, rName));
  if (it != m_table.end()) {
    return (*it).getMappingNumber();
  }
  return -1;
}

void
Map::createFromAdjLsdb(Nlsr& pnlsr)
{
  std::list<AdjLsa> adjLsdb = pnlsr.getLsdb().getAdjLsdb();
  for (std::list<AdjLsa>::iterator it = adjLsdb.begin();
       it != adjLsdb.end() ; it++) {
    addEntry((*it).getOrigRouter());
    std::list<Adjacent> adl = (*it).getAdl().getAdjList();
    for (std::list<Adjacent>::iterator itAdl = adl.begin();
         itAdl != adl.end() ; itAdl++) {
      addEntry((*itAdl).getName());
    }
  }
}

void
Map::createFromCoordinateLsdb(Nlsr& nlsr)
{
  for (CoordinateLsa lsa : nlsr.getLsdb().getCoordinateLsdb()) {
    addEntry(lsa.getOrigRouter());
  }
}

void
Map::reset()
{
  m_table.clear();
  m_mappingIndex = 0;
}

void
Map::writeLog()
{
  _LOG_DEBUG("---------------Map----------------------");
  for (std::list<MapEntry>::iterator it = m_table.begin(); it != m_table.end() ; it++) {
    _LOG_DEBUG("MapEntry: ( Router: " << (*it).getRouter() << " Mapping No: "
               << (*it).getMappingNumber() << " )");
  }
}

} //namespace nlsr
