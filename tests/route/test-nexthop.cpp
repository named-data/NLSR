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
 */

#include "route/nexthop.hpp"

#include "tests/boost-test.hpp"

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestNexthop)

static const ndn::FaceUri faceUri1("udp4://192.168.3.1:6363");
static const ndn::FaceUri faceUri2("udp4://192.168.3.2:6363");

static double
getHyperbolicAdjustedDecimal(unsigned int i)
{
  return static_cast<double>(i)/(10 * NextHop::HYPERBOLIC_COST_ADJUSTMENT_FACTOR);
}

static uint64_t
applyHyperbolicFactorAndRound(double d)
{
  return round(NextHop::HYPERBOLIC_COST_ADJUSTMENT_FACTOR * d);
}

BOOST_AUTO_TEST_CASE(LinkStateSetAndGet)
{
  NextHop hop1;
  hop1.setConnectingFaceUri(faceUri1);
  hop1.setRouteCost(12.34);
  BOOST_CHECK_EQUAL(hop1.getConnectingFaceUri(), faceUri1);
  BOOST_CHECK_EQUAL(hop1.getRouteCost(), 12.34);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), 12);

  NextHop hop2(faceUri2, 12.34);
  BOOST_CHECK_NE(hop1, hop2);
  BOOST_CHECK_LT(hop1, hop2);
  BOOST_CHECK_LE(hop1, hop2);
  BOOST_CHECK_GT(hop2, hop1);
  BOOST_CHECK_GE(hop2, hop1);

  hop2.setConnectingFaceUri(faceUri1);
  BOOST_CHECK_EQUAL(hop1, hop2);
  BOOST_CHECK_LE(hop1, hop2);
  BOOST_CHECK_GE(hop2, hop1);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), hop2.getRouteCostAsAdjustedInteger());
}

BOOST_AUTO_TEST_CASE(HyperbolicSetAndGet)
{
  NextHop hop1;
  hop1.setHyperbolic(true);
  hop1.setConnectingFaceUri(faceUri1);
  hop1.setRouteCost(12.34);
  BOOST_CHECK_EQUAL(hop1.getConnectingFaceUri(), faceUri1);
  BOOST_CHECK_EQUAL(hop1.getRouteCost(), 12.34);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), applyHyperbolicFactorAndRound(12.34));

  NextHop hop2(faceUri2, 12.34);
  hop2.setHyperbolic(true);
  BOOST_CHECK_NE(hop1, hop2);
  BOOST_CHECK_LT(hop1, hop2);
  BOOST_CHECK_LE(hop1, hop2);
  BOOST_CHECK_GT(hop2, hop1);
  BOOST_CHECK_GE(hop2, hop1);

  hop2.setConnectingFaceUri(faceUri1);
  BOOST_CHECK_EQUAL(hop1, hop2);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), hop2.getRouteCostAsAdjustedInteger());

  hop2.setRouteCost(12.35);
  BOOST_CHECK_LT(hop1, hop2);
  BOOST_CHECK_LT(hop1.getRouteCostAsAdjustedInteger(), hop2.getRouteCostAsAdjustedInteger());
}

BOOST_AUTO_TEST_CASE(HyperbolicRound)
{
  NextHop hop1;
  hop1.setHyperbolic(true);
  hop1.setConnectingFaceUri(faceUri1);
  hop1.setRouteCost(1 + getHyperbolicAdjustedDecimal(6));

  BOOST_CHECK_EQUAL(hop1.getConnectingFaceUri(), faceUri1);
  BOOST_CHECK_EQUAL(hop1.getRouteCost(), 1 + getHyperbolicAdjustedDecimal(6));
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(),
                    applyHyperbolicFactorAndRound((1 + getHyperbolicAdjustedDecimal(6))));

  NextHop hop2;
  hop2.setHyperbolic(true);

  hop2.setRouteCost(1 + getHyperbolicAdjustedDecimal(6));
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), hop2.getRouteCostAsAdjustedInteger());

  hop2.setRouteCost(1 + getHyperbolicAdjustedDecimal(5));
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), hop2.getRouteCostAsAdjustedInteger());

  hop2.setRouteCost(1 + getHyperbolicAdjustedDecimal(4));
  BOOST_CHECK_GT(hop1.getRouteCostAsAdjustedInteger(), hop2.getRouteCostAsAdjustedInteger());
}

const uint8_t NexthopData[] =
{
  // Header
  0x8f, 0x23,
  // Uri
  0x8d, 0x17, 0x75, 0x64, 0x70, 0x34, 0x3a, 0x2f, 0x2f, 0x31, 0x39, 0x32, 0x2e, 0x31, 0x36, 0x38,
  0x2e, 0x33, 0x2e, 0x31, 0x3a, 0x36, 0x33, 0x36, 0x33,
  // Cost
  0x86, 0x08, 0x3f, 0xfa, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66
};

BOOST_AUTO_TEST_CASE(Encode)
{
  NextHop nexthops1;
  nexthops1.setConnectingFaceUri(faceUri1);
  nexthops1.setRouteCost(1.65);

  BOOST_TEST(nexthops1.wireEncode() == NexthopData, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(Decode)
{
  NextHop nexthops1;
  nexthops1.wireDecode(ndn::Block{NexthopData});

  BOOST_REQUIRE_EQUAL(nexthops1.getConnectingFaceUri(), faceUri1);
  BOOST_REQUIRE_EQUAL(nexthops1.getRouteCost(), 1.65);
}

BOOST_AUTO_TEST_CASE(OutputStream)
{
  NextHop nexthops1;
  nexthops1.setConnectingFaceUri(faceUri1);
  nexthops1.setRouteCost(99);

  std::ostringstream os;
  os << nexthops1;
  BOOST_CHECK_EQUAL(os.str(), "NextHop(Uri: udp4://192.168.3.1:6363, Cost: 99)");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
