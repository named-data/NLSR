/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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
 *
 * \author Nicholas Gordon <nmgordon@memphis.edu>
 */

#include "route/routing-table-pool-entry.hpp"
#include "route/nexthop.hpp"
#include "route/nexthop-list.hpp"

#include "tests/boost-test.hpp"

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestRoutingTablePoolEntry)

BOOST_AUTO_TEST_CASE(EqualsOperator)
{
  NextHop hop1;
  hop1.setRouteCost(25);
  hop1.setConnectingFaceUri(ndn::FaceUri("udp4://192.168.3.1:6363"));

  NextHop hop2;
  hop2.setRouteCost(10);
  hop2.setConnectingFaceUri(ndn::FaceUri("udp4://192.168.3.2:6363"));

  NexthopList nhl1;
  NexthopList nhl2;

  nhl1.addNextHop(hop1);
  nhl1.addNextHop(hop2);

  RoutingTablePoolEntry rtpe1("/memphis/ndn/rtr1", 0);
  RoutingTablePoolEntry rtpe2("/memphis/ndn/rtr1", 0);

  rtpe1.setNexthopList(nhl1);
  rtpe2.setNexthopList(nhl1);

  BOOST_CHECK_EQUAL(rtpe1, rtpe2);
}

BOOST_AUTO_TEST_CASE(IncrementEntryUseCount)
{
  RoutingTablePoolEntry rtpe1("router1");

  rtpe1.incrementUseCount();

  BOOST_CHECK_EQUAL(rtpe1.getUseCount(), 2);
}

BOOST_AUTO_TEST_CASE(DecrementEntryUseCountNotZero)
{
  RoutingTablePoolEntry rtpe1("router1");

  rtpe1.decrementUseCount();

  BOOST_CHECK_EQUAL(rtpe1.getUseCount(), 0);
}

BOOST_AUTO_TEST_CASE(DecrementEntryUseCountAtZero)
{
  RoutingTablePoolEntry rtpe1("router1");

  rtpe1.decrementUseCount();
  rtpe1.decrementUseCount();

  BOOST_CHECK_EQUAL(rtpe1.getUseCount(), 0);
}

BOOST_AUTO_TEST_CASE(UpdateNextHopList)
{
  RoutingTablePoolEntry rtpe1("router1");
  NextHop nh1;
  NexthopList nhl1;

  nhl1.addNextHop(nh1);

  rtpe1.setNexthopList(nhl1);

  BOOST_CHECK_EQUAL(rtpe1.getNexthopList(), nhl1);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
