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

#include "route/routing-calculator.hpp"

#include "adjacency-list.hpp"
#include "adjacent.hpp"
#include "lsdb.hpp"
#include "nlsr.hpp"
#include "route/name-map.hpp"
#include "route/routing-table.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

namespace nlsr::tests {

constexpr time::system_clock::time_point MAX_TIME = time::system_clock::time_point::max();
static const ndn::Name ROUTER_A_NAME = "/ndn/site/%C1.Router/this-router";
static const ndn::Name ROUTER_B_NAME = "/ndn/site/%C1.Router/b";
static const ndn::Name ROUTER_C_NAME = "/ndn/site/%C1.Router/c";
static const ndn::FaceUri ROUTER_A_FACE("udp4://10.0.0.1:6363");
static const ndn::FaceUri ROUTER_B_FACE("udp4://10.0.0.2:6363");
static const ndn::FaceUri ROUTER_C_FACE("udp4://10.0.0.3:6363");
constexpr double LINK_AB_COST = 5.0;
constexpr double LINK_AC_COST = 10.0;
constexpr double LINK_BC_COST = 17.0;

/**
 * @brief Provide a topology for link-state routing calculator testing.
 *
 * The topology consists of three routers: A, B, C.
 * After calling all three setupRouter functions, they will form a triangle topology:
 *
 *   A-----B
 *    \   /
 *     \ /
 *      C
 *
 * The local router, as reported by `conf.getRouterPrefix()`, is router A.
 */
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
  }

  /**
   * @brief Insert Adjacency LSA of router A into LSDB.
   */
  void
  setupRouterA(double costAB = LINK_AB_COST, double costAC = LINK_AC_COST)
  {
    AdjacencyList& adjList = conf.getAdjacencyList();
    if (!std::isnan(costAB)) {
      adjList.insert(Adjacent(ROUTER_B_NAME, ROUTER_B_FACE, costAB, Adjacent::STATUS_ACTIVE, 0, 0));
    }
    if (!std::isnan(costAC)) {
      adjList.insert(Adjacent(ROUTER_C_NAME, ROUTER_C_FACE, costAC, Adjacent::STATUS_ACTIVE, 0, 0));
    }

    lsdb.installLsa(std::make_shared<AdjLsa>(ROUTER_A_NAME, 1, MAX_TIME, adjList));
  }

  /**
   * @brief Insert Adjacency LSA of router B into LSDB.
   */
  void
  setupRouterB(double costBC = LINK_BC_COST, double costBA = LINK_AB_COST)
  {
    AdjacencyList adjList;
    if (!std::isnan(costBC)) {
      adjList.insert(Adjacent(ROUTER_C_NAME, ROUTER_C_FACE, costBC, Adjacent::STATUS_ACTIVE, 0, 0));
    }
    if (!std::isnan(costBA)) {
      adjList.insert(Adjacent(ROUTER_A_NAME, ROUTER_A_FACE, costBA, Adjacent::STATUS_ACTIVE, 0, 0));
    }

    lsdb.installLsa(std::make_shared<AdjLsa>(ROUTER_B_NAME, 1, MAX_TIME, adjList));
  }

  /**
   * @brief Insert Adjacency LSA of router C into LSDB.
   */
  void
  setupRouterC(double costCA = LINK_AC_COST, double costCB = LINK_BC_COST)
  {
    AdjacencyList adjList;
    if (!std::isnan(costCA)) {
      adjList.insert(Adjacent(ROUTER_A_NAME, ROUTER_A_FACE, costCA, Adjacent::STATUS_ACTIVE, 0, 0));
    }
    if (!std::isnan(costCB)) {
      adjList.insert(Adjacent(ROUTER_B_NAME, ROUTER_B_FACE, costCB, Adjacent::STATUS_ACTIVE, 0, 0));
    }

    lsdb.installLsa(std::make_shared<AdjLsa>(ROUTER_C_NAME, 1, MAX_TIME, adjList));
  }

  /**
   * @brief Run link-state routing calculator.
   */
  void
  calculatePath()
  {
    auto lsaRange = lsdb.getLsdbIterator<AdjLsa>();
    NameMap map = NameMap::createFromAdjLsdb(lsaRange.first, lsaRange.second);
    calculateLinkStateRoutingPath(map, routingTable, conf, lsdb);
  }

  /**
   * @brief Verify that the routing table contains an entry with specific next hops.
   * @param destination Destination router.
   * @param expectedNextHops Expected next hops; order does not matter.
   */
  void
  checkRoutingTableEntry(const ndn::Name& destination,
                         std::initializer_list<NextHop> expectedNextHops) const
  {
    BOOST_TEST_CONTEXT("Checking routing table entry " << destination)
    {
      using NextHopSet = std::set<NextHop, NextHopUriSortedComparator>;

      NextHopSet expectedNextHopSet(expectedNextHops);

      const RoutingTableEntry* entry = routingTable.findRoutingTableEntry(destination);
      BOOST_REQUIRE(entry != nullptr);

      const NexthopList& actualNextHopList = entry->getNexthopList();
      NextHopSet actualNextHopSet(actualNextHopList.begin(), actualNextHopList.end());

      BOOST_TEST(expectedNextHopSet == actualNextHopSet, boost::test_tools::per_element());
    }
  }

public:
  ndn::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  Nlsr nlsr;

  RoutingTable& routingTable;
  Lsdb& lsdb;
};

BOOST_FIXTURE_TEST_SUITE(TestRoutingCalculatorLinkState, LinkStateCalculatorFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  setupRouterA();
  setupRouterB();
  setupRouterC();
  calculatePath();

  // Router A should be able to get to B through B and to B through C
  checkRoutingTableEntry(ROUTER_B_NAME, {
    {ROUTER_B_FACE, LINK_AB_COST},
    {ROUTER_C_FACE, LINK_AC_COST + LINK_BC_COST},
  });

  // Router A should be able to get to C through C and to C through B
  checkRoutingTableEntry(ROUTER_C_NAME, {
    {ROUTER_C_FACE, LINK_AC_COST},
    {ROUTER_B_FACE, LINK_AB_COST + LINK_BC_COST},
  });
}

BOOST_AUTO_TEST_CASE(Asymmetric)
{
  // Asymmetric link cost between B and C
  double higherCostBC = LINK_BC_COST + 1;
  setupRouterA();
  setupRouterB(higherCostBC);
  setupRouterC();

  // Calculation should consider the link between B and C as having cost = higherCostBC
  calculatePath();

  // Router A should be able to get to B through B and to B through C
  checkRoutingTableEntry(ROUTER_B_NAME, {
    {ROUTER_B_FACE, LINK_AB_COST},
    {ROUTER_C_FACE, LINK_AC_COST + higherCostBC},
  });

  // Router A should be able to get to C through C and to C through B
  checkRoutingTableEntry(ROUTER_C_NAME, {
    {ROUTER_C_FACE, LINK_AC_COST},
    {ROUTER_B_FACE, LINK_AB_COST + higherCostBC},
  });
}

BOOST_AUTO_TEST_CASE(NonAdjacentCost)
{
  // Break the link between B - C by setting it to NON_ADJACENT_COST.
  setupRouterA();
  setupRouterB(
    Adjacent::NON_ADJACENT_COST // B to C
  );
  setupRouterC();

  // Calculation should consider the link between B and C as down
  calculatePath();

  // Router A should be able to get to B through B but not through C
  checkRoutingTableEntry(ROUTER_B_NAME, {
    {ROUTER_B_FACE, LINK_AB_COST},
  });

  // Router A should be able to get to C through C but not through B
  checkRoutingTableEntry(ROUTER_C_NAME, {
    {ROUTER_C_FACE, LINK_AC_COST},
  });
}

BOOST_AUTO_TEST_CASE(AsymmetricZeroCostLink)
{
  // Asymmetric and zero link cost between B - C, and B - A.
  setupRouterA(
    0 // A to B
  );
  setupRouterB(
    0, // B to C: effective cost is still LINK_BC_COST, as specified on C-B link
    0  // B to A
  );
  setupRouterC();

  // Calculation should consider 0 link-cost between A and B
  calculatePath();

  // Router A should be able to get to B through B and C
  checkRoutingTableEntry(ROUTER_B_NAME, {
    {ROUTER_B_FACE, 0},
    {ROUTER_C_FACE, LINK_AC_COST + LINK_BC_COST},
  });

  // Router A should be able to get to C through C and B
  checkRoutingTableEntry(ROUTER_C_NAME, {
    {ROUTER_C_FACE, LINK_AC_COST},
    {ROUTER_B_FACE, 0 + LINK_BC_COST},
  });
}

BOOST_AUTO_TEST_CASE(OnePath)
{
  double costBC = 2.0;
  setupRouterA();
  setupRouterB(
    costBC // B to C
  );
  setupRouterC(
    LINK_AC_COST, // C to A
    costBC // C to B
  );

  // Calculate only one path per destination router.
  conf.setMaxFacesPerPrefix(1);
  // This triggers a different code path.
  calculatePath();

  // Shortest path to router B is via router B.
  checkRoutingTableEntry(ROUTER_B_NAME, {
    {ROUTER_B_FACE, LINK_AB_COST},
  });

  // Shortest path to router C is via router B.
  checkRoutingTableEntry(ROUTER_C_NAME, {
    {ROUTER_B_FACE, LINK_AB_COST + costBC},
  });
}

BOOST_AUTO_TEST_CASE(SourceRouterAbsent)
{
  // RouterA does not exist in the LSDB.
  // setupRouterA is not invoked.
  setupRouterB(
    LINK_BC_COST, // B to C
    NAN           // B to A: skipped
  );
  setupRouterC(
    NAN // C to A: skipped
  );

  // Calculation should do nothing and not cause errors.
  calculatePath();

  // There should be no routes.
  BOOST_CHECK(routingTable.m_rTable.empty());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
