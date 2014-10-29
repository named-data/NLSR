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
#include <list>
#include <cmath>
#include <ndn-cxx/common.hpp>

#include "adjacency-list.hpp"
#include "common.hpp"
#include "conf-parameter.hpp"
#include "nexthop-list.hpp"
#include "face-map.hpp"
#include "fib.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("Fib");

const uint64_t Fib::GRACE_PERIOD = 10;

using namespace std;
using namespace ndn;

static bool
fibEntryNameCompare(const FibEntry& fibEntry, const ndn::Name& name)
{
  return fibEntry.getName() == name;
}

void
Fib::cancelScheduledExpiringEvent(EventId eid)
{
  m_scheduler.cancelEvent(eid);
}


ndn::EventId
Fib::scheduleEntryExpiration(const ndn::Name& name, int32_t feSeqNum,
                             const ndn::time::seconds& expTime)
{
  _LOG_DEBUG("Fib::scheduleEntryExpiration Called");
  _LOG_INFO("Name: " << name << " Seq Num: " << feSeqNum);

  return m_scheduler.scheduleEvent(expTime, ndn::bind(&Fib::remove, this, name));
}

void
Fib::remove(const ndn::Name& name)
{
  _LOG_DEBUG("Fib::remove called");
  std::list<FibEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&fibEntryNameCompare, _1, name));
  if (it != m_table.end()) {
    for (std::list<NextHop>::iterator nhit =
           (*it).getNexthopList().getNextHops().begin();
         nhit != (*it).getNexthopList().getNextHops().end(); nhit++) {
      //remove entry from NDN-FIB
      if (isPrefixUpdatable(it->getName())) {
        unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
      }
    }
    _LOG_DEBUG("Cancelling Scheduled event. Name: " << name);
    cancelScheduledExpiringEvent((*it).getExpiringEventId());
    m_table.erase(it);
  }
}

bool
compareFaceUri(const NextHop& hop, const std::string& faceUri)
{
  return hop.getConnectingFaceUri() == faceUri;
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
      registerPrefix(name, it->getConnectingFaceUri(),
                     it->getRouteCostAsAdjustedInteger(),
                     ndn::time::seconds(m_refreshTime + GRACE_PERIOD),
                     ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
    }
  }

  entry.getNexthopList().sort();
}

void
Fib::removeOldNextHopsFromFibEntryAndNfd(FibEntry& entry, const NexthopList& installedHops)
{
  _LOG_DEBUG("Fib::removeOldNextHopsFromFibEntryAndNfd Called");

  const ndn::Name& name = entry.getName();
  NexthopList& entryHopList = entry.getNexthopList();

  for (NexthopList::iterator it = entryHopList.begin(); it != entryHopList.end();) {

    const std::string& faceUri = it->getConnectingFaceUri();

    // See if the nexthop is installed in NFD's FIB
    NexthopList::const_iterator foundIt = std::find_if(installedHops.cbegin(),
                                                       installedHops.cend(),
                                                       bind(&compareFaceUri, _1, faceUri));

    // The next hop is not installed
    if (foundIt == installedHops.cend()) {

      if (isPrefixUpdatable(name)) {
        // Remove the nexthop from NDN's FIB
        unregisterPrefix(name, it->getConnectingFaceUri());
      }

      // Remove the next hop from the FIB entry
      _LOG_DEBUG("Removing " << it->getConnectingFaceUri() << " from " << name);
      // Since the iterator will be invalidated on removal, dereference the original
      // and increment the copy
      entryHopList.removeNextHop(*(it++));
    }
    else {
      ++it;
    }
  }
}

void
Fib::update(const ndn::Name& name, NexthopList& allHops)
{
  _LOG_DEBUG("Fib::update called");

  // Sort all of the next hops so lower cost hops are prioritized
  allHops.sort();

  // Get the max possible faces which is the minumum of the configuration setting and
  // the length of the list of all next hops.
  unsigned int maxFaces = getNumberOfFacesForName(allHops);

  NexthopList hopsToAdd;
  unsigned int nFaces = 0;

  // Create a list of next hops to be installed with length == maxFaces
  for (NexthopList::iterator it = allHops.begin(); it != allHops.end() && nFaces < maxFaces;
       ++it, ++nFaces)
  {
    hopsToAdd.addNextHop(*it);
  }

  std::list<FibEntry>::iterator entryIt = std::find_if(m_table.begin(),
                                                       m_table.end(),
                                                       bind(&fibEntryNameCompare, _1, name));

  // New FIB entry
  if (entryIt == m_table.end()) {
    _LOG_DEBUG("New FIB Entry");

    // Don't create an entry for a name with no nexthops
    if (hopsToAdd.getSize() == 0) {
      return;
    }

    FibEntry entry(name);

    addNextHopsToFibEntryAndNfd(entry, hopsToAdd);

    // Set entry's expiration time point and sequence number
    entry.setExpirationTimePoint(ndn::time::system_clock::now() +
                                  ndn::time::seconds(m_refreshTime));
    entry.setSeqNo(1);

    // Schedule entry to be refreshed
    entry.setExpiringEventId(scheduleEntryExpiration(name , entry.getSeqNo(),
                                                     ndn::time::seconds(m_refreshTime)));
    m_table.push_back(entry);
  }
  else {
    // Existing FIB entry
    _LOG_DEBUG("Existing FIB Entry");

    FibEntry& entry = *entryIt;

    // Remove empty FIB entry
    if (hopsToAdd.getSize() == 0) {
      remove(name);
      return;
    }

    addNextHopsToFibEntryAndNfd(entry, hopsToAdd);

    removeOldNextHopsFromFibEntryAndNfd(entry, hopsToAdd);

    // Set entry's expiration time point
    entry.setExpirationTimePoint(ndn::time::system_clock::now() +
                                  ndn::time::seconds(m_refreshTime));
    // Increment sequence number
    entry.setSeqNo(entry.getSeqNo() + 1);

    // Cancel previously scheduled event
    m_scheduler.cancelEvent(entry.getExpiringEventId());

    // Schedule entry to be refreshed
    entry.setExpiringEventId(scheduleEntryExpiration(name , entry.getSeqNo(),
                                                     ndn::time::seconds(m_refreshTime)));
  }
}

void
Fib::clean()
{
  _LOG_DEBUG("Fib::clean called");
  for (std::list<FibEntry>::iterator it = m_table.begin(); it != m_table.end();
       ++it) {
    _LOG_DEBUG("Cancelling Scheduled event. Name: " << it->getName());
    cancelScheduledExpiringEvent((*it).getExpiringEventId());
    for (std::list<NextHop>::iterator nhit =
         (*it).getNexthopList().getNextHops().begin();
         nhit != (*it).getNexthopList().getNextHops().end(); nhit++) {
      //Remove entry from NDN-FIB
      unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
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
Fib::removeHop(NexthopList& nl, const std::string& doNotRemoveHopFaceUri,
               const ndn::Name& name)
{
  for (std::list<NextHop>::iterator it = nl.getNextHops().begin();
       it != nl.getNextHops().end();   ++it) {
    if (it->getConnectingFaceUri() != doNotRemoveHopFaceUri) {
      //Remove FIB Entry from NDN-FIB
      if (isPrefixUpdatable(name)) {
        unregisterPrefix(name, it->getConnectingFaceUri());
      }
    }
  }
}

void
Fib::createFace(const std::string& faceUri,
                const CommandSucceedCallback& onSuccess,
                const CommandFailCallback& onFailure)
{
  m_faceController.createFace(faceUri, onSuccess, onFailure);
}

void
Fib::destroyFace(const std::string& faceUri,
                 const CommandSucceedCallback& onSuccess,
                 const CommandFailCallback& onFailure)
{
  createFace(faceUri,
             ndn::bind(&Fib::destroyFaceInNfd, this, _1, onSuccess, onFailure),
             onFailure);
}

void
Fib::destroyFaceInNfd(const ndn::nfd::ControlParameters& faceDestroyResult,
                        const CommandSucceedCallback& onSuccess,
                        const CommandFailCallback& onFailure)
{
  ndn::nfd::ControlParameters faceParameters;
  faceParameters
    .setFaceId(faceDestroyResult.getFaceId());
  m_controller.start<ndn::nfd::FaceDestroyCommand>(faceParameters,
                                                   onSuccess,
                                                   onFailure);
}

void
Fib::registerPrefix(const ndn::Name& namePrefix, const std::string& faceUri,
                    uint64_t faceCost, const ndn::time::milliseconds& timeout,
                    uint64_t flags, uint8_t times)
{
  uint64_t faceId = m_adjacencyList.getFaceId(faceUri);
  if (faceId != 0) {
    ndn::nfd::ControlParameters faceParameters;
    faceParameters
     .setName(namePrefix)
     .setFaceId(faceId)
     .setFlags(flags)
     .setCost(faceCost)
     .setExpirationPeriod(timeout)
     .setOrigin(128);

    _LOG_DEBUG("Registering prefix: " << namePrefix << " Face Uri: " << faceUri
               << " Face Id: " << faceId);
    registerPrefixInNfd(faceParameters, faceUri, times);
  }
  else {
    _LOG_DEBUG("Error: No Face Id for face uri: " << faceUri);
  }
}

typedef void(Fib::*RegisterPrefixCallback)(const ndn::nfd::ControlParameters&,
                                           const ndn::nfd::ControlParameters&, uint8_t,
                                           const CommandSucceedCallback&,
                                           const CommandFailCallback&);

void
Fib::registerPrefix(const ndn::Name& namePrefix,
                    const std::string& faceUri,
                    uint64_t faceCost,
                    const ndn::time::milliseconds& timeout,
                    uint64_t flags,
                    uint8_t times,
                    const CommandSucceedCallback& onSuccess,
                    const CommandFailCallback& onFailure)

{
  ndn::nfd::ControlParameters parameters;
  parameters
    .setName(namePrefix)
    .setFlags(flags)
    .setCost(faceCost)
    .setExpirationPeriod(timeout)
    .setOrigin(128);
  createFace(faceUri,
             ndn::bind(static_cast<RegisterPrefixCallback>(&Fib::registerPrefixInNfd),
                       this, _1, parameters, times, onSuccess, onFailure),
             onFailure);
}

void
Fib::registerPrefixInNfd(ndn::nfd::ControlParameters& parameters,
                         const std::string& faceUri,
                         uint8_t times)
{
  m_controller.start<ndn::nfd::RibRegisterCommand>(parameters,
                                                   ndn::bind(&Fib::onRegistration, this, _1,
                                                             "Successful in name registration",
                                                             faceUri),
                                                   ndn::bind(&Fib::onRegistrationFailure,
                                                             this, _1, _2,
                                                             "Failed in name registration",
                                                             parameters,
                                                             faceUri, times));
}

void
Fib::registerPrefixInNfd(const ndn::nfd::ControlParameters& faceCreateResult,
                         const ndn::nfd::ControlParameters& parameters,
                         uint8_t times,
                         const CommandSucceedCallback& onSuccess,
                         const CommandFailCallback& onFailure)
{
  ndn::nfd::ControlParameters controlParameters;
  controlParameters
    .setName(parameters.getName())
    .setFaceId(faceCreateResult.getFaceId())
    .setCost(parameters.getCost())
    .setFlags(parameters.getFlags())
    .setExpirationPeriod(parameters.getExpirationPeriod())
    .setOrigin(128);
  m_controller.start<ndn::nfd::RibRegisterCommand>(controlParameters,
                                                   onSuccess,
                                                   onFailure);
}

void
Fib::unregisterPrefix(const ndn::Name& namePrefix, const std::string& faceUri)
{
  uint32_t faceId = m_faceMap.getFaceId(faceUri);
  _LOG_DEBUG("Unregister prefix: " << namePrefix << " Face Uri: " << faceUri);
  if (faceId > 0) {
    ndn::nfd::ControlParameters controlParameters;
    controlParameters
      .setName(namePrefix)
      .setFaceId(faceId)
      .setOrigin(128);
    m_controller.start<ndn::nfd::RibUnregisterCommand>(controlParameters,
                                                     ndn::bind(&Fib::onUnregistration, this, _1,
                                                               "Successful in unregistering name"),
                                                     ndn::bind(&Fib::onUnregistrationFailure,
                                                               this, _1, _2,
                                                               "Failed in unregistering name"));
  }
}

void
Fib::setStrategy(const ndn::Name& name, const std::string& strategy, uint32_t count)
{
  ndn::nfd::ControlParameters parameters;
  parameters
    .setName(name)
    .setStrategy(strategy);

  m_controller.start<ndn::nfd::StrategyChoiceSetCommand>(parameters,
                                                         bind(&Fib::onSetStrategySuccess, this, _1,
                                                              "Successfully set strategy choice"),
                                                         bind(&Fib::onSetStrategyFailure, this, _1, _2,
                                                              parameters,
                                                              count,
                                                              "Failed to set strategy choice"));
}

void
Fib::onRegistration(const ndn::nfd::ControlParameters& commandSuccessResult,
                    const std::string& message, const std::string& faceUri)
{
  _LOG_DEBUG("Register successful Prefix: " << commandSuccessResult.getName() <<
             " Face Uri: " << faceUri);
  m_faceMap.update(faceUri, commandSuccessResult.getFaceId());
  m_faceMap.writeLog();
}

void
Fib::onUnregistration(const ndn::nfd::ControlParameters& commandSuccessResult,
                      const std::string& message)
{
  _LOG_DEBUG("Unregister successful Prefix: " << commandSuccessResult.getName() <<
             " Face Id: " << commandSuccessResult.getFaceId());
}

void
Fib::onRegistrationFailure(uint32_t code, const std::string& error,
                           const std::string& message,
                           const ndn::nfd::ControlParameters& parameters,
                           const std::string& faceUri,
                           uint8_t times)
{
  _LOG_DEBUG(message << ": " << error << " (code: " << code << ")");
  _LOG_DEBUG("Prefix: " << parameters.getName() << " failed for: " << times);
  if (times < 3) {
    _LOG_DEBUG("Trying to register again...");
    registerPrefix(parameters.getName(), faceUri,
                   parameters.getCost(),
                   parameters.getExpirationPeriod(),
                   parameters.getFlags(), times+1);
  }
  else {
    _LOG_DEBUG("Registration trial given up");
  }
}

void
Fib::onUnregistrationFailure(uint32_t code, const std::string& error,
                            const std::string& message)
{
  _LOG_DEBUG(message << ": " << error << " (code: " << code << ")");
}

void
Fib::onSetStrategySuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                         const std::string& message)
{
  _LOG_DEBUG(message << ": " << commandSuccessResult.getStrategy() << " "
            << "for name: " << commandSuccessResult.getName());
}

void
Fib::onSetStrategyFailure(uint32_t code, const std::string& error,
                         const ndn::nfd::ControlParameters& parameters,
                         uint32_t count,
                         const std::string& message)
{
  _LOG_DEBUG(message << ": " << parameters.getStrategy() << " "
            << "for name: " << parameters.getName());
  if (count < 3) {
    setStrategy(parameters.getName(), parameters.getStrategy().toUri(),count+1);
  }
}

void
Fib::writeLog()
{
  _LOG_DEBUG("-------------------FIB-----------------------------");
  for (std::list<FibEntry>::iterator it = m_table.begin(); it != m_table.end();
       ++it) {
    (*it).writeLog();
  }
}

} //namespace nlsr
