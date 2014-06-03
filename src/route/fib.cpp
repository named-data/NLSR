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
    if (it->getSeqNo() == feSeqNum) {
      _LOG_DEBUG("Refreshing the FIB entry. Name: " <<  name);
      for (std::list<NextHop>::iterator nhit =
             (*it).getNexthopList().getNextHops().begin();
           nhit != (*it).getNexthopList().getNextHops().end(); nhit++) {
        // add entry to NDN-FIB
        registerPrefix(it->getName(), nhit->getConnectingFaceUri(),
                       std::ceil(nhit->getRouteCost()), m_refreshTime);
      }
      // increase sequence number and schedule refresh again
      it->setSeqNo(feSeqNum + 1);
      it->setExpiringEventId(scheduleEntryRefreshing(it->getName() ,
                                                     it->getSeqNo(),
                                                     ndn::time::seconds(m_refreshTime)));
    }
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
      if (!m_nlsr.getAdjacencyList().isNeighbor(it->getName())) {
        unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
      }
      else
      {
        if (m_nlsr.getAdjacencyList().getAdjacent(it->getName()).getConnectingFaceUri() !=
            nhit->getConnectingFaceUri()) {
          unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
        }
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
        registerPrefix(name, nhit->getConnectingFaceUri(),
                       std::ceil(nhit->getRouteCost()), m_refreshTime);
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
        registerPrefix(name, nhit->getConnectingFaceUri(),
                       std::ceil(nhit->getRouteCost()), m_refreshTime);
        removeHop(it->getNexthopList(), nhit->getConnectingFaceUri(), name);
        it->getNexthopList().reset();
        it->getNexthopList().addNextHop((*nhit));
        ++startFace;
        ++nhit;
        for (int i = startFace; i < endFace && nhit != nhl.end(); ++nhit, i++) {
          it->getNexthopList().addNextHop((*nhit));
          //Add Entry to NDN_FIB
          registerPrefix(name, nhit->getConnectingFaceUri(),
                         std::ceil(nhit->getRouteCost()), m_refreshTime);
        }
      }
      ndn::time::system_clock::TimePoint expirationTimePoint = ndn::time::system_clock::now();
      expirationTimePoint = expirationTimePoint + ndn::time::seconds(m_refreshTime);
      it->setExpirationTimePoint(expirationTimePoint);
      _LOG_DEBUG("Cancelling Scheduled event. Name: " << name);
      cancelScheduledExpiringEvent(it->getExpiringEventId());
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
      if (!m_nlsr.getAdjacencyList().isNeighbor(it->getName())) {
        unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
      }
      else {
        if (m_nlsr.getAdjacencyList().getAdjacent(it->getName()).getConnectingFaceUri() !=
            nhit->getConnectingFaceUri()) {
          unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
        }
      }
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

void
Fib::removeHop(NexthopList& nl, const std::string& doNotRemoveHopFaceUri,
               const ndn::Name& name)
{
  for (std::list<NextHop>::iterator it = nl.getNextHops().begin();
       it != nl.getNextHops().end();   ++it) {
    if (it->getConnectingFaceUri() != doNotRemoveHopFaceUri) {
      //Remove FIB Entry from NDN-FIB
      if (!m_nlsr.getAdjacencyList().isNeighbor(name)) {
        unregisterPrefix(name, it->getConnectingFaceUri());
      }
      else {
        if (m_nlsr.getAdjacencyList().getAdjacent(name).getConnectingFaceUri() !=
            it->getConnectingFaceUri()) {
          unregisterPrefix(name, it->getConnectingFaceUri());
        }
      }
    }
  }
}

void
Fib::registerPrefix(const ndn::Name& namePrefix, const std::string& faceUri,
                    uint64_t faceCost, uint64_t timeout)
{
  ndn::nfd::ControlParameters faceParameters;
  faceParameters
  .setUri(faceUri);

  m_controller.start<ndn::nfd::FaceCreateCommand>(faceParameters,
                                                  ndn::bind(&Fib::registerPrefixInNfd, this,_1,
                                                            namePrefix, faceCost, timeout),
                                                  ndn::bind(&Fib::onFailure, this, _1, _2,
                                                             "Failed in name registration"));
  
}

void
Fib::registerPrefixInNfd(const ndn::nfd::ControlParameters& faceCreateResult, 
                         const ndn::Name& namePrefix, uint64_t faceCost, uint64_t timeout)
{
  ndn::nfd::ControlParameters controlParameters;
  controlParameters
    .setName(namePrefix)
    .setFaceId(faceCreateResult.getFaceId())
    .setCost(faceCost)
    .setExpirationPeriod(ndn::time::milliseconds(timeout * 1000))
    .setOrigin(128);
  m_controller.start<ndn::nfd::RibRegisterCommand>(controlParameters,
                                                   ndn::bind(&Fib::onRegistration, this, _1,
                                                             "Successful in name registration",
                                                             faceCreateResult.getUri()),
                                                   ndn::bind(&Fib::onFailure, this, _1, _2,
                                                             "Failed in name registration"));
}

void
Fib::unregisterPrefix(const ndn::Name& namePrefix, const std::string& faceUri)
{
  uint32_t faceId = m_faceMap.getFaceId(faceUri);
  if (faceId > 0) {
    ndn::nfd::ControlParameters controlParameters;
    controlParameters
      .setName(namePrefix)
      .setFaceId(faceId)
      .setOrigin(128);
    m_controller.start<ndn::nfd::RibUnregisterCommand>(controlParameters,
                                                     ndn::bind(&Fib::onSuccess, this, _1,
                                                               "Successful in unregistering name"),
                                                     ndn::bind(&Fib::onFailure, this, _1, _2,
                                                               "Failed in unregistering name"));
  }
}

void
Fib::setStrategy(const ndn::Name& name, const std::string& strategy)
{
  ndn::nfd::ControlParameters parameters;
  parameters
    .setName(name)
    .setStrategy(strategy);

  m_controller.start<ndn::nfd::StrategyChoiceSetCommand>(parameters,
                                                         bind(&Fib::onSuccess, this, _1,
                                                              "Successfully set strategy choice"),
                                                         bind(&Fib::onFailure, this, _1, _2,
                                                              "Failed to set strategy choice"));
}

void
Fib::onRegistration(const ndn::nfd::ControlParameters& commandSuccessResult,
                    const std::string& message, const std::string& faceUri)
{
  m_faceMap.update(faceUri, commandSuccessResult.getFaceId());
  m_faceMap.writeLog();
}


void
Fib::onSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
               const std::string& message)
{
}

void
Fib::onFailure(uint32_t code, const std::string& error,
               const std::string& message)
{
  _LOG_DEBUG(message << ": " << error << " (code: " << code << ")");
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
