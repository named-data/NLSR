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

INIT_LOGGER("RoutingTable");

RoutingTable::RoutingTable(ndn::Scheduler& scheduler)
  : afterRoutingChange{ndn::make_unique<AfterRoutingChange>()}
  , m_scheduler(scheduler)
  , m_NO_NEXT_HOP{-12345}
  , m_routingCalcInterval{static_cast<uint32_t>(ROUTING_CALC_INTERVAL_DEFAULT)}
{
}

void
RoutingTable::calculate(Nlsr& pnlsr)
{
  pnlsr.getLsdb().writeCorLsdbLog();
  pnlsr.getLsdb().writeNameLsdbLog();
  pnlsr.getLsdb().writeAdjLsdbLog();
  pnlsr.getNamePrefixTable().writeLog();
  if (pnlsr.getIsRoutingTableCalculating() == false) {
    //setting routing table calculation
    pnlsr.setIsRoutingTableCalculating(true);

    bool isHrEnabled = pnlsr.getConfParameter().getHyperbolicState() != HYPERBOLIC_STATE_OFF;

    if ((!isHrEnabled
         &&
         pnlsr.getLsdb()
         .doesLsaExist(ndn::Name{pnlsr.getConfParameter().getRouterPrefix()}
                       .append(std::to_string(Lsa::Type::ADJACENCY)), Lsa::Type::ADJACENCY))
        ||
        (isHrEnabled
         &&
         pnlsr.getLsdb()
         .doesLsaExist(ndn::Name{pnlsr.getConfParameter().getRouterPrefix()}
                       .append(std::to_string(Lsa::Type::COORDINATE)), Lsa::Type::COORDINATE))) {
      if (pnlsr.getIsBuildAdjLsaSheduled() != 1) {
        NLSR_LOG_TRACE("Clearing old routing table");
        clearRoutingTable();
        // for dry run options
        clearDryRoutingTable();

        NLSR_LOG_DEBUG("Calculating routing table");

        // calculate Link State routing
        if ((pnlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_OFF)
            || (pnlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_DRY_RUN)) {
          calculateLsRoutingTable(pnlsr);
        }
        //calculate hyperbolic routing
        if (pnlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_ON) {
          calculateHypRoutingTable(pnlsr);
        }
        //calculate dry hyperbolic routing
        if (pnlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_DRY_RUN) {
          calculateHypDryRoutingTable(pnlsr);
        }
        // Inform the NPT that updates have been made
        NLSR_LOG_DEBUG("Calling Update NPT With new Route");
        (*afterRoutingChange)(m_rTable);
        writeLog(pnlsr.getConfParameter().getHyperbolicState());
        pnlsr.getNamePrefixTable().writeLog();
        pnlsr.getFib().writeLog();
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
      writeLog(pnlsr.getConfParameter().getHyperbolicState());
      pnlsr.getNamePrefixTable().writeLog();
      pnlsr.getFib().writeLog();
      //debugging purpose end
    }
    pnlsr.setIsRouteCalculationScheduled(false); //clear scheduled flag
    pnlsr.setIsRoutingTableCalculating(false); //unsetting routing table calculation
  }
  else {
    scheduleRoutingTableCalculation(pnlsr);
  }
}

void
RoutingTable::calculateLsRoutingTable(Nlsr& nlsr)
{
  NLSR_LOG_DEBUG("RoutingTable::calculateLsRoutingTable Called");

  Map map;
  map.createFromAdjLsdb(nlsr.getLsdb().getAdjLsdb().begin(), nlsr.getLsdb().getAdjLsdb().end());
  map.writeLog();

  size_t nRouters = map.getMapSize();

  LinkStateRoutingTableCalculator calculator(nRouters);

  calculator.calculatePath(map, std::ref(*this), nlsr);
}

void
RoutingTable::calculateHypRoutingTable(Nlsr& nlsr)
{
  Map map;
  map.createFromCoordinateLsdb(nlsr.getLsdb().getCoordinateLsdb().begin(),
                               nlsr.getLsdb().getCoordinateLsdb().end());
  map.writeLog();

  size_t nRouters = map.getMapSize();

  HyperbolicRoutingCalculator calculator(nRouters, false,
                                         nlsr.getConfParameter().getRouterPrefix());

  calculator.calculatePaths(map, std::ref(*this),
                            nlsr.getLsdb(), nlsr.getAdjacencyList());
}

void
RoutingTable::calculateHypDryRoutingTable(Nlsr& nlsr)
{
  Map map;
  map.createFromAdjLsdb(nlsr.getLsdb().getAdjLsdb().begin(), nlsr.getLsdb().getAdjLsdb().end());
  map.writeLog();

  size_t nRouters = map.getMapSize();

  HyperbolicRoutingCalculator calculator(nRouters, true,
                                         nlsr.getConfParameter().getRouterPrefix());

  calculator.calculatePaths(map, std::ref(*this),
                            nlsr.getLsdb(), nlsr.getAdjacencyList());
}

void
RoutingTable::scheduleRoutingTableCalculation(Nlsr& pnlsr)
{
  if (pnlsr.getIsRouteCalculationScheduled() != true) {
    NLSR_LOG_DEBUG("Scheduling routing table calculation in " << m_routingCalcInterval);

    m_scheduler.scheduleEvent(m_routingCalcInterval,
                              std::bind(&RoutingTable::calculate, this, std::ref(pnlsr)));

    pnlsr.setIsRouteCalculationScheduled(true);
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
  if (rteChk == 0) {
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
  std::list<RoutingTableEntry>::iterator it = std::find_if(m_rTable.begin(),
                                                           m_rTable.end(),
                                                           std::bind(&routingTableEntryCompare,
                                                                     _1, destRouter));
  if (it != m_rTable.end()) {
    return &(*it);
  }
  return 0;
}

void
RoutingTable::writeLog(int hyperbolicState)
{
  NLSR_LOG_DEBUG("---------------Routing Table------------------");
  for (std::list<RoutingTableEntry>::iterator it = m_rTable.begin() ;
       it != m_rTable.end(); ++it) {
    NLSR_LOG_DEBUG("Destination: " << (*it).getDestination());
    NLSR_LOG_DEBUG("Nexthops: ");
    (*it).getNexthopList().writeLog();
  }

  if (hyperbolicState == HYPERBOLIC_STATE_DRY_RUN) {
    NLSR_LOG_DEBUG("--------Hyperbolic Routing Table(Dry)---------");
    for (std::list<RoutingTableEntry>::iterator it = m_dryTable.begin() ;
        it != m_dryTable.end(); ++it) {
      NLSR_LOG_DEBUG("Destination: " << (*it).getDestination());
      NLSR_LOG_DEBUG("Nexthops: ");
      (*it).getNexthopList().writeLog();
    }
  }
}

void
RoutingTable::addNextHopToDryTable(const ndn::Name& destRouter, NextHop& nh)
{
  NLSR_LOG_DEBUG("Adding " << nh << " to dry table for destination: " << destRouter);

  std::list<RoutingTableEntry>::iterator it = std::find_if(m_dryTable.begin(),
                                                           m_dryTable.end(),
                                                           std::bind(&routingTableEntryCompare,
                                                                     _1, destRouter));
  if (it == m_dryTable.end()) {
    RoutingTableEntry rte(destRouter);
    rte.getNexthopList().addNextHop(nh);
    m_dryTable.push_back(rte);
  }
  else {
    (*it).getNexthopList().addNextHop(nh);
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
