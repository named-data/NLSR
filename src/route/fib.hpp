/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
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
 */

#ifndef NLSR_ROUTE_FIB_HPP
#define NLSR_ROUTE_FIB_HPP

#include "test-access-control.hpp"
#include "nexthop-list.hpp"

#include <ndn-cxx/mgmt/nfd/controller.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/time.hpp>

namespace nlsr {

using NextHopsUriSortedSet = NexthopListT<NextHopUriSortedComparator>;

struct FibEntry
{
  ndn::Name name;
  ndn::scheduler::ScopedEventId refreshEventId;
  int32_t seqNo = 1;
  NextHopsUriSortedSet nexthopSet;
};

using AfterRefreshCallback = std::function<void(FibEntry&)>;

class AdjacencyList;
class ConfParameter;

/*! \brief Maps names to lists of next hops, and exports this information to NFD.
 *
 * The FIB (Forwarding Information Base) is the "authoritative" source
 * of how to route Interests on this router to other nodes running
 * NLSR. In essence, the FIB is a map that takes name prefixes to a
 * list of next-hops out of this router. This class also contains
 * methods to inform NFD about these relationships. The FIB has its
 * entries populated by the NamePrefixTable
 *
 * \sa nlsr::NamePrefixTable
 * \sa nlsr::NamePrefixTable::addEntry
 * \sa nlsr::NamePrefixTable::updateWithNewRoute
 */
class Fib
{
public:
  Fib(ndn::Face& face, ndn::Scheduler& scheduler, AdjacencyList& adjacencyList,
      ConfParameter& conf, ndn::security::KeyChain& keyChain);

  /*! \brief Completely remove a name prefix from the FIB.
   *
   * If a name prefix is found to no longer be reachable from this
   * router, it will be removed from the FIB and all of its next-hops
   * will be unregistered from NFD.
   *
   * \sa nlsr::NamePrefixTable::removeEntry
   */
  void
  remove(const ndn::Name& name);

  /*! \brief Set the nexthop list of a name.
   *
   * This method is the entry for others to add next-hop information
   * to the FIB. Formally put, this method registers in NFD all
   * next-hops in allHops, and unregisters the set difference of
   * newHops - oldHops. This method also schedules the regular refresh
   * of those next hops.
   *
   * \param name The name prefix that the next-hops apply to
   * \param allHops A complete list of next-hops to associate with name.
   */
  void
  update(const ndn::Name& name, const NexthopList& allHops);

  void
  setEntryRefreshTime(int32_t fert)
  {
    m_refreshTime = fert;
  }

  /*! \brief Inform NFD of a next-hop
   *
   * This method informs NFD of a next-hop for some name prefix. This
   * method actually submits the information to NFD's RIB, which then
   * aggregates its own best hops and updates NFD's (the actual)
   * FIB. Typically, NLSR's FIB and NFD's FIB will be almost the
   * same. However, this is not necessarily the case and there may be
   * cases when other sources of information provide better next-hops
   * to NFD that NLSR doesn't know about. For example, an operator
   * could set up a direct link to a node that isn't running NLSR.
   *
   * \param namePrefix The name prefix to register a next-hop for
   * \param faceUri The faceUri of the adjacent that this prefix can be reached through
   * \param faceCost The cost to reach namePrefix through faceUri
   * \param timeout How long this registration should last
   * \param flags Route inheritance flags (CAPTURE, CHILD_INHERIT)
   * \param times How many times we have failed to register this prefix since the last success.
   *
   * \sa Fib::registerPrefixInNfd
   */
  void
  registerPrefix(const ndn::Name& namePrefix,
                 const ndn::FaceUri& faceUri,
                 uint64_t faceCost,
                 const ndn::time::milliseconds& timeout,
                 uint64_t flags,
                 uint8_t times);

  void
  setStrategy(const ndn::Name& name, const ndn::Name& strategy, uint32_t count);

  void
  writeLog();

private:
  /*! \brief Indicates whether a prefix is a direct neighbor or not.
   *
   * \return Whether the name is NOT associated with a direct neighbor
   */
  bool
  isNotNeighbor(const ndn::Name& name);

  /*! \brief Does one half of the updating of a FibEntry with new next-hops.
   *
   * Adds nexthops to a FibEntry and registers them in NFD.
   * \sa Fib::update
   * \sa Fib::removeOldNextHopsFromFibEntryAndNfd
   */
  void
  addNextHopsToFibEntryAndNfd(FibEntry& entry, const NextHopsUriSortedSet& hopsToAdd);

  unsigned int
  getNumberOfFacesForName(const NexthopList& nextHopList);

  /*! \brief Unregisters a prefix from NFD's RIB.
   *
   */
  void
  unregisterPrefix(const ndn::Name& namePrefix, const ndn::FaceUri& faceUri);

  /*! \brief Log registration success, and update the Face ID associated with a URI.
   */
  void
  onRegistrationSuccess(const ndn::nfd::ControlParameters& param,
                        const ndn::FaceUri& faceUri);

  /*! \brief Retry a prefix (next-hop) registration up to three (3) times.
   */
  void
  onRegistrationFailure(const ndn::nfd::ControlResponse& response,
                        const ndn::nfd::ControlParameters& parameters,
                        const ndn::FaceUri& faceUri,
                        uint8_t times);

  /*! \brief Log a successful strategy setting.
   */
  void
  onSetStrategySuccess(const ndn::nfd::ControlParameters& commandSuccessResult);

  /*! \brief Retry a strategy setting up to three (3) times.
   */
  void
  onSetStrategyFailure(const ndn::nfd::ControlResponse& response,
                       const ndn::nfd::ControlParameters& parameters,
                       uint32_t count);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  /*! \brief Schedule a refresh event for an entry.
   *
   * Schedules a refresh event for an entry. In order to form a
   * perpetual loop, refreshCallback needs to call
   * Fib::scheduleEntryRefresh in some way, with refreshCallback being
   * the same each time. In the current implementation, this is
   * accomplished by having a separate function, Fib::scheduleLoop,
   * that does this work.
   * \sa Fib::scheduleLoop
   */
  void
  scheduleEntryRefresh(FibEntry& entry, const AfterRefreshCallback& refreshCb);

private:
  /*! \brief Continue the entry refresh cycle.
   */
  void
  scheduleLoop(FibEntry& entry);

  /*! \brief Refreshes an entry in NFD.
   */
  void
  refreshEntry(const ndn::Name& name, AfterRefreshCallback refreshCb);

public:
  static inline const ndn::Name MULTICAST_STRATEGY{"/localhost/nfd/strategy/multicast"};
  static inline const ndn::Name BEST_ROUTE_STRATEGY{"/localhost/nfd/strategy/best-route"};

  ndn::signal::Signal<Fib, ndn::Name> onPrefixRegistrationSuccess;

private:
  ndn::Scheduler& m_scheduler;
  int32_t m_refreshTime;
  ndn::nfd::Controller m_controller;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::map<ndn::Name, FibEntry> m_table;

private:
  AdjacencyList& m_adjacencyList;
  ConfParameter& m_confParameter;

  /*! GRACE_PERIOD A "window" we append to the timeout time to
   * allow for things like stuttering prefix registrations and
   * processing time when refreshing events.
   */
  static constexpr uint64_t GRACE_PERIOD = 10;
};

} // namespace nlsr

#endif // NLSR_ROUTE_FIB_HPP
