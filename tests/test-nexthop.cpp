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
 * \author Ashlesh Gawande <agawande@memphis.edu>
 *
 **/
#include "route/nexthop.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestNexthop)

BOOST_AUTO_TEST_CASE(NexthopSetAndGet)
{
  NextHop hop1;
  hop1.setConnectingFaceUri("udp://test/uri");
  hop1.setRouteCost(12.34);

  BOOST_CHECK_EQUAL(hop1.getConnectingFaceUri(), "udp://test/uri");
  BOOST_CHECK_EQUAL(hop1.getRouteCost(), 12.34);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), 1234);

  NextHop hop2;

  hop2.setRouteCost(12.34);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), hop2.getRouteCostAsAdjustedInteger());

  hop2.setRouteCost(12.35);
  BOOST_CHECK(hop1.getRouteCostAsAdjustedInteger() < hop2.getRouteCostAsAdjustedInteger());
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
