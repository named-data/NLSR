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
#ifndef NLSR_MAP_HPP
#define NLSR_MAP_HPP

#include <iostream>
#include <list>
#include <boost/cstdint.hpp>

#include <ndn-cxx/common.hpp>

#include "map-entry.hpp"

namespace nlsr {

class Nlsr;

class Map
{
public:
  Map()
    : m_mappingIndex(0)
  {
  }


  void
  addEntry(const ndn::Name& rtrName);

  void
  createFromAdjLsdb(Nlsr& pnlsr);

  void
  createFromCoordinateLsdb(Nlsr& nlsr);

  const ndn::Name
  getRouterNameByMappingNo(int32_t mn);

  int32_t
  getMappingNoByRouterName(const ndn::Name& rName);

  void
  reset();

  std::list<MapEntry>&
  getMapList()
  {
    return m_table;
  }

  size_t
  getMapSize() const
  {
    return m_table.size();
  }

  void
  writeLog();

private:
  bool
  addEntry(MapEntry& mpe);

  int32_t m_mappingIndex;
  std::list<MapEntry> m_table;
};

} // namespace nlsr
#endif //NLSR_MAP_HPP
