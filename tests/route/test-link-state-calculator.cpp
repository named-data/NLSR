/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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
 */

#include "route/routing-table-calculator.hpp"

#include "adjacency-list.hpp"
#include "adjacent.hpp"
#include "lsdb.hpp"
#include "nlsr.hpp"
#include "route/map.hpp"
#include "route/routing-table.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

namespace nlsr {
namespace test {

constexpr ndn::time::system_clock::time_point MAX_TIME = ndn::time::system_clock::time_point::max();
static const ndn::Name ROUTER_A_NAME = "/ndn/site/%C1.Router/this-router";
static const ndn::Name ROUTER_B_NAME = "/ndn/site/%C1.Router/b";
static const ndn::Name ROUTER_C_NAME = "/ndn/site/%C1.Router/c";
static const ndn::FaceUri ROUTER_A_FACE("udp4://10.0.0.1");
static const ndn::FaceUri ROUTER_B_FACE("udp4://10.0.0.2");
static const ndn::FaceUri ROUTER_C_FACE("udp4://10.0.0.3");
constexpr double LINK_AB_COST = 5;
constexpr double LINK_AC_COST = 10;
constexpr double LINK_BC_COST = 17;

class LinkStateCalculatorFixture : public IoKeyChainFixture
{
public:
  LinkStateCalculatorFixture()
    : face(m_io, m_keyChain)
    , conf(face, m_keyChain)
    , confProcessor(conf)
    , nlsr(face, m_keyChain, conf)
    , routingTable(nlsr.m_routingTable)
    , lsdb(nlsr.m_lsdb)
  {
    setUpTopology();
  }

  // Triangle topology with routers A, B, C connected
  void setUpTopology()
  {
    Adjacent a(ROUTER_A_NAME, ROUTER_A_FACE, 0, Adjacent::STATUS_ACTIVE, 0, 0);
    Adjacent b(ROUTER_B_NAME, ROUTER_B_FACE, 0, Adjacent::STATUS_ACTIVE, 0, 0);
    Adjacent c(ROUTER_C_NAME, ROUTER_C_FACE, 0, Adjacent::STATUS_ACTIVE, 0, 0);

    // Router A
    b.setLinkCost(LINK_AB_COST);
    c.setLinkCost(LINK_AC_COST);

    AdjacencyList& adjacencyListA = conf.getAdjacencyList();
    adjacencyListA.insert(b);
    adjacencyListA.insert(c);

    AdjLsa adjA(a.getName(), 1, MAX_TIME, adjacencyListA);
    lsdb.installLsa(std::make_shared<AdjLsa>(adjA));

    // Router B
    a.setLinkCost(LINK_AB_COST);
    c.setLinkCost(LINK_BC_COST);

    AdjacencyList adjacencyListB;
    adjacencyListB.insert(a);
    adjacencyListB.insert(c);

    AdjLsa adjB(b.getName(), 1, MAX_TIME, adjacencyListB);
    lsdb.installLsa(std::make_shared<AdjLsa>(adjB));

    // Router C
    a.setLinkCost(LINK_AC_COST);
    b.setLinkCost(LINK_BC_COST);

    AdjacencyList adjacencyListC;
    adjacencyListC.insert(a);
    adjacencyListC.insert(b);

    AdjLsa adjC(c.getName(), 1, MAX_TIME, adjacencyListC);
    lsdb.installLsa(std::make_shared<AdjLsa>(adjC));

    auto lsaRange = lsdb.getLsdbIterator<AdjLsa>();
    map.createFromAdjLsdb(lsaRange.first, lsaRange.second);
  }

public:
  ndn::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  Nlsr nlsr;
  Map map;

  RoutingTable& routingTable;
  Lsdb& lsdb;
};

BOOST_FIXTURE_TEST_SUITE(TestLinkStateRoutingCalculator, LinkStateCalculatorFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  LinkStateRoutingTableCalculator calculator(map.getMapSize());
  calculator.calculatePath(map, routingTable, conf, lsdb);

  RoutingTableEntry* entryB = routingTable.findRoutingTableEntry(ROUTER_B_NAME);
  BOOST_REQUIRE(entryB != nullptr);

  // Router A should be able to get to B through B and to B through C
  NexthopList& bHopList = entryB->getNexthopList();
  BOOST_REQUIRE_EQUAL(bHopList.getNextHops().size(), 2);

  for (const NextHop& hop : bHopList) {
    auto faceUri = hop.getConnectingFaceUri();
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
    auto faceUri = hop.getConnectingFaceUri();
    uint64_t cost = hop.getRouteCostAsAdjustedInteger();

    BOOST_CHECK((faceUri == ROUTER_C_FACE && cost == LINK_AC_COST) ||
                (faceUri == ROUTER_B_FACE && cost == LINK_AB_COST + LINK_BC_COST));
  }
}

BOOST_AUTO_TEST_CASE(Asymmetric)
{
  // Asymmetric link cost between B and C
  auto lsa = nlsr.m_lsdb.findLsa<AdjLsa>(ndn::Name(ROUTER_B_NAME));
  BOOST_REQUIRE(lsa != nullptr);

  auto c = lsa->m_adl.findAdjacent(ROUTER_C_NAME);
  BOOST_REQUIRE(c != conf.getAdjacencyList().end());

  double higherLinkCost = LINK_BC_COST + 1;
  c->setLinkCost(higherLinkCost);

  // Calculation should consider the link between B and C as having cost = higherLinkCost
  LinkStateRoutingTableCalculator calculator(map.getMapSize());
  calculator.calculatePath(map, routingTable, conf, lsdb);

  RoutingTableEntry* entryB = routingTable.findRoutingTableEntry(ROUTER_B_NAME);
  BOOST_REQUIRE(entryB != nullptr);

  // Router A should be able to get to B through B and to B through C
  NexthopList& bHopList = entryB->getNexthopList();
  BOOST_REQUIRE_EQUAL(bHopList.getNextHops().size(), 2);

  for (const NextHop& hop : bHopList) {
    auto faceUri = hop.getConnectingFaceUri();
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
    auto faceUri = hop.getConnectingFaceUri();
    uint64_t cost = hop.getRouteCostAsAdjustedInteger();

    BOOST_CHECK((faceUri == ROUTER_C_FACE && cost == LINK_AC_COST) ||
                (faceUri == ROUTER_B_FACE && cost == LINK_AB_COST + higherLinkCost));
  }
}

BOOST_AUTO_TEST_CASE(NonAdjacentCost)
{
  // Asymmetric link cost between B and C
  auto lsa = nlsr.m_lsdb.findLsa<AdjLsa>(ROUTER_B_NAME);
  BOOST_REQUIRE(lsa != nullptr);

  auto c = lsa->m_adl.findAdjacent(ROUTER_C_NAME);
  BOOST_REQUIRE(c != conf.getAdjacencyList().end());

  // Break the link between B - C by setting it to a NON_ADJACENT_COST.
  c->setLinkCost(Adjacent::NON_ADJACENT_COST);

  // Calculation should consider the link between B and C as down
  LinkStateRoutingTableCalculator calculator(map.getMapSize());
  calculator.calculatePath(map, routingTable, conf, lsdb);

  // Router A should be able to get to B through B but not through C
  RoutingTableEntry* entryB = routingTable.findRoutingTableEntry(ROUTER_B_NAME);
  BOOST_REQUIRE(entryB != nullptr);

  auto bHopList = entryB->getNexthopList();
  BOOST_REQUIRE_EQUAL(bHopList.getNextHops().size(), 1);

  const auto nextHopForB = bHopList.getNextHops().begin();

  BOOST_CHECK(nextHopForB->getConnectingFaceUri() == ROUTER_B_FACE &&
              nextHopForB->getRouteCostAsAdjustedInteger() == LINK_AB_COST);

  // Router A should be able to get to C through C but not through B
  auto entryC = routingTable.findRoutingTableEntry(ROUTER_C_NAME);
  BOOST_REQUIRE(entryC != nullptr);

  NexthopList& cHopList = entryC->getNexthopList();
  BOOST_REQUIRE_EQUAL(cHopList.getNextHops().size(), 1);

  const auto nextHopForC = cHopList.getNextHops().begin();

  BOOST_CHECK(nextHopForC->getConnectingFaceUri() == ROUTER_C_FACE &&
              nextHopForC->getRouteCostAsAdjustedInteger() == LINK_AC_COST);
}

BOOST_AUTO_TEST_CASE(AsymmetricZeroCostLink)
{
  // Asymmetric and zero link cost between B - C, and B - A.
  auto lsaB = nlsr.m_lsdb.findLsa<AdjLsa>(ROUTER_B_NAME);
  BOOST_REQUIRE(lsaB != nullptr);

  auto c = lsaB->m_adl.findAdjacent(ROUTER_C_NAME);
  BOOST_REQUIRE(c != conf.getAdjacencyList().end());
  // Re-adjust link cost to 0 from B-C. However, this should not set B-C cost 0 because C-B
  // cost is greater that 0 i.e. 17
  c->setLinkCost(0);

  auto a = lsaB->m_adl.findAdjacent(ROUTER_A_NAME);
  BOOST_REQUIRE(a != conf.getAdjacencyList().end());

  auto lsaA = nlsr.m_lsdb.findLsa<AdjLsa>(ROUTER_A_NAME);
  BOOST_REQUIRE(lsaA != nullptr);

  auto b = lsaA->m_adl.findAdjacent(ROUTER_B_NAME);
  BOOST_REQUIRE(b != conf.getAdjacencyList().end());

  // Re-adjust link cost to 0 from both the direction i.e B-A and A-B
  a->setLinkCost(0);
  b->setLinkCost(0);

  // Calculation should consider 0 link-cost between B and C
  LinkStateRoutingTableCalculator calculator(map.getMapSize());
  calculator.calculatePath(map, routingTable, conf, lsdb);

  // Router A should be able to get to B through B and C
  RoutingTableEntry* entryB = routingTable.findRoutingTableEntry(ROUTER_B_NAME);
  BOOST_REQUIRE(entryB != nullptr);

  // Node can have neighbors with zero cost, so the nexthop count should be 2
  NexthopList& bHopList = entryB->getNexthopList();
  BOOST_REQUIRE_EQUAL(bHopList.getNextHops().size(), 2);

  const auto nextHopForB = bHopList.getNextHops().begin();
  // Check if the next hop via B is through A or not after the cost adjustment
  BOOST_CHECK(nextHopForB->getConnectingFaceUri() == ROUTER_B_FACE &&
              nextHopForB->getRouteCostAsAdjustedInteger() == 0);

  // Router A should be able to get to C through C and B
  auto entryC = routingTable.findRoutingTableEntry(ROUTER_C_NAME);
  BOOST_REQUIRE(entryC != nullptr);

  NexthopList& cHopList = entryC->getNexthopList();
  BOOST_REQUIRE_EQUAL(cHopList.getNextHops().size(), 2);

  const auto nextHopForC = cHopList.getNextHops().begin();
  // Check if the nextHop from C is via A or not
  BOOST_CHECK(nextHopForC->getConnectingFaceUri() == ROUTER_C_FACE &&
              nextHopForC->getRouteCostAsAdjustedInteger() == LINK_AC_COST);

}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
