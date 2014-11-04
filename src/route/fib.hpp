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
#ifndef NLSR_FIB_HPP
#define NLSR_FIB_HPP

#include <list>
#include <boost/cstdint.hpp>

#include <ndn-cxx/management/nfd-controller.hpp>
#include <ndn-cxx/util/time.hpp>
#include "face-map.hpp"
#include "fib-entry.hpp"
#include "test-access-control.hpp"
#include "utility/face-controller.hpp"

namespace nlsr {

typedef ndn::function<void(const ndn::nfd::ControlParameters&)> CommandSucceedCallback;
typedef ndn::function<void(uint32_t/*code*/,const std::string&/*reason*/)> CommandFailCallback;

class AdjacencyList;
class ConfParameter;

class Fib
{
public:
  Fib(ndn::Face& face, ndn::Scheduler& scheduler, AdjacencyList& adjacencyList, ConfParameter& conf,
      ndn::KeyChain& keyChain)
    : m_scheduler(scheduler)
    , m_table()
    , m_refreshTime(0)
    , m_controller(face, keyChain)
    , m_faceController(face.getIoService(), m_controller)
    , m_faceMap()
    , m_adjacencyList(adjacencyList)
    , m_confParameter(conf)
  {
  }

  ~Fib()
  {
  }

  void
  remove(const ndn::Name& name);

  void
  update(const ndn::Name& name, NexthopList& allHops);

  void
  clean();

  void
  setEntryRefreshTime(int32_t fert)
  {
    m_refreshTime = fert;
  }

private:
  bool
  isPrefixUpdatable(const ndn::Name& name);

  void
  addNextHopsToFibEntryAndNfd(FibEntry& entry, NexthopList& hopsToAdd);

  void
  removeOldNextHopsFromFibEntryAndNfd(FibEntry& entry, const NexthopList& installedHops);

  void
  removeHop(NexthopList& nl, const std::string& doNotRemoveHopFaceUri,
            const ndn::Name& name);

  unsigned int
  getNumberOfFacesForName(NexthopList& nextHopList);

  ndn::EventId
  scheduleEntryExpiration(const ndn::Name& name, int32_t feSeqNum,
                          const ndn::time::seconds& expTime);

  void
  cancelScheduledExpiringEvent(ndn::EventId eid);

public:
  void
  registerPrefix(const ndn::Name& namePrefix, const std::string& faceUri,
                 uint64_t faceCost,
                 const ndn::time::milliseconds& timeout,
                 uint64_t flags, uint8_t times);

  void
  registerPrefix(const ndn::Name& namePrefix,
                 const std::string& faceUri,
                 uint64_t faceCost,
                 const ndn::time::milliseconds& timeout,
                 uint64_t flags,
                 uint8_t times,
                 const CommandSucceedCallback& onSuccess,
                 const CommandFailCallback& onFailure);

  void
  setStrategy(const ndn::Name& name, const std::string& strategy, uint32_t count);

  void
  writeLog();

  void
  destroyFace(const std::string& faceUri,
              const CommandSucceedCallback& onSuccess,
              const CommandFailCallback& onFailure);

private:
  void
  createFace(const std::string& faceUri,
             const CommandSucceedCallback& onSuccess,
             const CommandFailCallback& onFailure);

  void
  registerPrefixInNfd(ndn::nfd::ControlParameters& parameters,
                      const std::string& faceUri,
                      uint8_t times);

  void
  registerPrefixInNfd(const ndn::nfd::ControlParameters& faceCreateResult,
                      const ndn::nfd::ControlParameters& parameters,
                      uint8_t times,
                      const CommandSucceedCallback& onSuccess,
                      const CommandFailCallback& onFailure);

  void
  destroyFaceInNfd(const ndn::nfd::ControlParameters& faceDestroyResult,
                   const CommandSucceedCallback& onSuccess,
                   const CommandFailCallback& onFailure);

  void
  unregisterPrefix(const ndn::Name& namePrefix, const std::string& faceUri);

  void
  onRegistration(const ndn::nfd::ControlParameters& commandSuccessResult,
                 const std::string& message, const std::string& faceUri);

  void
  onUnregistration(const ndn::nfd::ControlParameters& commandSuccessResult,
                   const std::string& message);

  void
  onRegistrationFailure(uint32_t code, const std::string& error,
                        const std::string& message,
                        const ndn::nfd::ControlParameters& parameters,
                        const std::string& faceUri,
                        uint8_t times);

  void
  onUnregistrationFailure(uint32_t code, const std::string& error,
                        const std::string& message);

  void
  onSetStrategySuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                       const std::string& message);

  void
  onSetStrategyFailure(uint32_t code, const std::string& error,
                       const ndn::nfd::ControlParameters& parameters,
                       uint32_t count,
                       const std::string& message);

private:
  ndn::Scheduler& m_scheduler;

  std::list<FibEntry> m_table;
  int32_t m_refreshTime;
  ndn::nfd::Controller m_controller;
  util::FaceController m_faceController;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  FaceMap m_faceMap;

private:
  AdjacencyList& m_adjacencyList;
  ConfParameter& m_confParameter;

  static const uint64_t GRACE_PERIOD;
};

}//namespace nlsr
#endif //NLSR_FIB_HPP
