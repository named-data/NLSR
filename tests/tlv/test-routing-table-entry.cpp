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

#include "tlv/routing-table-entry.hpp"

#include "../boost-test.hpp"

namespace nlsr {
namespace tlv {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestRoutingTableEntry)

const uint8_t RoutingTableEntryWithNexthopsData[] =
{
  // Header
  0x91, 0x3b,
  // Destination
  0x8e, 0x09, 0x07, 0x07, 0x08, 0x05, 0x64, 0x65, 0x73, 0x74, 0x31,
  // Nexthop
  0x8f, 0x16, 0x8d, 0x08, 0x6e, 0x65, 0x78, 0x74, 0x68, 0x6f, 0x70,
  0x31, 0x86, 0x0a, 0x86, 0x08, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xfa, 0x3f,
  // Nexthop
  0x8f, 0x16, 0x8d, 0x08, 0x6e, 0x65, 0x78, 0x74, 0x68, 0x6f, 0x70,
  0x32, 0x86, 0x0a, 0x86, 0x08, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xfa, 0x3f
};

const uint8_t RoutingTableEntryWithoutNexthopsData[] =
{
  // Header
  0x91, 0x0b,
  // Destination
  0x8e, 0x09, 0x07, 0x07, 0x08, 0x05, 0x64, 0x65, 0x73, 0x74, 0x31
};

BOOST_AUTO_TEST_CASE(RoutingTableEntryEncodeWithNexthops)
{
  RoutingTable rt;

  Destination des1;
  des1.setName("dest1");
  rt.setDestination(des1);

  NextHop nexthops1;
  nexthops1.setUri("nexthop1");
  nexthops1.setCost(1.65);
  rt.addNexthops(nexthops1);

  NextHop nexthops2;
  nexthops2.setUri("nexthop2");
  nexthops2.setCost(1.65);
  rt.addNexthops(nexthops2);

  const ndn::Block& wire = rt.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(RoutingTableEntryWithNexthopsData,
                                  RoutingTableEntryWithNexthopsData +
                                    sizeof(RoutingTableEntryWithNexthopsData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(RoutingTableEntryDecodeWithNexthops)
{
  RoutingTable rt;

  rt.wireDecode(ndn::Block(RoutingTableEntryWithNexthopsData,
                                      sizeof(RoutingTableEntryWithNexthopsData)));

  Destination des = rt.getDestination();
  BOOST_CHECK_EQUAL(des.getName(), "dest1");

  BOOST_CHECK_EQUAL(rt.hasNexthops(), true);
  std::list<NextHop> nexthops = rt.getNextHops();
  std::list<NextHop>::const_iterator it = nexthops.begin();
  BOOST_CHECK_EQUAL(it->getUri(), "nexthop1");
  BOOST_CHECK_EQUAL(it->getCost(), 1.65);

  it++;
  BOOST_CHECK_EQUAL(it->getUri(), "nexthop2");
  BOOST_CHECK_EQUAL(it->getCost(), 1.65);
}

BOOST_AUTO_TEST_CASE(RoutingTableEntryEncodeWithoutNexthops)
{
  RoutingTable rt;

  Destination des1;
  des1.setName("dest1");
  rt.setDestination(des1);

  const ndn::Block& wire = rt.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(RoutingTableEntryWithoutNexthopsData,
                                  RoutingTableEntryWithoutNexthopsData +
                                    sizeof(RoutingTableEntryWithoutNexthopsData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(RoutingTableEntryDecodeWithoutNexthops)
{
  RoutingTable rt;

  rt.wireDecode(ndn::Block(RoutingTableEntryWithoutNexthopsData,
                           sizeof(RoutingTableEntryWithoutNexthopsData)));

  Destination des = rt.getDestination();
  BOOST_CHECK_EQUAL(des.getName(), "dest1");

  BOOST_CHECK_EQUAL(rt.hasNexthops(), false);
}


BOOST_AUTO_TEST_CASE(RoutingTableEntryClear)
{
  RoutingTable rt;
  Destination des1;
  des1.setName("dest1");
  rt.setDestination(des1);

  NextHop nexthops1;
  nexthops1.setUri("nexthop1");
  nexthops1.setCost(99);
  rt.addNexthops(nexthops1);

  BOOST_CHECK_EQUAL(rt.getNextHops().size(), 1);

  std::list<NextHop> nexthops = rt.getNextHops();
  std::list<NextHop>::const_iterator it = nexthops.begin();
  BOOST_CHECK_EQUAL(it->getUri(), "nexthop1");
  BOOST_CHECK_EQUAL(it->getCost(), 99);

  rt.clearNexthops();
  BOOST_CHECK_EQUAL(rt.getNextHops().size(), 0);

  NextHop nexthops2;
  nexthops2.setUri("nexthop2");
  nexthops2.setCost(99);
  rt.addNexthops(nexthops2);

  BOOST_CHECK_EQUAL(rt.getNextHops().size(), 1);

  nexthops = rt.getNextHops();
  it = nexthops.begin();
  BOOST_CHECK_EQUAL(it->getUri(), "nexthop2");
  BOOST_CHECK_EQUAL(it->getCost(), 99);
}

BOOST_AUTO_TEST_CASE(RoutingTableEntryOutputStream)
{
  RoutingTable rt;
  Destination des1;
  des1.setName("dest1");
  rt.setDestination(des1);

  NextHop nexthops1;
  nexthops1.setUri("nexthop1");
  nexthops1.setCost(99);
  rt.addNexthops(nexthops1);

  std::ostringstream os;
  os << rt;

  BOOST_CHECK_EQUAL(os.str(),
                    "Destination: /dest1\n"
                    "NexthopList(\n"
                    "NextHop(Uri: nexthop1, Cost: 99)\n"
                    ")");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
