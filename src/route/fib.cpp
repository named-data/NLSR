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

#include "nlsr.hpp"
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
  return fibEntry.getName() == name ;
}

void
Fib::cancelScheduledExpiringEvent(EventId eid)
{
  m_nlsr.getScheduler().cancelEvent(eid);
}


ndn::EventId
Fib::scheduleEntryRefreshing(const ndn::Name& name, int32_t feSeqNum,
                             const ndn::time::seconds& expTime)
{
  _LOG_DEBUG("Fib::scheduleEntryRefreshing Called");
  _LOG_DEBUG("Name: " << name << " Seq Num: " << feSeqNum);
  return m_nlsr.getScheduler().scheduleEvent(expTime,
                                             ndn::bind(&Fib::refreshEntry, this,
                                                       name, feSeqNum));
}

void
Fib::refreshEntry(const ndn::Name& name, int32_t feSeqNum)
{
  _LOG_DEBUG("Fib::refreshEntry Called");
  _LOG_DEBUG("Name: " << name << " Seq Num: " << feSeqNum);
  std::list<FibEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&fibEntryNameCompare, _1, name));
  if (it != m_table.end()) {
    cancelScheduledExpiringEvent((*it).getExpiringEventId());
    _LOG_DEBUG("Refreshing the FIB entry. Name: " <<  name);
    for (std::list<NextHop>::iterator nhit =
           (*it).getNexthopList().getNextHops().begin();
           nhit != (*it).getNexthopList().getNextHops().end(); nhit++) {
      // add entry to NDN-FIB
      if (isPrefixUpdatable(it->getName())) {
        registerPrefix(it->getName(), nhit->getConnectingFaceUri(),
                       std::ceil(nhit->getRouteCost()),
                       ndn::time::seconds(m_refreshTime + GRACE_PERIOD));
      }
    }
    // increase sequence number and schedule refresh again
    it->setSeqNo(feSeqNum + 1);
    it->setExpiringEventId(scheduleEntryRefreshing(it->getName() ,
                                                   it->getSeqNo(),
                                                   ndn::time::seconds(m_refreshTime)));
  }
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


void
Fib::update(const ndn::Name& name, NexthopList& nextHopList)
{
  _LOG_DEBUG("Fib::updateFib Called");
  int startFace = 0;
  int endFace = getNumberOfFacesForName(nextHopList,
                                        m_nlsr.getConfParameter().getMaxFacesPerPrefix());
  std::list<FibEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&fibEntryNameCompare, _1, name));
  if (it == m_table.end()) {
    if (nextHopList.getSize() > 0) {
      nextHopList.sort();
      FibEntry newEntry(name);
      std::list<NextHop> nhl = nextHopList.getNextHops();
      std::list<NextHop>::iterator nhit = nhl.begin();
      for (int i = startFace; i < endFace && nhit != nhl.end(); ++nhit, i++) {
        newEntry.getNexthopList().addNextHop((*nhit));
        //Add entry to NDN-FIB
        if (isPrefixUpdatable(name)) {
          registerPrefix(name, nhit->getConnectingFaceUri(),
                         std::ceil(nhit->getRouteCost()),
                         ndn::time::seconds(m_refreshTime + GRACE_PERIOD));
        }
      }
      newEntry.getNexthopList().sort();
      ndn::time::system_clock::TimePoint expirationTimePoint = ndn::time::system_clock::now();
      expirationTimePoint = expirationTimePoint + ndn::time::seconds(m_refreshTime);
      newEntry.setExpirationTimePoint(expirationTimePoint);
      newEntry.setSeqNo(1);
      newEntry.setExpiringEventId(scheduleEntryRefreshing(name , 1,
                                                          ndn::time::seconds(m_refreshTime)));
      m_table.push_back(newEntry);
    }
  }
  else {
    _LOG_DEBUG("Old FIB Entry");
    if (nextHopList.getSize() > 0) {
      nextHopList.sort();
      if (!it->isEqualNextHops(nextHopList)) {
        std::list<NextHop> nhl = nextHopList.getNextHops();
        std::list<NextHop>::iterator nhit = nhl.begin();
        // Add first Entry to NDN-FIB
        if (isPrefixUpdatable(name)) {
          registerPrefix(name, nhit->getConnectingFaceUri(),
                         std::ceil(nhit->getRouteCost()),
                         ndn::time::seconds(m_refreshTime + GRACE_PERIOD));
        }
        removeHop(it->getNexthopList(), nhit->getConnectingFaceUri(), name);
        it->getNexthopList().reset();
        it->getNexthopList().addNextHop((*nhit));
        ++startFace;
        ++nhit;
        for (int i = startFace; i < endFace && nhit != nhl.end(); ++nhit, i++) {
          it->getNexthopList().addNextHop((*nhit));
          //Add Entry to NDN_FIB
          if (isPrefixUpdatable(name)) {
            registerPrefix(name, nhit->getConnectingFaceUri(),
                           std::ceil(nhit->getRouteCost()),
                           ndn::time::seconds(m_refreshTime + GRACE_PERIOD));
          }
        }
      }
      ndn::time::system_clock::TimePoint expirationTimePoint = ndn::time::system_clock::now();
      expirationTimePoint = expirationTimePoint + ndn::time::seconds(m_refreshTime);
      it->setExpirationTimePoint(expirationTimePoint);
      it->setSeqNo(it->getSeqNo() + 1);
      (*it).setExpiringEventId(scheduleEntryRefreshing(it->getName() ,
                                                       it->getSeqNo(),
                                                       ndn::time::seconds(m_refreshTime)));
    }
    else {
      remove(name);
    }
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

int
Fib::getNumberOfFacesForName(NexthopList& nextHopList,
                             uint32_t maxFacesPerPrefix)
{
  int endFace = 0;
  if ((maxFacesPerPrefix == 0) || (nextHopList.getSize() <= maxFacesPerPrefix)) {
    return nextHopList.getSize();
  }
  else {
    return maxFacesPerPrefix;
  }
  return endFace;
}

bool
Fib::isPrefixUpdatable(const ndn::Name& name) {
  if (!m_nlsr.getAdjacencyList().isNeighbor(name)) {
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
  ndn::nfd::ControlParameters faceParameters;
  faceParameters
    .setUri(faceUri);
  m_controller.start<ndn::nfd::FaceCreateCommand>(faceParameters,
                                                  onSuccess,
                                                  onFailure);
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
                    uint64_t faceCost, const ndn::time::milliseconds& timeout)
{
  createFace(faceUri,
             ndn::bind(&Fib::registerPrefixInNfd, this,_1, namePrefix, faceCost, timeout,
                       faceUri),
             ndn::bind(&Fib::onFailure, this, _1, _2,"Failed in name registration"));
}

void
Fib::registerPrefix(const ndn::Name& namePrefix,
                    const std::string& faceUri,
                    uint64_t faceCost,
                    const ndn::time::milliseconds& timeout,
                    const CommandSucceedCallback& onSuccess,
                    const CommandFailCallback& onFailure)

{
 createFace(faceUri,
            ndn::bind(&Fib::registerPrefixInNfd, this,_1,
                      namePrefix, faceCost, timeout, onSuccess, onFailure),
            onFailure);
}

void
Fib::registerPrefixInNfd(const ndn::nfd::ControlParameters& faceCreateResult, 
                         const ndn::Name& namePrefix, uint64_t faceCost,
                         const ndn::time::milliseconds& timeout,
                         const std::string& faceUri)
{
  ndn::nfd::ControlParameters controlParameters;
  controlParameters
    .setName(namePrefix)
    .setFaceId(faceCreateResult.getFaceId())
    .setCost(faceCost)
    .setExpirationPeriod(timeout)
    .setOrigin(128);
  m_controller.start<ndn::nfd::RibRegisterCommand>(controlParameters,
                                                   ndn::bind(&Fib::onRegistration, this, _1,
                                                             "Successful in name registration",
                                                             faceUri),
                                                   ndn::bind(&Fib::onFailure, this, _1, _2,
                                                             "Failed in name registration"));
}

void
Fib::registerPrefixInNfd(const ndn::nfd::ControlParameters& faceCreateResult,
                         const ndn::Name& namePrefix, uint64_t faceCost,
                         const ndn::time::milliseconds& timeout,
                         const CommandSucceedCallback& onSuccess,
                         const CommandFailCallback& onFailure)
{
  ndn::nfd::ControlParameters controlParameters;
  controlParameters
    .setName(namePrefix)
    .setFaceId(faceCreateResult.getFaceId())
    .setCost(faceCost)
    .setExpirationPeriod(timeout)
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
                                                     ndn::bind(&Fib::onFailure, this, _1, _2,
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
Fib::onFailure(uint32_t code, const std::string& error,
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
