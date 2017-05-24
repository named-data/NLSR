/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
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
 **/

#ifndef NLSR_ROUTE_FIB_HPP
#define NLSR_ROUTE_FIB_HPP

#include "face-map.hpp"
#include "fib-entry.hpp"
#include "test-access-control.hpp"
#include "utility/face-controller.hpp"

#include <ndn-cxx/mgmt/nfd/controller.hpp>
#include <ndn-cxx/util/time.hpp>

namespace nlsr {

typedef ndn::nfd::Controller::CommandSucceedCallback CommandSucceedCallback;
typedef ndn::nfd::Controller::CommandFailCallback CommandFailCallback;
typedef std::function<void(FibEntry&)> afterRefreshCallback;

class AdjacencyList;
class ConfParameter;
class FibEntry;

class Fib
{
public:
  Fib(ndn::Face& face, ndn::Scheduler& scheduler, AdjacencyList& adjacencyList, ConfParameter& conf,
      ndn::KeyChain& keyChain)
    : m_scheduler(scheduler)
    , m_refreshTime(0)
    , m_controller(face, keyChain)
    , m_faceController(face.getIoService(), m_controller)
    , m_adjacencyList(adjacencyList)
    , m_confParameter(conf)
  {
  }

  FibEntry*
  processUpdate(const ndn::Name& name, NexthopList& allHops);

  VIRTUAL_WITH_TESTS void
  remove(const ndn::Name& name);

  VIRTUAL_WITH_TESTS void
  update(const ndn::Name& name, NexthopList& allHops);

  void
  clean();

  void
  setEntryRefreshTime(int32_t fert)
  {
    m_refreshTime = fert;
  }

  void
  registerPrefix(const ndn::Name& namePrefix,
                 const std::string& faceUri,
                 uint64_t faceCost,
                 const ndn::time::milliseconds& timeout,
                 uint64_t flags,
                 uint8_t times);

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
  bool
  isPrefixUpdatable(const ndn::Name& name);

  void
  addNextHopsToFibEntryAndNfd(FibEntry& entry, NexthopList& hopsToAdd);

  void
  removeOldNextHopsFromFibEntryAndNfd(FibEntry& entry, const NexthopList& installedHops);

  unsigned int
  getNumberOfFacesForName(NexthopList& nextHopList);

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
  onRegistrationSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                        const std::string& message, const std::string& faceUri);

  void
  onRegistrationFailure(const ndn::nfd::ControlResponse& response,
                        const std::string& message,
                        const ndn::nfd::ControlParameters& parameters,
                        const std::string& faceUri,
                        uint8_t times);

  void
  onUnregistrationSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                          const std::string& message);

  void
  onUnregistrationFailure(const ndn::nfd::ControlResponse& response,
                               const std::string& message);

  void
  onSetStrategySuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                       const std::string& message);

  void
  onSetStrategyFailure(const ndn::nfd::ControlResponse& response,
                       const ndn::nfd::ControlParameters& parameters,
                       uint32_t count,
                       const std::string& message);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  void
  scheduleEntryRefresh(FibEntry& entry, const afterRefreshCallback& refreshCb);

private:
  void
  scheduleLoop(FibEntry& entry);

  void
  cancelEntryRefresh(const FibEntry& entry);

  void
  refreshEntry(const ndn::Name& name, afterRefreshCallback refreshCb);

private:
  ndn::Scheduler& m_scheduler;
  int32_t m_refreshTime;
  ndn::nfd::Controller m_controller;
  util::FaceController m_faceController;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  FaceMap m_faceMap;
  std::map<ndn::Name, FibEntry> m_table;

private:
  AdjacencyList& m_adjacencyList;
  ConfParameter& m_confParameter;

  static const uint64_t GRACE_PERIOD;
};

} // namespace nlsr

#endif // NLSR_ROUTE_FIB_HPP
