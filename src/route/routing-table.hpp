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
#ifndef NLSR_ROUTING_TABLE_HPP
#define NLSR_ROUTING_TABLE_HPP

#include <iostream>
#include <utility>
#include <string>
#include <boost/cstdint.hpp>

#include "routing-table-entry.hpp"

namespace nlsr {

class Nlsr;
class NextHop;

class RoutingTable
{
public:
  RoutingTable()
    : m_NO_NEXT_HOP(-12345)
  {
  }
  void
  calculate(Nlsr& pnlsr);

  void
  addNextHop(const ndn::Name& destRouter, NextHop& nh);

  void
  printRoutingTable();

  void
  addNextHopToDryTable(const ndn::Name& destRouter, NextHop& nh);

  void
  printDryRoutingTable();

  RoutingTableEntry*
  findRoutingTableEntry(const ndn::Name& destRouter);

  void
  scheduleRoutingTableCalculation(Nlsr& pnlsr);

  int
  getNoNextHop()
  {
    return m_NO_NEXT_HOP;
  }

private:
  void
  calculateLsRoutingTable(Nlsr& pnlsr);

  void
  calculateHypRoutingTable(Nlsr& pnlsr);

  void
  calculateHypDryRoutingTable(Nlsr& pnlsr);

  void
  clearRoutingTable();

  void
  clearDryRoutingTable();

  void
  writeLog();

  const int m_NO_NEXT_HOP;

  std::list<RoutingTableEntry> m_rTable;
  std::list<RoutingTableEntry> m_dryTable;
};

}//namespace nlsr

#endif //NLSR_ROUTING_TABLE_HPP
