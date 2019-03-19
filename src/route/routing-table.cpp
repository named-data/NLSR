/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  The University of Memphis,
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
 **/

#include "routing-table.hpp"
#include "nlsr.hpp"
#include "map.hpp"
#include "conf-parameter.hpp"
#include "routing-table-calculator.hpp"
#include "routing-table-entry.hpp"
#include "name-prefix-table.hpp"
#include "logger.hpp"

#include <iostream>
#include <list>
#include <string>

namespace nlsr {

INIT_LOGGER(route.RoutingTable);

RoutingTable::RoutingTable(ndn::Scheduler& scheduler, Fib& fib, Lsdb& lsdb,
                           NamePrefixTable& namePrefixTable, ConfParameter& confParam)
  : afterRoutingChange{std::make_unique<AfterRoutingChange>()}
  , m_scheduler(scheduler)
  , m_fib(fib)
  , m_lsdb(lsdb)
  , m_namePrefixTable(namePrefixTable)
  , m_NO_NEXT_HOP{-12345}
  , m_routingCalcInterval{confParam.getRoutingCalcInterval()}
  , m_isRoutingTableCalculating(false)
  , m_isRouteCalculationScheduled(false)
  , m_confParam(confParam)
{
}

void
RoutingTable::calculate()
{
  m_lsdb.writeCorLsdbLog();
  m_lsdb.writeNameLsdbLog();
  m_lsdb.writeAdjLsdbLog();
  m_namePrefixTable.writeLog();
  if (m_isRoutingTableCalculating == false) {
    // setting routing table calculation
    m_isRoutingTableCalculating = true;

    bool isHrEnabled = m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF;

    if ((!isHrEnabled &&
         m_lsdb
         .doesLsaExist(ndn::Name{m_confParam.getRouterPrefix()}
                       .append(std::to_string(Lsa::Type::ADJACENCY)), Lsa::Type::ADJACENCY))
        ||
        (isHrEnabled &&
         m_lsdb
         .doesLsaExist(ndn::Name{m_confParam.getRouterPrefix()}
                       .append(std::to_string(Lsa::Type::COORDINATE)), Lsa::Type::COORDINATE))) {
      if (m_lsdb.getIsBuildAdjLsaSheduled() != 1) {
        NLSR_LOG_TRACE("Clearing old routing table");
        clearRoutingTable();
        // for dry run options
        clearDryRoutingTable();

        NLSR_LOG_DEBUG("Calculating routing table");

        // calculate Link State routing
        if ((m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_OFF)
            || (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_DRY_RUN)) {
          calculateLsRoutingTable();
        }
        // calculate hyperbolic routing
        if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
          calculateHypRoutingTable(false);
        }
        // calculate dry hyperbolic routing
        if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_DRY_RUN) {
          calculateHypRoutingTable(true);
        }
        // Inform the NPT that updates have been made
        NLSR_LOG_DEBUG("Calling Update NPT With new Route");
        (*afterRoutingChange)(m_rTable);
        writeLog();
        m_namePrefixTable.writeLog();
        m_fib.writeLog();
      }
      else {
        NLSR_LOG_DEBUG("Adjacency building is scheduled, so"
                   " routing table can not be calculated :(");
      }
    }
    else {
      NLSR_LOG_DEBUG("No Adj LSA of router itself,"
                 " so Routing table can not be calculated :(");
      clearRoutingTable();
      clearDryRoutingTable(); // for dry run options
      // need to update NPT here
      NLSR_LOG_DEBUG("Calling Update NPT With new Route");
      (*afterRoutingChange)(m_rTable);
      writeLog();
      m_namePrefixTable.writeLog();
      m_fib.writeLog();
      // debugging purpose end
    }
    m_isRouteCalculationScheduled = false; // clear scheduled flag
    m_isRoutingTableCalculating = false; // unsetting routing table calculation
  }
  else {
    scheduleRoutingTableCalculation();
  }
}

void
RoutingTable::calculateLsRoutingTable()
{
  NLSR_LOG_DEBUG("RoutingTable::calculateLsRoutingTable Called");

  Map map;
  map.createFromAdjLsdb(m_lsdb.getAdjLsdb().begin(), m_lsdb.getAdjLsdb().end());
  map.writeLog();

  size_t nRouters = map.getMapSize();

  LinkStateRoutingTableCalculator calculator(nRouters);

  calculator.calculatePath(map, *this, m_confParam, m_lsdb.getAdjLsdb());
}

void
RoutingTable::calculateHypRoutingTable(bool isDryRun)
{
  Map map;
  map.createFromCoordinateLsdb(m_lsdb.getCoordinateLsdb().begin(),
                               m_lsdb.getCoordinateLsdb().end());
  map.writeLog();

  size_t nRouters = map.getMapSize();

  HyperbolicRoutingCalculator calculator(nRouters, isDryRun, m_confParam.getRouterPrefix());

  calculator.calculatePath(map, *this, m_lsdb, m_confParam.getAdjacencyList());
}

void
RoutingTable::scheduleRoutingTableCalculation()
{
  if (!m_isRouteCalculationScheduled) {
    NLSR_LOG_DEBUG("Scheduling routing table calculation in " << m_routingCalcInterval);
    m_scheduler.schedule(m_routingCalcInterval, [this] { calculate(); });
    m_isRouteCalculationScheduled = true;
  }
}

static bool
routingTableEntryCompare(RoutingTableEntry& rte, ndn::Name& destRouter)
{
  return rte.getDestination() == destRouter;
}

void
RoutingTable::addNextHop(const ndn::Name& destRouter, NextHop& nh)
{
  NLSR_LOG_DEBUG("Adding " << nh << " for destination: " << destRouter);

  RoutingTableEntry* rteChk = findRoutingTableEntry(destRouter);
  if (rteChk == nullptr) {
    RoutingTableEntry rte(destRouter);
    rte.getNexthopList().addNextHop(nh);
    m_rTable.push_back(rte);
  }
  else {
    rteChk->getNexthopList().addNextHop(nh);
  }
}

RoutingTableEntry*
RoutingTable::findRoutingTableEntry(const ndn::Name& destRouter)
{
  auto it = std::find_if(m_rTable.begin(), m_rTable.end(),
                         std::bind(&routingTableEntryCompare, _1, destRouter));
  if (it != m_rTable.end()) {
    return &(*it);
  }
  return nullptr;
}

void
RoutingTable::writeLog()
{
  NLSR_LOG_DEBUG("---------------Routing Table------------------");
  for (const auto& rte : m_rTable) {
    NLSR_LOG_DEBUG("Destination: " << rte.getDestination());
    NLSR_LOG_DEBUG("Nexthops: ");
    rte.getNexthopList().writeLog();
  }

  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_DRY_RUN) {
    NLSR_LOG_DEBUG("--------Hyperbolic Routing Table(Dry)---------");
    for (const auto& rte : m_dryTable) {
      NLSR_LOG_DEBUG("Destination: " << rte.getDestination());
      NLSR_LOG_DEBUG("Nexthops: ");
      rte.getNexthopList().writeLog();
    }
  }
}

void
RoutingTable::addNextHopToDryTable(const ndn::Name& destRouter, NextHop& nh)
{
  NLSR_LOG_DEBUG("Adding " << nh << " to dry table for destination: " << destRouter);

  auto it = std::find_if(m_dryTable.begin(), m_dryTable.end(),
                         std::bind(&routingTableEntryCompare, _1, destRouter));
  if (it == m_dryTable.end()) {
    RoutingTableEntry rte(destRouter);
    rte.getNexthopList().addNextHop(nh);
    m_dryTable.push_back(rte);
  }
  else {
    it->getNexthopList().addNextHop(nh);
  }
}

void
RoutingTable::clearRoutingTable()
{
  if (m_rTable.size() > 0) {
    m_rTable.clear();
  }
}

void
RoutingTable::clearDryRoutingTable()
{
  if (m_dryTable.size() > 0) {
    m_dryTable.clear();
  }
}

} // namespace nlsr
