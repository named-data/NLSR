/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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

class RoutingTableFixture : public UnitTestTimeFixture
{
public:
  RoutingTableFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , conf(face, m_keyChain)
    , confProcessor(conf)
    , lsdb(face, m_keyChain, conf)
    , rt(m_scheduler, lsdb, conf)
  {
  }

public:
  ndn::util::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;

  Lsdb lsdb;
  RoutingTable rt;
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

BOOST_FIXTURE_TEST_CASE(UpdateFromLsdb, RoutingTableFixture)
{
  ndn::time::system_clock::TimePoint testTimePoint = ndn::time::system_clock::now() + 3600_s;
  ndn::Name router2("/router2");
  AdjLsa adjLsa(router2, 12, testTimePoint, 2, conf.getAdjacencyList());
  std::shared_ptr<Lsa> lsaPtr = std::make_shared<AdjLsa>(adjLsa);
  BOOST_CHECK(!rt.m_isRouteCalculationScheduled);
  lsdb.installLsa(lsaPtr);
  BOOST_CHECK(rt.m_isRouteCalculationScheduled);

  // After 15_s (by default) routing table calculation is done
  advanceClocks(15_s);
  BOOST_CHECK(!rt.m_isRouteCalculationScheduled);

  // Update to installed LSA
  std::shared_ptr<Lsa> lsaPtr2 = std::make_shared<AdjLsa>(adjLsa);
  auto adjPtr = std::static_pointer_cast<AdjLsa>(lsaPtr2);
  adjPtr->addAdjacent(Adjacent("router3"));
  adjPtr->setSeqNo(13);
  lsdb.installLsa(lsaPtr2);
  BOOST_CHECK(rt.m_isRouteCalculationScheduled);

  // Insert a neighbor so that AdjLsa can be installed
  AdjacencyList adjl;
  Adjacent ownAdj(conf.getRouterPrefix());
  ownAdj.setStatus(Adjacent::STATUS_ACTIVE);
  adjl.insert(ownAdj);
  AdjLsa adjLsa4("/router4", 12, testTimePoint, 2, adjl);
  lsaPtr = std::make_shared<AdjLsa>(adjLsa4);
  lsdb.installLsa(lsaPtr);

  Adjacent adj("/router4");
  adj.setStatus(Adjacent::STATUS_ACTIVE);
  conf.getAdjacencyList().insert(adj);
  lsdb.scheduleAdjLsaBuild();
  BOOST_CHECK_EQUAL(rt.m_rTable.size(), 0);
  advanceClocks(15_s);
  BOOST_CHECK_EQUAL(rt.m_rTable.size(), 1);

  rt.wireEncode();
  BOOST_CHECK(rt.m_wire.isValid());
  BOOST_CHECK_GT(rt.m_wire.size(), 0);

  // Remove own Adj Lsa - Make sure routing table is wiped out
  conf.getAdjacencyList().setStatusOfNeighbor("/router4", Adjacent::STATUS_INACTIVE);
  conf.getAdjacencyList().setTimedOutInterestCount("/router4", HELLO_RETRIES_MAX);
  lsdb.scheduleAdjLsaBuild();
  advanceClocks(15_s);
  BOOST_CHECK_EQUAL(rt.m_rTable.size(), 0);
  BOOST_CHECK(!rt.m_wire.isValid());

  // Check that HR routing is scheduled, once Coordinate LSA is added
  BOOST_CHECK(!rt.m_isRouteCalculationScheduled);
  rt.m_hyperbolicState = HYPERBOLIC_STATE_ON;
  CoordinateLsa clsa("router5", 12, testTimePoint, 2.5, {30.0});
  auto clsaPtr = std::make_shared<CoordinateLsa>(clsa);
  lsdb.installLsa(clsaPtr);
  BOOST_CHECK(rt.m_isRouteCalculationScheduled);

  Adjacent router5("/router5");
  router5.setStatus(Adjacent::STATUS_ACTIVE);
  conf.getAdjacencyList().insert(router5);
  conf.getAdjacencyList().setStatusOfNeighbor("/router5", Adjacent::STATUS_ACTIVE);
  advanceClocks(15_s);
  rt.wireEncode();
  BOOST_CHECK(rt.m_wire.isValid());
  BOOST_CHECK_GT(rt.m_wire.size(), 0);
  BOOST_CHECK(!rt.m_isRouteCalculationScheduled);

  // Emulate HelloProtocol neighbor down
  conf.getAdjacencyList().setStatusOfNeighbor("/router5", Adjacent::STATUS_INACTIVE);
  rt.scheduleRoutingTableCalculation();
  advanceClocks(15_s);
  BOOST_CHECK_EQUAL(rt.m_rTable.size(), 0);
  BOOST_CHECK(!rt.m_wire.isValid());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
