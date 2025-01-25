/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
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

#include "fib.hpp"
#include "adjacency-list.hpp"
#include "conf-parameter.hpp"
#include "logger.hpp"
#include "nexthop-list.hpp"

#include <ndn-cxx/mgmt/nfd/control-command.hpp>

#include <algorithm>
#include <cmath>
#include <map>

namespace nlsr {

INIT_LOGGER(route.Fib);

Fib::Fib(ndn::Face& face, ndn::Scheduler& scheduler, AdjacencyList& adjacencyList,
         ConfParameter& conf, ndn::security::KeyChain& keyChain)
  : m_scheduler(scheduler)
  , m_refreshTime(2 * conf.getLsaRefreshTime())
  , m_controller(face, keyChain)
  , m_adjacencyList(adjacencyList)
  , m_confParameter(conf)
{
}

void
Fib::remove(const ndn::Name& name)
{
  NLSR_LOG_DEBUG("Fib::remove called");
  auto it = m_table.find(name);

  // Only unregister the prefix if it ISN'T a neighbor.
  if (it != m_table.end() && isNotNeighbor((it->second).name)) {
    for (const auto& nexthop : (it->second).nexthopSet) {
      unregisterPrefix((it->second).name, nexthop.getConnectingFaceUri());
    }
    m_table.erase(it);
  }
}

void
Fib::addNextHopsToFibEntryAndNfd(FibEntry& entry, const NextHopsUriSortedSet& hopsToAdd)
{
  const ndn::Name& name = entry.name;

  bool shouldRegister = isNotNeighbor(name);

  for (const auto& hop : hopsToAdd)
  {
    // Add nexthop to FIB entry
    NLSR_LOG_DEBUG("Adding " << hop.getConnectingFaceUri() << " to " << entry.name);
    entry.nexthopSet.addNextHop(hop);

    if (shouldRegister) {
      // Add nexthop to NDN-FIB
      registerPrefix(name, ndn::FaceUri(hop.getConnectingFaceUri()),
                     hop.getRouteCostAsAdjustedInteger(),
                     ndn::time::seconds(m_refreshTime + GRACE_PERIOD),
                     ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
    }
  }
}

void
Fib::update(const ndn::Name& name, const NexthopList& allHops)
{
  NLSR_LOG_DEBUG("Fib::update called");

  // Get the max possible faces which is the minimum of the configuration setting and
  // the length of the list of all next hops.
  unsigned int maxFaces = getNumberOfFacesForName(allHops);

  NextHopsUriSortedSet hopsToAdd;
  unsigned int nFaces = 0;

  // Create a list of next hops to be installed with length == maxFaces
  for (auto it = allHops.cbegin(); it != allHops.cend() && nFaces < maxFaces; ++it, ++nFaces) {
    hopsToAdd.addNextHop(*it);
  }

  auto entryIt = m_table.find(name);

  // New FIB entry that has nextHops
  if (entryIt == m_table.end() && hopsToAdd.size() != 0) {
    NLSR_LOG_DEBUG("New FIB Entry");

    FibEntry entry;
    entry.name = name;
    addNextHopsToFibEntryAndNfd(entry, hopsToAdd);

    entryIt = m_table.try_emplace(name, std::move(entry)).first;
  }
  // Existing FIB entry that may or may not have nextHops
  else {
    // Existing FIB entry
    NLSR_LOG_DEBUG("Existing FIB Entry");

    // Remove empty FIB entry
    if (hopsToAdd.size() == 0) {
      remove(name);
      return;
    }

    FibEntry& entry = entryIt->second;
    addNextHopsToFibEntryAndNfd(entry, hopsToAdd);

    std::set<NextHop, NextHopUriSortedComparator> hopsToRemove;
    std::set_difference(entry.nexthopSet.begin(), entry.nexthopSet.end(),
                        hopsToAdd.begin(), hopsToAdd.end(),
                        std::inserter(hopsToRemove, hopsToRemove.begin()),
                        NextHopUriSortedComparator());

    bool isUpdatable = isNotNeighbor(entry.name);
    // Remove the uninstalled next hops from NFD and FIB entry
    for (const auto& hop : hopsToRemove){
      if (isUpdatable) {
        unregisterPrefix(entry.name, hop.getConnectingFaceUri());
      }
      NLSR_LOG_DEBUG("Removing " << hop.getConnectingFaceUri() << " from " << entry.name);
      entry.nexthopSet.removeNextHop(hop);
    }

    // Increment sequence number
    entry.seqNo += 1;
    entryIt = m_table.find(name);
  }

  if (entryIt != m_table.end() &&
      !entryIt->second.refreshEventId &&
      isNotNeighbor(entryIt->second.name)) {
    scheduleEntryRefresh(entryIt->second, [this] (FibEntry& entry) { scheduleLoop(entry); });
  }
}

unsigned int
Fib::getNumberOfFacesForName(const NexthopList& nextHopList)
{
  uint32_t nNextHops = static_cast<uint32_t>(nextHopList.getNextHops().size());
  uint32_t nMaxFaces = m_confParameter.getMaxFacesPerPrefix();

  // 0 == all faces
  return nMaxFaces == 0 ? nNextHops : std::min(nNextHops, nMaxFaces);
}

bool
Fib::isNotNeighbor(const ndn::Name& name)
{
  return !m_adjacencyList.isNeighbor(name);
}

void
Fib::registerPrefix(const ndn::Name& namePrefix, const ndn::FaceUri& faceUri,
                    uint64_t faceCost, const ndn::time::milliseconds& timeout,
                    uint64_t flags, uint8_t times)
{
  uint64_t faceId = m_adjacencyList.getFaceId(faceUri);

  if (faceId > 0) {
    ndn::nfd::ControlParameters faceParameters;
    faceParameters
     .setName(namePrefix)
     .setFaceId(faceId)
     .setFlags(flags)
     .setCost(faceCost)
     .setExpirationPeriod(timeout)
     .setOrigin(ndn::nfd::ROUTE_ORIGIN_NLSR);

    NLSR_LOG_DEBUG("Registering prefix: " << faceParameters.getName() << " faceUri: " << faceUri);
    m_controller.start<ndn::nfd::RibRegisterCommand>(faceParameters,
      std::bind(&Fib::onRegistrationSuccess, this, _1, faceUri),
      std::bind(&Fib::onRegistrationFailure, this, _1, faceParameters, faceUri, times));
  }
  else {
    NLSR_LOG_WARN("Error: No Face Id for face uri: " << faceUri);
  }
}

void
Fib::onRegistrationSuccess(const ndn::nfd::ControlParameters& param,
                           const ndn::FaceUri& faceUri)
{
  NLSR_LOG_DEBUG("Successful in name registration: " << param.getName() <<
                 " Face Uri: " << faceUri << " faceId: " << param.getFaceId());

  auto adjacent = m_adjacencyList.findAdjacent(faceUri);
  if (adjacent != m_adjacencyList.end()) {
    adjacent->setFaceId(param.getFaceId());
  }
  onPrefixRegistrationSuccess(param.getName());
}

void
Fib::onRegistrationFailure(const ndn::nfd::ControlResponse& response,
                           const ndn::nfd::ControlParameters& parameters,
                           const ndn::FaceUri& faceUri,
                           uint8_t times)
{
  NLSR_LOG_DEBUG("Failed in name registration: " << response.getText() <<
                 " (code: " << response.getCode() << ")");
  NLSR_LOG_DEBUG("Prefix: " << parameters.getName() << " failed for: " << +times);
  if (times < 3) {
    NLSR_LOG_DEBUG("Trying to register again...");
    registerPrefix(parameters.getName(), faceUri,
                   parameters.getCost(),
                   parameters.getExpirationPeriod(),
                   parameters.getFlags(), times+1);
  }
  else {
    NLSR_LOG_DEBUG("Registration trial given up");
  }
}

void
Fib::unregisterPrefix(const ndn::Name& namePrefix, const ndn::FaceUri& faceUri)
{
  uint64_t faceId = 0;
  auto adjacent = m_adjacencyList.findAdjacent(faceUri);
  if (adjacent != m_adjacencyList.end()) {
    faceId = adjacent->getFaceId();
  }

  NLSR_LOG_DEBUG("Unregister prefix: " << namePrefix << " Face Uri: " << faceUri);
  if (faceId > 0) {
    ndn::nfd::ControlParameters controlParameters;
    controlParameters
      .setName(namePrefix)
      .setFaceId(faceId)
      .setOrigin(ndn::nfd::ROUTE_ORIGIN_NLSR);

    m_controller.start<ndn::nfd::RibUnregisterCommand>(controlParameters,
      [] (const ndn::nfd::ControlParameters& commandSuccessResult) {
        NLSR_LOG_DEBUG("Unregister successful Prefix: " << commandSuccessResult.getName() <<
                       " Face Id: " << commandSuccessResult.getFaceId());
      },
      [] (const ndn::nfd::ControlResponse& response) {
        NLSR_LOG_DEBUG("Failed in unregistering name: " << response.getText() <<
                       " (code " << response.getCode() << ")");
      });
  }
}

void
Fib::setStrategy(const ndn::Name& name, const ndn::Name& strategy, uint32_t count)
{
  ndn::nfd::ControlParameters parameters;
  parameters
    .setName(name)
    .setStrategy(strategy);

  m_controller.start<ndn::nfd::StrategyChoiceSetCommand>(parameters,
    std::bind(&Fib::onSetStrategySuccess, this, _1),
    std::bind(&Fib::onSetStrategyFailure, this, _1, parameters, count));
}

void
Fib::onSetStrategySuccess(const ndn::nfd::ControlParameters& commandSuccessResult)
{
  NLSR_LOG_DEBUG("Successfully set strategy choice: " << commandSuccessResult.getStrategy() <<
                 " for name: " << commandSuccessResult.getName());
}

void
Fib::onSetStrategyFailure(const ndn::nfd::ControlResponse&,
                          const ndn::nfd::ControlParameters& parameters,
                          uint32_t count)
{
  NLSR_LOG_DEBUG("Failed to set strategy choice: " << parameters.getStrategy() <<
                 " for name: " << parameters.getName());
  if (count < 3) {
    setStrategy(parameters.getName(), parameters.getStrategy().toUri(), count + 1);
  }
}

void
Fib::scheduleEntryRefresh(FibEntry& entry, const AfterRefreshCallback& refreshCallback)
{
  NLSR_LOG_DEBUG("Scheduling refresh for " << entry.name <<
                 " Seq Num: " << entry.seqNo <<
                 " in " << m_refreshTime << " seconds");

  entry.refreshEventId = m_scheduler.schedule(ndn::time::seconds(m_refreshTime),
                                              std::bind(&Fib::refreshEntry, this,
                                                        entry.name, refreshCallback));
}

void
Fib::scheduleLoop(FibEntry& entry)
{
  scheduleEntryRefresh(entry, std::bind(&Fib::scheduleLoop, this, _1));
}

void
Fib::refreshEntry(const ndn::Name& name, AfterRefreshCallback refreshCb)
{
  auto it = m_table.find(name);
  if (it == m_table.end()) {
    return;
  }

  FibEntry& entry = it->second;
  NLSR_LOG_DEBUG("Refreshing " << entry.name << " Seq Num: " << entry.seqNo);

  entry.seqNo += 1;

  for (const NextHop& hop : entry.nexthopSet) {
    registerPrefix(entry.name,
                   ndn::FaceUri(hop.getConnectingFaceUri()),
                   hop.getRouteCostAsAdjustedInteger(),
                   ndn::time::seconds(m_refreshTime + GRACE_PERIOD),
                   ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
  }

  refreshCb(entry);
}

void
Fib::writeLog()
{
  NLSR_LOG_DEBUG("-------------------FIB-----------------------------");
  for (const auto& entry : m_table) {
    NLSR_LOG_DEBUG("Name prefix: "  << entry.first);
    NLSR_LOG_DEBUG("Seq No: " <<  entry.second.seqNo);
    NLSR_LOG_DEBUG("Nexthop List: \n" << entry.second.nexthopSet);
  }
}

} // namespace nlsr
