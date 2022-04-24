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

#include "route/routing-table-entry.hpp"
#include "tests/boost-test.hpp"

namespace nlsr {
namespace test {

BOOST_AUTO_TEST_SUITE(TestRoutingTableEntry)

BOOST_AUTO_TEST_CASE(RoutingTableEntryDestination)
{
  RoutingTableEntry rte1("router1");

  BOOST_CHECK_EQUAL(rte1.getDestination(), "router1");
}

const uint8_t RoutingTableEntryWithNexthopsData[] =
{
  // Header
  0x91, 0x35,
  // Destination Name
  0x07, 0x07, 0x08, 0x05, 0x64, 0x65, 0x73, 0x74, 0x31, 0x8f, 0x14,
  // Nexthop
  0x8d, 0x08, 0x6e, 0x65, 0x78, 0x74, 0x68, 0x6f, 0x70, 0x31, 0x86, 0x08, 0x3f, 0xfa,
  0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x8f, 0x14, 0x8d, 0x08,
  // Nexthop
  0x6e, 0x65, 0x78, 0x74, 0x68, 0x6f, 0x70, 0x32, 0x86, 0x08, 0x3f, 0xfa, 0x66, 0x66,
  0x66, 0x66, 0x66, 0x66
};

const uint8_t RoutingTableEntryWithoutNexthopsData[] =
{
  // Header
  0x91, 0x09,
  // Destination Name
  0x07, 0x07, 0x08, 0x05, 0x64, 0x65, 0x73, 0x74, 0x31
};

BOOST_AUTO_TEST_CASE(RoutingTableEntryEncodeWithNexthops)
{
  RoutingTableEntry rte(ndn::Name("dest1"));

  NextHop nexthops1;
  nexthops1.setConnectingFaceUri("nexthop1");
  nexthops1.setRouteCost(1.65);
  rte.getNexthopList().addNextHop(nexthops1);

  NextHop nexthops2;
  nexthops2.setConnectingFaceUri("nexthop2");
  nexthops2.setRouteCost(1.65);
  rte.getNexthopList().addNextHop(nexthops2);

  const ndn::Block& wire = rte.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(RoutingTableEntryWithNexthopsData,
                                  RoutingTableEntryWithNexthopsData +
                                    sizeof(RoutingTableEntryWithNexthopsData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(RoutingTableEntryDecodeWithNexthops)
{
  RoutingTableEntry rte(ndn::Block(RoutingTableEntryWithNexthopsData,
                                   sizeof(RoutingTableEntryWithNexthopsData)));
  BOOST_CHECK_EQUAL(rte.getDestination(), "dest1");

  BOOST_CHECK(rte.getNexthopList().size() != 0);
  auto it = rte.getNexthopList().begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), "nexthop1");
  BOOST_CHECK_EQUAL(it->getRouteCost(), 1.65);

  it++;
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), "nexthop2");
  BOOST_CHECK_EQUAL(it->getRouteCost(), 1.65);
}

BOOST_AUTO_TEST_CASE(RoutingTableEntryEncodeWithoutNexthops)
{
  RoutingTableEntry rte(ndn::Name("dest1"));

  auto wire = rte.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(RoutingTableEntryWithoutNexthopsData,
                                  RoutingTableEntryWithoutNexthopsData +
                                    sizeof(RoutingTableEntryWithoutNexthopsData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(RoutingTableEntryDecodeWithoutNexthops)
{
  RoutingTableEntry rte(ndn::Block(RoutingTableEntryWithoutNexthopsData,
                                   sizeof(RoutingTableEntryWithoutNexthopsData)));

  BOOST_CHECK_EQUAL(rte.getDestination(), "dest1");
  BOOST_CHECK(rte.getNexthopList().size() == 0);
}


BOOST_AUTO_TEST_CASE(RoutingTableEntryClear)
{
  RoutingTableEntry rte(ndn::Name("dest1"));

  NextHop nexthops1;
  nexthops1.setConnectingFaceUri("nexthop1");
  nexthops1.setRouteCost(99);
  rte.getNexthopList().addNextHop(nexthops1);

  BOOST_CHECK_EQUAL(rte.getNexthopList().size(), 1);

  auto it = rte.getNexthopList().begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), "nexthop1");
  BOOST_CHECK_EQUAL(it->getRouteCost(), 99);

  rte.getNexthopList().clear();
  BOOST_CHECK_EQUAL(rte.getNexthopList().size(), 0);

  NextHop nexthops2;
  nexthops2.setConnectingFaceUri("nexthop2");
  nexthops2.setRouteCost(99);
  rte.getNexthopList().addNextHop(nexthops2);

  BOOST_CHECK_EQUAL(rte.getNexthopList().size(), 1);
  it =  rte.getNexthopList().begin();
  BOOST_CHECK_EQUAL(it->getConnectingFaceUri(), "nexthop2");
  BOOST_CHECK_EQUAL(it->getRouteCost(), 99);
}

BOOST_AUTO_TEST_CASE(RoutingTableEntryOutputStream)
{
  RoutingTableEntry rte(ndn::Name("dest1"));

  NextHop nexthops1;
  nexthops1.setConnectingFaceUri("nexthop1");
  nexthops1.setRouteCost(99);
  rte.getNexthopList().addNextHop(nexthops1);

  std::ostringstream os;
  os << rte;

  BOOST_CHECK_EQUAL(os.str(),
                    "  Destination: /dest1\n"
                    "    NextHop(Uri: nexthop1, Cost: 99)\n");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
