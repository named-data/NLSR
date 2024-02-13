/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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

#include "route/nexthop-list.hpp"
#include "route/nexthop.hpp"
#include "route/fib.hpp"

#include "tests/boost-test.hpp"

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestNhl)

BOOST_AUTO_TEST_CASE(NhlAddNextHop)
{
  NextHop np1;

  NexthopList nhl1;

  nhl1.addNextHop(np1);
  BOOST_CHECK_EQUAL(nhl1.size(), (uint32_t)1);

  nhl1.removeNextHop(np1);
  BOOST_CHECK_EQUAL(nhl1.size(), (uint32_t)0);
}

BOOST_AUTO_TEST_CASE(LinkStateRemoveNextHop)
{
  NextHop hop1;
  hop1.setRouteCost(12.34);

  NexthopList hopList;
  hopList.addNextHop(hop1);

  NextHop hop2;
  hop2.setRouteCost(13.01);

  BOOST_REQUIRE_EQUAL(hopList.size(), 1);

  hopList.removeNextHop(hop2);
  BOOST_CHECK_EQUAL(hopList.size(), 1);

  hopList.removeNextHop(hop1);
  BOOST_CHECK_EQUAL(hopList.size(), 0);
}

BOOST_AUTO_TEST_CASE(HyperbolicRemoveNextHop)
{
  NextHop hop1;
  hop1.setHyperbolic(true);
  hop1.setRouteCost(12.34);

  NexthopList hopList;
  hopList.addNextHop(hop1);

  NextHop hop2;
  hop2.setHyperbolic(true);
  hop2.setRouteCost(12.35);

  BOOST_REQUIRE_EQUAL(hopList.size(), 1);

  hopList.removeNextHop(hop2);
  BOOST_CHECK_EQUAL(hopList.size(), 1);

  hopList.removeNextHop(hop1);
  BOOST_CHECK_EQUAL(hopList.size(), 0);
}

BOOST_AUTO_TEST_CASE(TieBreaker)
{
  // equal-cost hops are sorted consistently
  NextHop hopA;
  hopA.setRouteCost(25);
  hopA.setConnectingFaceUri(ndn::FaceUri("udp4://192.168.3.1:6363"));

  NextHop hopZ;
  hopZ.setRouteCost(25);
  hopZ.setConnectingFaceUri(ndn::FaceUri("udp4://192.168.3.9:6363"));

  NexthopList list;
  list.addNextHop(hopA);
  list.addNextHop(hopZ);

  auto it = list.begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), hopA.getConnectingFaceUri());

  list.clear();
  list.addNextHop(hopZ);
  list.addNextHop(hopA);

  it = list.begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), hopA.getConnectingFaceUri());
}

BOOST_AUTO_TEST_CASE(SortOnAddAndRemove)
{
  NexthopList list;

  NextHop hopA(ndn::FaceUri("udp4://192.168.3.1:6363"), 10);
  NextHop hopB(ndn::FaceUri("udp4://192.168.3.2:6363"), 5);
  NextHop hopC(ndn::FaceUri("udp4://192.168.3.3:6363"), 25);

  list.addNextHop(hopA);
  list.addNextHop(hopB);
  list.addNextHop(hopC);

  BOOST_REQUIRE_EQUAL(list.size(), 3);

  double lastCost = 0;
  for (const auto& hop : list) {
    BOOST_CHECK(hop.getRouteCost() > lastCost);
    lastCost = hop.getRouteCost();
  }

  // removing a hop keep the list sorted
  list.removeNextHop(hopA);

  BOOST_REQUIRE_EQUAL(list.size(), 2);

  lastCost = 0;
  for (const auto& hop : list) {
    BOOST_CHECK(hop.getRouteCost() > lastCost);
    lastCost = hop.getRouteCost();
  }
}

/* If there are two NextHops going to the same neighbor, then the list
   should always select the one with the cheaper cost. This would be
   caused by a Name being advertised by two different routers, which
   are reachable through the same neighbor.
 */
BOOST_AUTO_TEST_CASE(UseCheaperNextHop)
{
  NexthopList list;

  NextHop hopA(ndn::FaceUri("udp4://10.0.0.1:6363"), 10);
  NextHop hopB(ndn::FaceUri("udp4://10.0.0.1:6363"), 5);

  list.addNextHop(hopA);
  list.addNextHop(hopB);

  BOOST_REQUIRE_EQUAL(list.size(), 1);

  for (const auto& hop : list) {
    BOOST_CHECK_EQUAL(hop, hopB);
  }
}

/* Fib needs a NexthopList to be sorted by FaceUri when updating
   to avoid removing prefixes that were just installed. The above
   test does not apply to this scenario as the NexthopList
   sorted by cost is given to the Fib::update.
 */
BOOST_AUTO_TEST_CASE(NextHopListDiffForFibUpdate) // #5179
{
  // If default sorter is used then difference results in
  // the same hops to remove as those that were added
  NexthopList nhl1;
  nhl1.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.13:6363"), 28));
  nhl1.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.9:6363"), 38));
  nhl1.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.26:6363"), 44));

  NexthopList nhl2;
  nhl2.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.9:6363"), 21));
  nhl2.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.13:6363"), 26));
  nhl2.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.26:6363"), 42));

  std::set<NextHop> hopsToRemove;
  std::set_difference(nhl2.begin(), nhl2.end(),
                      nhl1.begin(), nhl1.end(),
                      std::inserter(hopsToRemove, hopsToRemove.begin()));

  BOOST_CHECK_EQUAL(hopsToRemove.size(), 3);

  // Sorted by FaceUri
  NextHopsUriSortedSet nhs1;
  nhs1.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.13:6363"), 28));
  nhs1.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.9:6363"), 38));
  nhs1.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.26:6363"), 44));

  NextHopsUriSortedSet nhs2;
  nhs2.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.9:6363"), 21));
  nhs2.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.13:6363"), 26));
  nhs2.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.26:6363"), 42));

  std::set<NextHop, NextHopUriSortedComparator> hopsToRemove2;
  std::set_difference(nhs2.begin(), nhs2.end(),
                      nhs1.begin(), nhs1.end(),
                      std::inserter(hopsToRemove2, hopsToRemove2.begin()),
                      NextHopUriSortedComparator());

  BOOST_CHECK_EQUAL(hopsToRemove2.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
