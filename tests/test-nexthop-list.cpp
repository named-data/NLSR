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
  NextHop hopA;
  hopA.setRouteCost(25);
  hopA.setConnectingFaceUri("AAA");

  NextHop hopZ;
  hopZ.setRouteCost(25);
  hopZ.setConnectingFaceUri("ZZZ");

  NexthopList list;
  list.addNextHop(hopA);
  list.addNextHop(hopZ);

  list.sort();

  NexthopList::iterator it = list.begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), hopA.getConnectingFaceUri());

  list.reset();

  list.addNextHop(hopZ);
  list.addNextHop(hopA);

  list.sort();

  it = list.begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), hopA.getConnectingFaceUri());
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
