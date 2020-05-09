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

#include "route/routing-table.hpp"
#include "nlsr.hpp"
#include "route/routing-table-entry.hpp"
#include "route/nexthop.hpp"

#include "tests/test-common.hpp"

namespace nlsr {
namespace test {

class RoutingTableFixture
{
public:
  RoutingTableFixture()
    : conf(face, keyChain)
    , nlsr(face, keyChain, conf)
    , rt(nlsr.m_routingTable)
  {
  }

public:
  ndn::util::DummyClientFace face;
  ndn::KeyChain keyChain;
  ConfParameter conf;
  Nlsr nlsr;

  RoutingTable& rt;
};

BOOST_AUTO_TEST_SUITE(TestRoutingTable)

BOOST_FIXTURE_TEST_CASE(RoutingTableAddNextHop, RoutingTableFixture)
{
  NextHop nh1;
  const std::string DEST_ROUTER = "destRouter";
  rt.addNextHop(DEST_ROUTER, nh1);

  BOOST_CHECK_EQUAL(rt.findRoutingTableEntry(DEST_ROUTER)->getDestination(), DEST_ROUTER);
}

const uint8_t RoutingTableData1[] =
{
  // Header
  0x90, 0x20,
  // Routing table entry
  0x91, 0x1e,
  // Destination
  0x07, 0x07, 0x08, 0x05, 0x64, 0x65, 0x73, 0x74, 0x31, 0x8f, 0x13,
  // Nexthop
  0x8d, 0x07, 0x6e, 0x65, 0x78, 0x74, 0x68, 0x6f, 0x70, 0x86, 0x08, 0x3f, 0xfa, 0x66,
  0x66, 0x66, 0x66, 0x66, 0x66
};

const uint8_t RoutingTableData2[] =
{
  // Header
  0x90, 0x00
};

BOOST_FIXTURE_TEST_CASE(RoutingTableEncode1, RoutingTableFixture)
{
  NextHop nexthops;
  nexthops.setConnectingFaceUri("nexthop");
  nexthops.setRouteCost(1.65);
  rt.addNextHop("dest1", nexthops);

  auto wire = rt.wireEncode();
  BOOST_REQUIRE_EQUAL_COLLECTIONS(RoutingTableData1,
                                  RoutingTableData1 + sizeof(RoutingTableData1),
                                  wire.begin(), wire.end());
}

BOOST_FIXTURE_TEST_CASE(RoutingTableEncode2, RoutingTableFixture)
{
  auto wire = rt.wireEncode();
  BOOST_REQUIRE_EQUAL_COLLECTIONS(RoutingTableData2,
                                  RoutingTableData2 + sizeof(RoutingTableData2),
                                  wire.begin(), wire.end());
}

BOOST_FIXTURE_TEST_CASE(RoutingTableDecode1, RoutingTableFixture)
{
  RoutingTableStatus rtStatus(ndn::Block(RoutingTableData1, sizeof(RoutingTableData1)));

  auto it1 = rtStatus.m_rTable.begin();

  ndn::Name des1 = it1->getDestination();
  BOOST_CHECK_EQUAL(des1, "dest1");

  auto it2 = it1->getNexthopList().begin();
  BOOST_CHECK_EQUAL(it2->getConnectingFaceUri(), "nexthop");
  BOOST_CHECK_EQUAL(it2->getRouteCost(), 1.65);

  BOOST_CHECK_EQUAL(rtStatus.m_rTable.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(RoutingTableOutputStream, RoutingTableFixture)
{
  NextHop nexthops;
  nexthops.setConnectingFaceUri("nexthop");
  nexthops.setRouteCost(99);
  rt.addNextHop("dest1", nexthops);

  std::ostringstream os;
  os << rt;

  BOOST_CHECK_EQUAL(os.str(),
                    "Routing Table:\n"
                    "  Destination: /dest1\n"
                    "    NextHop(Uri: nexthop, Cost: 99)\n");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
