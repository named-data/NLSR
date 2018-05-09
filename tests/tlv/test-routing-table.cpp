/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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

#include "tlv/routing-table-status.hpp"

#include "../boost-test.hpp"

namespace nlsr {
namespace tlv {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestRoutingTable)

const uint8_t RoutingTableData1[] =
{
  // Header
  0x90, 0x24,
  // Routing table entry
  0x91, 0x22,
    // Destination
    0x8e, 0x09, 0x07, 0x07, 0x08, 0x05, 0x64, 0x65, 0x73, 0x74, 0x31,
    // Nexthop
    0x8f, 0x15, 0x8d, 0x07, 0x6e, 0x65, 0x78, 0x74, 0x68, 0x6f, 0x70,
    0x86, 0x0a, 0x86, 0x08, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xfa, 0x3f
};

const uint8_t RoutingTableData2[] =
{
  // Header
  0x90, 0x00
};

BOOST_AUTO_TEST_CASE(RoutingTableEncode1)
{
  RoutingTableStatus rtStatus;

  Destination des;
  des.setName("dest1");

  // RoutingtableEntry
  RoutingTable rt;
  rt.setDestination(des);

  NextHop nexthops;
  nexthops.setUri("nexthop");
  nexthops.setCost(1.65);
  rt.addNexthops(nexthops);

  rtStatus.addRoutingTable(rt);

  const ndn::Block& wire = rtStatus.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(RoutingTableData1,
                                  RoutingTableData1 + sizeof(RoutingTableData1),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(RoutingTableEncode2)
{
  RoutingTableStatus rtStatus;

  const ndn::Block& wire = rtStatus.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(RoutingTableData2,
                                  RoutingTableData2 + sizeof(RoutingTableData2),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(RoutingTableDecode1)
{
  RoutingTableStatus rtStatus;

  rtStatus.wireDecode(ndn::Block(RoutingTableData1, sizeof(RoutingTableData1)));

  std::list<RoutingTable> rte = rtStatus.getRoutingtable();
  std::list<RoutingTable>::const_iterator it1 = rte.begin();

  Destination des1 = it1->getDestination();
  BOOST_CHECK_EQUAL(des1.getName(), "dest1");

  std::list<NextHop> nexthops = it1->getNextHops();
  std::list<NextHop>::const_iterator it2 = nexthops.begin();
  BOOST_CHECK_EQUAL(it2->getUri(), "nexthop");
  BOOST_CHECK_EQUAL(it2->getCost(), 1.65);

  BOOST_CHECK_EQUAL(rtStatus.hasRoutingTable(), true);
}

BOOST_AUTO_TEST_CASE(RoutingTableDecode2)
{
  RoutingTableStatus rtStatus;

  rtStatus.wireDecode(ndn::Block(RoutingTableData2, sizeof(RoutingTableData2)));

  BOOST_CHECK_EQUAL(rtStatus.hasRoutingTable(), false);
}

BOOST_AUTO_TEST_CASE(RoutingTableClear)
{
  RoutingTableStatus rtStatus;
  Destination des;
  des.setName("dest1");

  // RoutingtableEntry
  RoutingTable rt;
  rt.setDestination(des);

  NextHop nexthops;
  nexthops.setUri("nexthop");
  nexthops.setCost(1.65);
  rt.addNexthops(nexthops);

  rtStatus.addRoutingTable(rt);

  BOOST_CHECK_EQUAL(rtStatus.hasRoutingTable(), true);
  rtStatus.clearRoutingTable();
  BOOST_CHECK_EQUAL(rtStatus.hasRoutingTable(), false);
}

BOOST_AUTO_TEST_CASE(RoutingTableOutputStream)
{
  RoutingTableStatus rtStatus;
  Destination des;
  des.setName("dest1");

  // RoutingtableEntry
  RoutingTable rt;
  rt.setDestination(des);

  NextHop nexthops;
  nexthops.setUri("nexthop");
  nexthops.setCost(99);
  rt.addNexthops(nexthops);

  rtStatus.addRoutingTable(rt);

  std::ostringstream os;
  os << rtStatus;

  BOOST_CHECK_EQUAL(os.str(), "Routing Table Status: \n"
                                  "Destination: /dest1\n"
                                  "NexthopList(\n"
                                  "NextHop(Uri: nexthop, Cost: 99)\n"
                                  ")");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
