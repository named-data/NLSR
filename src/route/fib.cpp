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

#include "fib.hpp"
#include "adjacency-list.hpp"
#include "conf-parameter.hpp"
#include "logger.hpp"
#include "nexthop-list.hpp"

#include <map>
#include <cmath>
#include <algorithm>
#include <iterator>

namespace nlsr {

INIT_LOGGER("Fib");

const uint64_t Fib::GRACE_PERIOD = 10;

void
Fib::remove(const ndn::Name& name)
{
  NLSR_LOG_DEBUG("Fib::remove called");
  std::map<ndn::Name, FibEntry>::iterator it = m_table.find(name);
  if (it != m_table.end()) {
    for (std::set<NextHop, NextHopComparator>::iterator nhit =
           (it->second).getNexthopList().getNextHops().begin();
         nhit != (it->second).getNexthopList().getNextHops().end(); nhit++) {
      //remove entry from NDN-FIB
      // Only unregister the prefix if it ISN'T a neighbor.
      if (isPrefixUpdatable((it->second).getName())) {
        unregisterPrefix((it->second).getName(), nhit->getConnectingFaceUri());
      }
    }
    cancelEntryRefresh(it->second);
    m_table.erase(it);
  }
}

void
Fib::addNextHopsToFibEntryAndNfd(FibEntry& entry, NexthopList& hopsToAdd)
{
  const ndn::Name& name = entry.getName();

  for (NexthopList::iterator it = hopsToAdd.begin(); it != hopsToAdd.end(); ++it)
  {
    // Add nexthop to FIB entry
    entry.getNexthopList().addNextHop(*it);

    if (isPrefixUpdatable(name)) {
      // Add nexthop to NDN-FIB
      registerPrefix(name, ndn::FaceUri(it->getConnectingFaceUri()),
                     it->getRouteCostAsAdjustedInteger(),
                     ndn::time::seconds(m_refreshTime + GRACE_PERIOD),
                     ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
    }
  }
}

void
Fib::update(const ndn::Name& name, NexthopList& allHops)
{
  NLSR_LOG_DEBUG("Fib::update called");

  // Get the max possible faces which is the minumum of the configuration setting and
  // the length of the list of all next hops.
  unsigned int maxFaces = getNumberOfFacesForName(allHops);

  NexthopList hopsToAdd;
  unsigned int nFaces = 0;

  // Create a list of next hops to be installed with length == maxFaces
  for (NexthopList::iterator it = allHops.begin(); it != allHops.end() && nFaces < maxFaces;
       ++it, ++nFaces) {
    hopsToAdd.addNextHop(*it);
  }

  std::map<ndn::Name, FibEntry>::iterator entryIt = m_table.find(name);

  // New FIB entry that has nextHops
  if (entryIt == m_table.end() && hopsToAdd.size() != 0) {
    NLSR_LOG_DEBUG("New FIB Entry");

    FibEntry entry(name);

    addNextHopsToFibEntryAndNfd(entry, hopsToAdd);

    m_table.emplace(name, entry);

    entryIt = m_table.find(name);
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

    FibEntry& entry = (entryIt->second);
    addNextHopsToFibEntryAndNfd(entry, hopsToAdd);

    std::set<NextHop, NextHopComparator> hopsToRemove;
    std::set_difference(entry.getNexthopList().begin(), entry.getNexthopList().end(),
                        hopsToAdd.begin(), hopsToAdd.end(),
                        std::inserter(hopsToRemove, hopsToRemove.end()), NextHopComparator());

    bool isUpdatable = isPrefixUpdatable(entry.getName());
    // Remove the uninstalled next hops from NFD and FIB entry
    for (const auto& hop: hopsToRemove){
      if (isUpdatable) {
        unregisterPrefix(entry.getName(), hop.getConnectingFaceUri());
      }
      NLSR_LOG_DEBUG("Removing " << hop.getConnectingFaceUri() << " from " << entry.getName());
      entry.getNexthopList().removeNextHop(hop);
    }

    // Increment sequence number
    entry.setSeqNo(entry.getSeqNo() + 1);

    entryIt = m_table.find(name);

  }
  if (entryIt != m_table.end() && !entryIt->second.getRefreshEventId()) {
    scheduleEntryRefresh(entryIt->second,
                         [this] (FibEntry& entry) {
                           scheduleLoop(entry);
                         });
  }
}

void
Fib::clean()
{
  NLSR_LOG_DEBUG("Fib::clean called");
  for (std::map<ndn::Name, FibEntry>::iterator it = m_table.begin();
       it != m_table.end();
       ++it) {
    NLSR_LOG_DEBUG("Cancelling Scheduled event. Name: " << it->second.getName());
    cancelEntryRefresh(it->second);
    for (std::set<NextHop, NextHopComparator>::iterator nhit =
         (it->second).getNexthopList().getNextHops().begin();
         nhit != (it->second).getNexthopList().getNextHops().end(); nhit++) {
      //Remove entry from NDN-FIB
      unregisterPrefix((it->second).getName(), nhit->getConnectingFaceUri());
    }
  }
  if (m_table.size() > 0) {
    m_table.clear();
  }
}

unsigned int
Fib::getNumberOfFacesForName(NexthopList& nextHopList)
{
  uint32_t nNextHops = static_cast<uint32_t>(nextHopList.getNextHops().size());
  uint32_t nMaxFaces = m_confParameter.getMaxFacesPerPrefix();

  // Allow all faces
  if (nMaxFaces == 0) {
    return nNextHops;
  }
  else {
    return std::min(nNextHops, nMaxFaces);
  }
}

bool
Fib::isPrefixUpdatable(const ndn::Name& name) {
  if (!m_adjacencyList.isNeighbor(name)) {
    return true;
  }

  return false;
}

void
Fib::registerPrefix(const ndn::Name& namePrefix, const ndn::FaceUri& faceUri,
                    uint64_t faceCost,
                    const ndn::time::milliseconds& timeout,
                    uint64_t flags, uint8_t times)
{
  uint64_t faceId = m_adjacencyList.getFaceId(ndn::FaceUri(faceUri));

  if (faceId != 0) {
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
                                                   std::bind(&Fib::onRegistrationSuccess, this, _1,
                                                             "Successful in name registration",
                                                             faceUri),
                                                   std::bind(&Fib::onRegistrationFailure,
                                                             this, _1,
                                                             "Failed in name registration",
                                                             faceParameters,
                                                             faceUri, times));
  }
  else {
    NLSR_LOG_WARN("Error: No Face Id for face uri: " << faceUri);
  }
}

void
Fib::onRegistrationSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                           const std::string& message, const ndn::FaceUri& faceUri)
{
  NLSR_LOG_DEBUG(message << ": " << commandSuccessResult.getName() <<
             " Face Uri: " << faceUri << " faceId: " << commandSuccessResult.getFaceId());

  AdjacencyList::iterator adjacent = m_adjacencyList.findAdjacent(faceUri);
  if (adjacent != m_adjacencyList.end()) {
    adjacent->setFaceId(commandSuccessResult.getFaceId());
  }

  // Update the fast-access FaceMap with the new Face ID, too
  m_faceMap.update(faceUri.toString(), commandSuccessResult.getFaceId());
  m_faceMap.writeLog();
}

void
Fib::onRegistrationFailure(const ndn::nfd::ControlResponse& response,
                           const std::string& message,
                           const ndn::nfd::ControlParameters& parameters,
                           const ndn::FaceUri& faceUri,
                           uint8_t times)
{
  NLSR_LOG_DEBUG(message << ": " << response.getText() << " (code: " << response.getCode() << ")");
  NLSR_LOG_DEBUG("Prefix: " << parameters.getName() << " failed for: " << times);
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
Fib::unregisterPrefix(const ndn::Name& namePrefix, const std::string& faceUri)
{
  uint32_t faceId = m_faceMap.getFaceId(faceUri);
  NLSR_LOG_DEBUG("Unregister prefix: " << namePrefix << " Face Uri: " << faceUri);
  if (faceId > 0) {
    ndn::nfd::ControlParameters controlParameters;
    controlParameters
      .setName(namePrefix)
      .setFaceId(faceId)
      .setOrigin(ndn::nfd::ROUTE_ORIGIN_NLSR);
    m_controller.start<ndn::nfd::RibUnregisterCommand>(controlParameters,
                                                       std::bind(&Fib::onUnregistrationSuccess, this, _1,
                                                                 "Successful in unregistering name"),
                                                       std::bind(&Fib::onUnregistrationFailure,
                                                                 this, _1,
                                                                 "Failed in unregistering name"));
  }
}

void
Fib::onUnregistrationSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                             const std::string& message)
{
  NLSR_LOG_DEBUG("Unregister successful Prefix: " << commandSuccessResult.getName() <<
             " Face Id: " << commandSuccessResult.getFaceId());
}

void
Fib::onUnregistrationFailure(const ndn::nfd::ControlResponse& response,
                             const std::string& message)
{
  NLSR_LOG_DEBUG(message << ": " << response.getText() << " (code: " << response.getCode() << ")");
}

void
Fib::setStrategy(const ndn::Name& name, const std::string& strategy, uint32_t count)
{
  ndn::nfd::ControlParameters parameters;
  parameters
    .setName(name)
    .setStrategy(strategy);

  m_controller.start<ndn::nfd::StrategyChoiceSetCommand>(parameters,
                                                         std::bind(&Fib::onSetStrategySuccess, this, _1,
                                                              "Successfully set strategy choice"),
                                                         std::bind(&Fib::onSetStrategyFailure, this, _1,
                                                              parameters,
                                                              count,
                                                              "Failed to set strategy choice"));
}

void
Fib::onSetStrategySuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                         const std::string& message)
{
  NLSR_LOG_DEBUG(message << ": " << commandSuccessResult.getStrategy() << " "
            << "for name: " << commandSuccessResult.getName());
}

void
Fib::onSetStrategyFailure(const ndn::nfd::ControlResponse& response,
                          const ndn::nfd::ControlParameters& parameters,
                          uint32_t count,
                          const std::string& message)
{
  NLSR_LOG_DEBUG(message << ": " << parameters.getStrategy() << " "
            << "for name: " << parameters.getName());
  if (count < 3) {
    setStrategy(parameters.getName(), parameters.getStrategy().toUri(),count+1);
  }
}

void
Fib::scheduleEntryRefresh(FibEntry& entry, const afterRefreshCallback& refreshCallback)
{
  NLSR_LOG_DEBUG("Scheduling refresh for " << entry.getName()
                                       << " Seq Num: " << entry.getSeqNo()
                                       << " in " << m_refreshTime << " seconds");

  entry.setRefreshEventId(m_scheduler.scheduleEvent(ndn::time::seconds(m_refreshTime),
                                                    std::bind(&Fib::refreshEntry, this,
                                                              entry.getName(), refreshCallback)));
}

void
Fib::scheduleLoop(FibEntry& entry)
{
  scheduleEntryRefresh(entry,
                       std::bind(&Fib::scheduleLoop, this, _1));
}

void
Fib::cancelEntryRefresh(const FibEntry& entry)
{
  NLSR_LOG_DEBUG("Cancelling refresh for " << entry.getName() << " Seq Num: " << entry.getSeqNo());

  m_scheduler.cancelEvent(entry.getRefreshEventId());
  entry.getRefreshEventId().reset();
}

void
Fib::refreshEntry(const ndn::Name& name, afterRefreshCallback refreshCb)
{
  std::map<ndn::Name, FibEntry>::iterator it = m_table.find(name);

  if (it == m_table.end()) {
    return;
  }

  FibEntry& entry = it->second;
  NLSR_LOG_DEBUG("Refreshing " << entry.getName() << " Seq Num: " << entry.getSeqNo());

  // Increment sequence number
    entry.setSeqNo(entry.getSeqNo() + 1);

  for (const NextHop& hop : entry) {
    registerPrefix(entry.getName(),
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
  for (std::map<ndn::Name, FibEntry>::iterator it = m_table.begin();
       it != m_table.end();
       ++it) {
    (it->second).writeLog();
  }
}

} // namespace nlsr
