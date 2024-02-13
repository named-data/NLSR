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

#include "route/routing-table-entry.hpp"

#include "tests/boost-test.hpp"

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestRoutingTableEntry)

BOOST_AUTO_TEST_CASE(Destination)
{
  RoutingTableEntry rte1("router1");

  BOOST_CHECK_EQUAL(rte1.getDestination(), "router1");
}

static const ndn::FaceUri NEXTHOP1("udp4://192.168.3.1:6363");
static const ndn::FaceUri NEXTHOP2("udp4://192.168.3.2:6363");

static const uint8_t RoutingTableEntryWithNexthopsData[] = {
  // Header
  0x91, 0x53,
  // Destination Name
  0x07, 0x07, 0x08, 0x05, 0x64, 0x65, 0x73, 0x74, 0x31,
  // Nexthop
  0x8f, 0x23,
  // Nexthop.Uri
  0x8d, 0x17, 0x75, 0x64, 0x70, 0x34, 0x3a, 0x2f, 0x2f, 0x31, 0x39, 0x32, 0x2e, 0x31, 0x36, 0x38,
  0x2e, 0x33, 0x2e, 0x31, 0x3a, 0x36, 0x33, 0x36, 0x33,
  // Nexthop.CostDouble
  0x86, 0x08, 0x3f, 0xfa, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
  // Nexthop
  0x8f, 0x23,
  // Nexthop.Uri
  0x8d, 0x17, 0x75, 0x64, 0x70, 0x34, 0x3a, 0x2f, 0x2f, 0x31, 0x39, 0x32, 0x2e, 0x31, 0x36, 0x38,
  0x2e, 0x33, 0x2e, 0x32, 0x3a, 0x36, 0x33, 0x36, 0x33,
  // Nexthop.CostDouble
  0x86, 0x08, 0x3f, 0xfa, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
};

static const uint8_t RoutingTableEntryWithoutNexthopsData[] = {
  // Header
  0x91, 0x09,
  // Destination Name
  0x07, 0x07, 0x08, 0x05, 0x64, 0x65, 0x73, 0x74, 0x31
};

BOOST_AUTO_TEST_CASE(EncodeWithNexthops)
{
  RoutingTableEntry rte(ndn::Name("dest1"));

  NextHop nexthops1;
  nexthops1.setConnectingFaceUri(NEXTHOP1);
  nexthops1.setRouteCost(1.65);
  rte.getNexthopList().addNextHop(nexthops1);

  NextHop nexthops2;
  nexthops2.setConnectingFaceUri(NEXTHOP2);
  nexthops2.setRouteCost(1.65);
  rte.getNexthopList().addNextHop(nexthops2);

  BOOST_TEST(rte.wireEncode() == RoutingTableEntryWithNexthopsData,
             boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(DecodeWithNexthops)
{
  RoutingTableEntry rte(ndn::Block{RoutingTableEntryWithNexthopsData});
  BOOST_CHECK_EQUAL(rte.getDestination(), "dest1");

  BOOST_CHECK(rte.getNexthopList().size() != 0);
  auto it = rte.getNexthopList().begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), NEXTHOP1);
  BOOST_CHECK_EQUAL(it->getRouteCost(), 1.65);

  it++;
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), NEXTHOP2);
  BOOST_CHECK_EQUAL(it->getRouteCost(), 1.65);
}

BOOST_AUTO_TEST_CASE(EncodeWithoutNexthops)
{
  RoutingTableEntry rte(ndn::Name("dest1"));
  BOOST_TEST(rte.wireEncode() == RoutingTableEntryWithoutNexthopsData,
             boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(DecodeWithoutNexthops)
{
  RoutingTableEntry rte(ndn::Block{RoutingTableEntryWithoutNexthopsData});
  BOOST_CHECK_EQUAL(rte.getDestination(), "dest1");
  BOOST_CHECK_EQUAL(rte.getNexthopList().size(), 0);
}

BOOST_AUTO_TEST_CASE(Clear)
{
  RoutingTableEntry rte(ndn::Name("dest1"));

  NextHop nexthops1;
  nexthops1.setConnectingFaceUri(NEXTHOP1);
  nexthops1.setRouteCost(99);
  rte.getNexthopList().addNextHop(nexthops1);

  BOOST_CHECK_EQUAL(rte.getNexthopList().size(), 1);

  auto it = rte.getNexthopList().begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), NEXTHOP1);
  BOOST_CHECK_EQUAL(it->getRouteCost(), 99);

  rte.getNexthopList().clear();
  BOOST_CHECK_EQUAL(rte.getNexthopList().size(), 0);

  NextHop nexthops2;
  nexthops2.setConnectingFaceUri(NEXTHOP2);
  nexthops2.setRouteCost(99);
  rte.getNexthopList().addNextHop(nexthops2);

  BOOST_CHECK_EQUAL(rte.getNexthopList().size(), 1);
  it =  rte.getNexthopList().begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), NEXTHOP2);
  BOOST_CHECK_EQUAL(it->getRouteCost(), 99);
}

BOOST_AUTO_TEST_CASE(OutputStream)
{
  RoutingTableEntry rte(ndn::Name("dest1"));
  rte.getNexthopList().addNextHop({NEXTHOP1, 99});

  std::ostringstream os;
  os << rte;

  BOOST_CHECK_EQUAL(os.str(),
                    "  Destination: /dest1\n"
                    "    NextHop(Uri: udp4://192.168.3.1:6363, Cost: 99)\n");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
