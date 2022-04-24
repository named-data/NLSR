/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  The University of Memphis,
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

namespace nlsr {
namespace test {

BOOST_AUTO_TEST_SUITE(TestNexthop)

double
getHyperbolicAdjustedDecimal(unsigned int i)
{
  return static_cast<double>(i)/(10*NextHop::HYPERBOLIC_COST_ADJUSTMENT_FACTOR);
}

uint64_t
applyHyperbolicFactorAndRound(double d)
{
  return round(NextHop::HYPERBOLIC_COST_ADJUSTMENT_FACTOR*d);
}

BOOST_AUTO_TEST_CASE(LinkStateSetAndGet)
{
  NextHop hop1;
  hop1.setConnectingFaceUri("udp://test/uri");
  hop1.setRouteCost(12.34);

  BOOST_CHECK_EQUAL(hop1.getConnectingFaceUri(), "udp://test/uri");
  BOOST_CHECK_EQUAL(hop1.getRouteCost(), 12.34);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), 12);

  NextHop hop2;

  hop2.setRouteCost(12.34);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), hop2.getRouteCostAsAdjustedInteger());
}

BOOST_AUTO_TEST_CASE(HyperbolicSetAndGet)
{
  NextHop hop1;
  hop1.setHyperbolic(true);
  hop1.setConnectingFaceUri("udp://test/uri");
  hop1.setRouteCost(12.34);

  BOOST_CHECK_EQUAL(hop1.getConnectingFaceUri(), "udp://test/uri");
  BOOST_CHECK_EQUAL(hop1.getRouteCost(), 12.34);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), applyHyperbolicFactorAndRound(12.34));

  NextHop hop2;
  hop2.setHyperbolic(true);

  hop2.setRouteCost(12.34);
  BOOST_CHECK_EQUAL(hop1.getRouteCostAsAdjustedInteger(), hop2.getRouteCostAsAdjustedInteger());

  hop2.setRouteCost(12.35);
  BOOST_CHECK(hop1.getRouteCostAsAdjustedInteger() < hop2.getRouteCostAsAdjustedInteger());
}

BOOST_AUTO_TEST_CASE(HyperbolicRound)
{
  NextHop hop1;
  hop1.setHyperbolic(true);
  hop1.setConnectingFaceUri("udp://test/uri");
  hop1.setRouteCost(1 + getHyperbolicAdjustedDecimal(6));

  BOOST_CHECK_EQUAL(hop1.getConnectingFaceUri(), "udp://test/uri");
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
  BOOST_CHECK(hop1.getRouteCostAsAdjustedInteger() > hop2.getRouteCostAsAdjustedInteger());
}

const uint8_t NexthopData[] =
{
  // Header
  0x8f, 0x1d,
  // Uri
  0x8d, 0x11, 0x2f, 0x74, 0x65, 0x73, 0x74, 0x2f, 0x6e, 0x65, 0x78, 0x74, 0x68, 0x6f,
  0x70, 0x2f, 0x74, 0x6c, 0x76,
  // Cost
  0x86, 0x08, 0x3f, 0xfa, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66
};

BOOST_AUTO_TEST_CASE(NexthopEncode)
{
  NextHop nexthops1;
  nexthops1.setConnectingFaceUri("/test/nexthop/tlv");
  nexthops1.setRouteCost(1.65);

  const ndn::Block& wire = nexthops1.wireEncode();
  BOOST_REQUIRE_EQUAL_COLLECTIONS(NexthopData,
                                  NexthopData + sizeof(NexthopData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(NexthopDecode)
{
  NextHop nexthops1;

  nexthops1.wireDecode(ndn::Block(NexthopData, sizeof(NexthopData)));

  BOOST_REQUIRE_EQUAL(nexthops1.getConnectingFaceUri(), "/test/nexthop/tlv");
  BOOST_REQUIRE_EQUAL(nexthops1.getRouteCost(), 1.65);
}

BOOST_AUTO_TEST_CASE(AdjacencyOutputStream)
{
  NextHop nexthops1;
  nexthops1.setConnectingFaceUri("/test/nexthop/tlv");
  nexthops1.setRouteCost(99);

  std::ostringstream os;
  os << nexthops1;
  BOOST_CHECK_EQUAL(os.str(), "NextHop(Uri: /test/nexthop/tlv, Cost: 99)");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
