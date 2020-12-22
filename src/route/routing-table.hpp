/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  The University of Memphis,
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
 */

#ifndef NLSR_ROUTING_TABLE_HPP
#define NLSR_ROUTING_TABLE_HPP

#include "conf-parameter.hpp"
#include "routing-table-entry.hpp"
#include "signals.hpp"
#include "lsdb.hpp"
#include "route/fib.hpp"
#include "test-access-control.hpp"

#include <ndn-cxx/util/scheduler.hpp>

namespace nlsr {

class NextHop;

/*! \brief Data abstraction for routing table status
 *
 * RtStatus := RT-STATUS-TYPE TLV-LENGTH
 *              RouteTableEntry*
 *
 * \sa https://redmine.named-data.net/projects/nlsr/wiki/Routing_Table_Dataset
 */
class RoutingTableStatus
{
public:
  using Error = ndn::tlv::Error;

  RoutingTableStatus() = default;

  RoutingTableStatus(const ndn::Block& block)
  {
    wireDecode(block);
  }

  const std::list<RoutingTableEntry>&
  getRoutingTableEntry() const
  {
    return m_rTable;
  }

  const std::list<RoutingTableEntry>&
  getDryRoutingTableEntry() const
  {
    return m_dryTable;
  }

  const ndn::Block&
  wireEncode() const;

private:
  void
  wireDecode(const ndn::Block& wire);

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

PUBLIC_WITH_TESTS_ELSE_PROTECTED:
  std::list<RoutingTableEntry> m_dryTable;
  std::list<RoutingTableEntry> m_rTable;
  mutable ndn::Block m_wire;
};

std::ostream&
operator<<(std::ostream& os, const RoutingTableStatus& rts);

class RoutingTable : public RoutingTableStatus
{
public:
  explicit
  RoutingTable(ndn::Scheduler& scheduler, Fib& fib, Lsdb& lsdb,
               NamePrefixTable& namePrefixTable, ConfParameter& confParam);

  /*! \brief Calculates a list of next hops for each router in the network.
   *
   *  Calculates the list of next hops to every other router in the network.
   */
  void
  calculate();

  /*! \brief Adds a next hop to a routing table entry.
   *  \param destRouter The destination router whose RTE we want to modify.
   *  \param nh The next hop to add to the RTE.
   */
  void
  addNextHop(const ndn::Name& destRouter, NextHop& nh);

  /*! \brief Adds a next hop to a routing table entry in a dry run scenario.
   *  \param destRouter The destination router whose RTE we want to modify.
   *  \param nh The next hop to add to the router.
   */
  void
  addNextHopToDryTable(const ndn::Name& destRouter, NextHop& nh);

  RoutingTableEntry*
  findRoutingTableEntry(const ndn::Name& destRouter);

  /*! \brief Schedules a calculation event in the event scheduler only
   *  if one isn't already scheduled.
   */
  void
  scheduleRoutingTableCalculation();

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

  uint64_t
  getRtSize()
  {
    return m_rTable.size();
  }

private:
  /*! \brief Calculates a link-state routing table. */
  void
  calculateLsRoutingTable();

  /*! \brief Calculates a HR routing table. */
  void
  calculateHypRoutingTable(bool isDryRun);

  void
  clearRoutingTable();

  void
  clearDryRoutingTable();

public:
  std::unique_ptr<AfterRoutingChange> afterRoutingChange;

private:
  ndn::Scheduler& m_scheduler;
  Fib& m_fib;
  Lsdb& m_lsdb;
  NamePrefixTable& m_namePrefixTable;

  ndn::time::seconds m_routingCalcInterval;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  bool m_isRoutingTableCalculating;
  bool m_isRouteCalculationScheduled;

  ConfParameter& m_confParam;
};

} // namespace nlsr

#endif // NLSR_ROUTING_TABLE_HPP
