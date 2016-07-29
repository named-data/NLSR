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

#include "route/routing-table-calculator.hpp"

#include "adjacency-list.hpp"
#include "lsa.hpp"
#include "lsdb.hpp"
#include "nlsr.hpp"
#include "test-common.hpp"
#include "route/map.hpp"
#include "route/routing-table.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

static const ndn::time::system_clock::TimePoint MAX_TIME =
  ndn::time::system_clock::TimePoint::max();

class LinkStateCalculatorFixture : public BaseFixture
{
public:
  LinkStateCalculatorFixture()
    : face(make_shared<ndn::util::DummyClientFace>(g_ioService))
    , nlsr(g_ioService, g_scheduler, ndn::ref(*face))
    , routingTable(nlsr.getRoutingTable())
    , lsdb(nlsr.getLsdb())
  {
    setUpTopology();
  }

  // Triangle topology with routers A, B, C connected
  void setUpTopology()
  {
    INIT_LOGGERS("/tmp", "TRACE");

    ConfParameter& conf = nlsr.getConfParameter();
    conf.setNetwork("/ndn");
    conf.setSiteName("/router");
    conf.setRouterName("/a");
    conf.buildRouterPrefix();

    Adjacent a(ROUTER_A_NAME, ROUTER_A_FACE, 0, Adjacent::STATUS_ACTIVE, 0, 0);
    Adjacent b(ROUTER_B_NAME, ROUTER_B_FACE, 0, Adjacent::STATUS_ACTIVE, 0, 0);
    Adjacent c(ROUTER_C_NAME, ROUTER_C_FACE, 0, Adjacent::STATUS_ACTIVE, 0, 0);

    // Router A
    b.setLinkCost(LINK_AB_COST);
    c.setLinkCost(LINK_AC_COST);

    AdjacencyList& adjacencyListA = nlsr.getAdjacencyList();
    adjacencyListA.insert(b);
    adjacencyListA.insert(c);

    AdjLsa adjA(a.getName(), 1, MAX_TIME, 2, adjacencyListA);
    lsdb.installAdjLsa(adjA);

    // Router B
    a.setLinkCost(LINK_AB_COST);
    c.setLinkCost(LINK_BC_COST);

    AdjacencyList adjacencyListB;
    adjacencyListB.insert(a);
    adjacencyListB.insert(c);

    AdjLsa adjB(b.getName(), 1, MAX_TIME, 2, adjacencyListB);
    lsdb.installAdjLsa(adjB);

    // Router C
    a.setLinkCost(LINK_AC_COST);
    b.setLinkCost(LINK_BC_COST);

    AdjacencyList adjacencyListC;
    adjacencyListC.insert(a);
    adjacencyListC.insert(b);

    AdjLsa adjC(c.getName(), 1, MAX_TIME, 2, adjacencyListC);
    lsdb.installAdjLsa(adjC);

    map.createFromAdjLsdb(nlsr);
  }

public:
  shared_ptr<ndn::util::DummyClientFace> face;
  Nlsr nlsr;
  Map map;

  RoutingTable& routingTable;
  Lsdb& lsdb;

  static const ndn::Name ROUTER_A_NAME;
  static const ndn::Name ROUTER_B_NAME;
  static const ndn::Name ROUTER_C_NAME;

  static const std::string ROUTER_A_FACE;
  static const std::string ROUTER_B_FACE;
  static const std::string ROUTER_C_FACE;

  static const double LINK_AB_COST;
  static const double LINK_AC_COST;
  static const double LINK_BC_COST;
};

const ndn::Name LinkStateCalculatorFixture::ROUTER_A_NAME = "/ndn/router/a";
const ndn::Name LinkStateCalculatorFixture::ROUTER_B_NAME = "/ndn/router/b";
const ndn::Name LinkStateCalculatorFixture::ROUTER_C_NAME = "/ndn/router/c";

const std::string LinkStateCalculatorFixture::ROUTER_A_FACE = "face-a";
const std::string LinkStateCalculatorFixture::ROUTER_B_FACE = "face-b";
const std::string LinkStateCalculatorFixture::ROUTER_C_FACE = "face-c";

const double LinkStateCalculatorFixture::LINK_AB_COST = 5;
const double LinkStateCalculatorFixture::LINK_AC_COST = 10;
const double LinkStateCalculatorFixture::LINK_BC_COST = 17;

BOOST_FIXTURE_TEST_SUITE(TestLinkStateRoutingCalculator, LinkStateCalculatorFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  LinkStateRoutingTableCalculator calculator(map.getMapSize());
  calculator.calculatePath(map, routingTable, nlsr);

  RoutingTableEntry* entryB = routingTable.findRoutingTableEntry(ROUTER_B_NAME);
  BOOST_REQUIRE(entryB != nullptr);

  // Router A should be able to get to B through B and to B through C
  NexthopList& bHopList = entryB->getNexthopList();
  BOOST_REQUIRE_EQUAL(bHopList.getNextHops().size(), 2);

  for (const NextHop& hop : bHopList) {
    std::string faceUri = hop.getConnectingFaceUri();
    uint64_t cost = hop.getRouteCostAsAdjustedInteger();

    BOOST_CHECK((faceUri == ROUTER_B_FACE && cost == LINK_AB_COST) ||
                (faceUri == ROUTER_C_FACE && cost == LINK_AC_COST + LINK_BC_COST));

  }

  RoutingTableEntry* entryC = routingTable.findRoutingTableEntry(ROUTER_C_NAME);
  BOOST_REQUIRE(entryC != nullptr);

  // Router A should be able to get to C through C and to C through B
  NexthopList& cHopList = entryC->getNexthopList();
  BOOST_REQUIRE_EQUAL(cHopList.getNextHops().size(), 2);

  for (const NextHop& hop : cHopList) {
    std::string faceUri = hop.getConnectingFaceUri();
    uint64_t cost = hop.getRouteCostAsAdjustedInteger();

    BOOST_CHECK((faceUri == ROUTER_C_FACE && cost == LINK_AC_COST) ||
                (faceUri == ROUTER_B_FACE && cost == LINK_AB_COST + LINK_BC_COST));
  }
}

BOOST_AUTO_TEST_CASE(Asymmetric)
{
  // Asymmetric link cost between B and C
  ndn::Name key = ndn::Name(ROUTER_B_NAME).append(AdjLsa::TYPE_STRING);
  AdjLsa* lsa = nlsr.getLsdb().findAdjLsa(key);
  BOOST_REQUIRE(lsa != nullptr);

  Adjacent* c = lsa->getAdl().findAdjacent(ROUTER_C_NAME);
  BOOST_REQUIRE(c != nullptr);

  double higherLinkCost = LINK_BC_COST + 1;
  c->setLinkCost(higherLinkCost);

  // Calculation should consider the link between B and C as having cost = higherLinkCost
  LinkStateRoutingTableCalculator calculator(map.getMapSize());
  calculator.calculatePath(map, routingTable, nlsr);

  RoutingTableEntry* entryB = routingTable.findRoutingTableEntry(ROUTER_B_NAME);
  BOOST_REQUIRE(entryB != nullptr);

  // Router A should be able to get to B through B and to B through C
  NexthopList& bHopList = entryB->getNexthopList();
  BOOST_REQUIRE_EQUAL(bHopList.getNextHops().size(), 2);

  for (const NextHop& hop : bHopList) {
    std::string faceUri = hop.getConnectingFaceUri();
    uint64_t cost = hop.getRouteCostAsAdjustedInteger();

    BOOST_CHECK((faceUri == ROUTER_B_FACE && cost == LINK_AB_COST) ||
                (faceUri == ROUTER_C_FACE && cost == LINK_AC_COST + higherLinkCost));

  }

  RoutingTableEntry* entryC = routingTable.findRoutingTableEntry(ROUTER_C_NAME);
  BOOST_REQUIRE(entryC != nullptr);

  // Router A should be able to get to C through C and to C through B
  NexthopList& cHopList = entryC->getNexthopList();
  BOOST_REQUIRE_EQUAL(cHopList.getNextHops().size(), 2);

  for (const NextHop& hop : cHopList) {
    std::string faceUri = hop.getConnectingFaceUri();
    uint64_t cost = hop.getRouteCostAsAdjustedInteger();

    BOOST_CHECK((faceUri == ROUTER_C_FACE && cost == LINK_AC_COST) ||
                (faceUri == ROUTER_B_FACE && cost == LINK_AB_COST + higherLinkCost));
  }
}

BOOST_AUTO_TEST_CASE(AsymmetricZeroCost)
{
  // Asymmetric link cost between B and C
  ndn::Name key = ndn::Name(ROUTER_B_NAME).append(AdjLsa::TYPE_STRING);
  AdjLsa* lsa = nlsr.getLsdb().findAdjLsa(key);
  BOOST_REQUIRE(lsa != nullptr);

  Adjacent* c = lsa->getAdl().findAdjacent(ROUTER_C_NAME);
  BOOST_REQUIRE(c != nullptr);

  c->setLinkCost(0);

  // Calculation should consider the link between B and C as down
  LinkStateRoutingTableCalculator calculator(map.getMapSize());
  calculator.calculatePath(map, routingTable, nlsr);

  // Router A should be able to get to B through B but not through C
  RoutingTableEntry* entryB = routingTable.findRoutingTableEntry(ROUTER_B_NAME);
  BOOST_REQUIRE(entryB != nullptr);

  NexthopList& bHopList = entryB->getNexthopList();
  BOOST_REQUIRE_EQUAL(bHopList.getNextHops().size(), 1);

  NextHop& nextHopForB = bHopList.getNextHops().front();

  BOOST_CHECK(nextHopForB.getConnectingFaceUri() == ROUTER_B_FACE &&
              nextHopForB.getRouteCostAsAdjustedInteger() == LINK_AB_COST);

  // Router A should be able to get to C through C but not through B
  RoutingTableEntry* entryC = routingTable.findRoutingTableEntry(ROUTER_C_NAME);
  BOOST_REQUIRE(entryC != nullptr);

  NexthopList& cHopList = entryC->getNexthopList();
  BOOST_REQUIRE_EQUAL(cHopList.getNextHops().size(), 1);

  NextHop& nextHopForC = cHopList.getNextHops().front();

  BOOST_CHECK(nextHopForC.getConnectingFaceUri() == ROUTER_C_FACE &&
              nextHopForC.getRouteCostAsAdjustedInteger() == LINK_AC_COST);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
