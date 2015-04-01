/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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
 **/

#include "route/nexthop-list.hpp"
#include "route/nexthop.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestNhl)

BOOST_AUTO_TEST_CASE(NhlAddNextHop)
{
  NextHop np1;

  NexthopList nhl1;

  nhl1.addNextHop(np1);
  BOOST_CHECK_EQUAL(nhl1.getSize(), (uint32_t)1);

  nhl1.removeNextHop(np1);
  BOOST_CHECK_EQUAL(nhl1.getSize(), (uint32_t)0);
}

BOOST_AUTO_TEST_CASE(LinkStateRemoveNextHop)
{
  NextHop hop1;
  hop1.setRouteCost(12.34);

  NexthopList hopList;
  hopList.addNextHop(hop1);

  NextHop hop2;
  hop2.setRouteCost(13.01);

  BOOST_REQUIRE_EQUAL(hopList.getSize(), 1);

  hopList.removeNextHop(hop2);
  BOOST_CHECK_EQUAL(hopList.getSize(), 1);

  hopList.removeNextHop(hop1);
  BOOST_CHECK_EQUAL(hopList.getSize(), 0);
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

  BOOST_REQUIRE_EQUAL(hopList.getSize(), 1);

  hopList.removeNextHop(hop2);
  BOOST_CHECK_EQUAL(hopList.getSize(), 1);

  hopList.removeNextHop(hop1);
  BOOST_CHECK_EQUAL(hopList.getSize(), 0);
}

BOOST_AUTO_TEST_CASE(TieBreaker)
{
  // equal-cost hops are sorted lexicographically
  NextHop hopA;
  hopA.setRouteCost(25);
  hopA.setConnectingFaceUri("AAAZZ");

  NextHop hopZ;
  hopZ.setRouteCost(25);
  hopZ.setConnectingFaceUri("ZZA");

  NexthopList list;
  list.addNextHop(hopA);
  list.addNextHop(hopZ);

  NexthopList::iterator it = list.begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), hopA.getConnectingFaceUri());

  list.reset();
  list.addNextHop(hopZ);
  list.addNextHop(hopA);

  it = list.begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), hopA.getConnectingFaceUri());


  // equal-cost and lexicographically equal hops are sorted by the length of their face uris
  NextHop longUriHop;
  longUriHop.setRouteCost(25);
  longUriHop.setConnectingFaceUri("AAAAAA");

  NextHop shortUriHop;
  shortUriHop.setRouteCost(25);
  shortUriHop.setConnectingFaceUri("AAA");

  list.reset();
  list.addNextHop(longUriHop);
  list.addNextHop(shortUriHop);

  it = list.begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), shortUriHop.getConnectingFaceUri());
}

BOOST_AUTO_TEST_CASE(SortOnAddAndRemove)
{
  NexthopList list;

  NextHop hopA("A", 10);
  NextHop hopB("B", 5);
  NextHop hopC("C", 25);

  list.addNextHop(hopA);
  list.addNextHop(hopB);
  list.addNextHop(hopC);

  BOOST_REQUIRE_EQUAL(list.getSize(), 3);

  double lastCost = 0;
  for (const auto& hop : list) {
    BOOST_CHECK(hop.getRouteCost() > lastCost);
    lastCost = hop.getRouteCost();
  }

  // removing a hop keep the list sorted
  list.removeNextHop(hopA);

  BOOST_REQUIRE_EQUAL(list.getSize(), 2);

  lastCost = 0;
  for (const auto& hop : list) {
    BOOST_CHECK(hop.getRouteCost() > lastCost);
    lastCost = hop.getRouteCost();
  }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
