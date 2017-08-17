/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
 *                           Regents of the University of California
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
 **/

#ifndef NLSR_ROUTING_TABLE_HPP
#define NLSR_ROUTING_TABLE_HPP

#include "conf-parameter.hpp"
#include "routing-table-entry.hpp"
#include "signals.hpp"

#include <iostream>
#include <utility>
#include <string>
#include <boost/cstdint.hpp>
#include <ndn-cxx/util/scheduler.hpp>

namespace nlsr {

class Nlsr;
class NextHop;

class RoutingTable
{
public:
  RoutingTable(ndn::Scheduler& scheduler);

  /*! \brief Calculates a list of next hops for each router in the network.
   * \param pnlsr The NLSR object that contains the LSAs needed for adj. info.
   *
   * Calculates the list of next hops to every other router in the network.
   */
  void
  calculate(Nlsr& pnlsr);

  /*! \brief Adds a next hop to a routing table entry.
   * \param destRouter The destination router whose RTE we want to modify.
   * \param nh The next hop to add to the RTE.
   */
  void
  addNextHop(const ndn::Name& destRouter, NextHop& nh);

  /*! \brief Adds a next hop to a routing table entry in a dry run scenario.
   * \param destRouter The destination router whose RTE we want to modify.
   * \param nh The next hop to add to the router.
   */
  void
  addNextHopToDryTable(const ndn::Name& destRouter, NextHop& nh);

  RoutingTableEntry*
  findRoutingTableEntry(const ndn::Name& destRouter);

  /*! \brief Schedules a calculation event in the event scheduler only
   * if one isn't already scheduled.
   * \param pnlsr The NLSR whose scheduling status is needed.
   */
  void
  scheduleRoutingTableCalculation(Nlsr& pnlsr);

  int
  getNoNextHop()
  {
    return m_NO_NEXT_HOP;
  }

  void
  setRoutingCalcInterval(uint32_t interval)
  {
    m_routingCalcInterval = ndn::time::seconds(interval);
  }

  const ndn::time::seconds&
  getRoutingCalcInterval() const
  {
    return m_routingCalcInterval;
  }

private:
  /*! \brief Calculates a link-state routing table. */
  void
  calculateLsRoutingTable(Nlsr& pnlsr);

  /*! \brief Calculates a HR routing table. */
  void
  calculateHypRoutingTable(Nlsr& pnlsr);

  /*! \brief Calculates a dry-run HR routing table. */
  void
  calculateHypDryRoutingTable(Nlsr& pnlsr);

  void
  clearRoutingTable();

  void
  clearDryRoutingTable();

  void
  writeLog(int hyperbolicState);

public:
  std::shared_ptr<AfterRoutingChange> afterRoutingChange;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::list<RoutingTableEntry> m_rTable;

private:
  ndn::Scheduler& m_scheduler;

  const int m_NO_NEXT_HOP;

  std::list<RoutingTableEntry> m_dryTable;

  ndn::time::seconds m_routingCalcInterval;
};

} // namespace nlsr

#endif // NLSR_ROUTING_TABLE_HPP
