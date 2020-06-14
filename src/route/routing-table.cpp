/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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

#include "routing-table.hpp"
#include "nlsr.hpp"
#include "map.hpp"
#include "conf-parameter.hpp"
#include "routing-table-calculator.hpp"
#include "routing-table-entry.hpp"
#include "logger.hpp"
#include "tlv-nlsr.hpp"

namespace nlsr {

INIT_LOGGER(route.RoutingTable);

RoutingTable::RoutingTable(ndn::Scheduler& scheduler, Lsdb& lsdb, ConfParameter& confParam)
  : m_scheduler(scheduler)
  , m_lsdb(lsdb)
  , m_routingCalcInterval{confParam.getRoutingCalcInterval()}
  , m_isRoutingTableCalculating(false)
  , m_isRouteCalculationScheduled(false)
  , m_confParam(confParam)
  , m_hyperbolicState(m_confParam.getHyperbolicState())
{
  m_afterLsdbModified = lsdb.onLsdbModified.connect(
    [this] (std::shared_ptr<Lsa> lsa, LsdbUpdate updateType,
            const auto& namesToAdd, const auto& namesToRemove) {
      auto type = lsa->getType();
      bool updateForOwnAdjacencyLsa = lsa->getOriginRouter() == m_confParam.getRouterPrefix() &&
                                      type == Lsa::Type::ADJACENCY;
      bool scheduleCalculation = false;

      if (updateType == LsdbUpdate::REMOVED && updateForOwnAdjacencyLsa) {
        // If own Adjacency LSA is removed then we have no ACTIVE neighbors.
        // (Own Coordinate LSA is never removed. But routing table calculation is scheduled
        // in HelloProtocol. The routing table calculator for HR takes into account
        // the INACTIVE status of the link).
        NLSR_LOG_DEBUG("No Adj LSA of router itself, routing table can not be calculated :(");
        clearRoutingTable();
        clearDryRoutingTable();
        NLSR_LOG_DEBUG("Calling Update NPT With new Route");
        afterRoutingChange(m_rTable);
        NLSR_LOG_DEBUG(*this);
        m_ownAdjLsaExist = false;
      }

      if (updateType == LsdbUpdate::INSTALLED && updateForOwnAdjacencyLsa) {
        m_ownAdjLsaExist = true;
      }

      // Don;t do anything on removal, wait for HelloProtocol to confirm and then react
      if (updateType == LsdbUpdate::INSTALLED || updateType == LsdbUpdate::UPDATED) {
        if ((type == Lsa::Type::ADJACENCY  && m_hyperbolicState != HYPERBOLIC_STATE_ON) ||
            (type == Lsa::Type::COORDINATE && m_hyperbolicState != HYPERBOLIC_STATE_OFF)) {
          scheduleCalculation = true;
        }
      }

      if (scheduleCalculation) {
        scheduleRoutingTableCalculation();
      }
    }
  );
}

void
RoutingTable::calculate()
{
  m_lsdb.writeLog();
  NLSR_LOG_TRACE("Calculating routing table");

  if (m_isRoutingTableCalculating == false) {
    m_isRoutingTableCalculating = true;

    if (m_hyperbolicState == HYPERBOLIC_STATE_OFF) {
      calculateLsRoutingTable();
    }
    else if (m_hyperbolicState == HYPERBOLIC_STATE_DRY_RUN) {
      calculateLsRoutingTable();
      calculateHypRoutingTable(true);
    }
    else if (m_hyperbolicState == HYPERBOLIC_STATE_ON) {
      calculateHypRoutingTable(false);
    }

    m_isRouteCalculationScheduled = false;
    m_isRoutingTableCalculating = false;
  }
  else {
    scheduleRoutingTableCalculation();
  }
}

void
RoutingTable::calculateLsRoutingTable()
{
  NLSR_LOG_TRACE("CalculateLsRoutingTable Called");

  if (m_lsdb.getIsBuildAdjLsaScheduled()) {
    NLSR_LOG_DEBUG("Adjacency build is scheduled, routing table can not be calculated :(");
    return;
  }

  // We only check this in LS since we never remove our own Coordinate LSA,
  // whereas we remove our own Adjacency LSA if we don't have any neighbors
  if (!m_ownAdjLsaExist) {
    return;
  }

  clearRoutingTable();

  Map map;
  auto lsaRange = m_lsdb.getLsdbIterator<AdjLsa>();
  map.createFromAdjLsdb(lsaRange.first, lsaRange.second);
  map.writeLog();

  size_t nRouters = map.getMapSize();

  LinkStateRoutingTableCalculator calculator(nRouters);

  calculator.calculatePath(map, *this, m_confParam, m_lsdb);

  NLSR_LOG_DEBUG("Calling Update NPT With new Route");
  afterRoutingChange(m_rTable);
  NLSR_LOG_DEBUG(*this);
}

void
RoutingTable::calculateHypRoutingTable(bool isDryRun)
{
  if (isDryRun) {
    clearDryRoutingTable();
  }
  else {
    clearRoutingTable();
  }

  Map map;
  auto lsaRange = m_lsdb.getLsdbIterator<CoordinateLsa>();
  map.createFromCoordinateLsdb(lsaRange.first, lsaRange.second);
  map.writeLog();

  size_t nRouters = map.getMapSize();

  HyperbolicRoutingCalculator calculator(nRouters, isDryRun, m_confParam.getRouterPrefix());

  calculator.calculatePath(map, *this, m_lsdb, m_confParam.getAdjacencyList());

  if (!isDryRun) {
    NLSR_LOG_DEBUG("Calling Update NPT With new Route");
    afterRoutingChange(m_rTable);
    NLSR_LOG_DEBUG(*this);
  }
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
  m_wire.reset();
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
  m_wire.reset();
}

void
RoutingTable::clearRoutingTable()
{
  m_rTable.clear();
  m_wire.reset();
}

void
RoutingTable::clearDryRoutingTable()
{
  m_dryTable.clear();
  m_wire.reset();
}

template<ndn::encoding::Tag TAG>
size_t
RoutingTableStatus::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  for (auto it = m_dryTable.rbegin(); it != m_dryTable.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  for (auto it = m_rTable.rbegin(); it != m_rTable.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::RoutingTable);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(RoutingTableStatus);

const ndn::Block&
RoutingTableStatus::wireEncode() const
{
  if (m_wire.hasWire()) {
    return m_wire;
  }

  ndn::EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  ndn::EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();

  return m_wire;
}

void
RoutingTableStatus::wireDecode(const ndn::Block& wire)
{
  m_rTable.clear();

  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::RoutingTable) {
    NDN_THROW(Error("RoutingTable", m_wire.type()));
  }

  m_wire.parse();
  auto val = m_wire.elements_begin();

  std::set<ndn::Name> destinations;
  for (; val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::RoutingTableEntry; ++val) {
    auto entry = RoutingTableEntry(*val);

    if (destinations.emplace(entry.getDestination()).second) {
      m_rTable.push_back(entry);
    }
    else {
      // If destination already exists then this is the start of dry HR table
      m_dryTable.push_back(entry);
    }
  }

  if (val != m_wire.elements_end()) {
    NDN_THROW(Error("Unrecognized TLV of type " + ndn::to_string(val->type()) + " in RoutingTable"));
  }
}

std::ostream&
operator<<(std::ostream& os, const RoutingTableStatus& rts)
{
  os << "Routing Table:\n";
  for (const auto& rte : rts.getRoutingTableEntry()) {
    os << rte;
  }

  if (!rts.getDryRoutingTableEntry().empty()) {
    os << "Dry-Run Hyperbolic Routing Table:\n";
    for (const auto& rte : rts.getDryRoutingTableEntry()) {
      os << rte;
    }
  }
  return os;
}

} // namespace nlsr
