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
#include "lsdb.hpp"
#include "nlsr.hpp"
#include "route/name-map.hpp"
#include "route/routing-table.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

namespace nlsr::tests {

constexpr time::system_clock::time_point MAX_TIME = time::system_clock::time_point::max();
static const ndn::Name ROUTER_A_NAME = "/ndn/router/a";
static const ndn::Name ROUTER_B_NAME = "/ndn/router/b";
static const ndn::Name ROUTER_C_NAME = "/ndn/router/c";
static const ndn::FaceUri ROUTER_A_FACE("udp4://10.0.0.1:6363");
static const ndn::FaceUri ROUTER_B_FACE("udp4://10.0.0.2:6363");
static const ndn::FaceUri ROUTER_C_FACE("udp4://10.0.0.3:6363");

class HyperbolicCalculatorFixture : public IoKeyChainFixture
{
public:
  HyperbolicCalculatorFixture()
    : face(m_io, m_keyChain)
    , conf(face, m_keyChain)
    , nlsr(face, m_keyChain, conf)
    , routingTable(nlsr.m_routingTable)
    , adjacencies(conf.getAdjacencyList())
    , lsdb(nlsr.m_lsdb)
  {
  }

  // Triangle topology with routers A, B, C connected
  void setUpTopology(std::vector<double> anglesA, std::vector<double> anglesB,
                     std::vector<double> anglesC)
  {
    Adjacent a(ROUTER_A_NAME, ndn::FaceUri(ROUTER_A_FACE), 0, Adjacent::STATUS_ACTIVE, 0, 0);
    Adjacent b(ROUTER_B_NAME, ndn::FaceUri(ROUTER_B_FACE), 0, Adjacent::STATUS_ACTIVE, 0, 0);
    Adjacent c(ROUTER_C_NAME, ndn::FaceUri(ROUTER_C_FACE), 0, Adjacent::STATUS_ACTIVE, 0, 0);

    // Router A
    adjacencies.insert(b);
    adjacencies.insert(c);

    AdjLsa adjA(a.getName(), 1, MAX_TIME, adjacencies);
    lsdb.installLsa(std::make_shared<AdjLsa>(adjA));

    CoordinateLsa coordA(adjA.getOriginRouter(), 1, MAX_TIME, 16.23, anglesA);
    lsdb.installLsa(std::make_shared<CoordinateLsa>(coordA));

    // Router B
    a.setFaceId(1);
    c.setFaceId(2);

    AdjacencyList adjacencyListB;
    adjacencyListB.insert(a);
    adjacencyListB.insert(c);

    AdjLsa adjB(b.getName(), 1, MAX_TIME, adjacencyListB);
    lsdb.installLsa(std::make_shared<AdjLsa>(adjB));

    CoordinateLsa coordB(adjB.getOriginRouter(), 1, MAX_TIME, 16.59, anglesB);
    lsdb.installLsa(std::make_shared<CoordinateLsa>(coordB));

    // Router C
    a.setFaceId(1);
    b.setFaceId(2);

    AdjacencyList adjacencyListC;
    adjacencyListC.insert(a);
    adjacencyListC.insert(b);

    AdjLsa adjC(c.getName(), 1, MAX_TIME, adjacencyListC);
    lsdb.installLsa(std::make_shared<AdjLsa>(adjC));

    CoordinateLsa coordC(adjC.getOriginRouter(), 1, MAX_TIME, 14.11, anglesC);
    lsdb.installLsa(std::make_shared<CoordinateLsa>(coordC));

    auto lsaRange = lsdb.getLsdbIterator<CoordinateLsa>();
    map = NameMap::createFromCoordinateLsdb(lsaRange.first, lsaRange.second);
  }

  void runTest(const double& expectedCost)
  {
    calculateHyperbolicRoutingPath(map, routingTable, lsdb, adjacencies, ROUTER_A_NAME, false);

    RoutingTableEntry* entryB = routingTable.findRoutingTableEntry(ROUTER_B_NAME);

    // Router A should be able to get to B through B with cost 0 and to B through C
    NexthopList& bHopList = entryB->getNexthopList();
    BOOST_REQUIRE_EQUAL(bHopList.getNextHops().size(), 2);

    for (auto it = bHopList.begin(); it != bHopList.end(); ++it) {
      auto faceUri = it->getConnectingFaceUri();
      uint64_t cost = it->getRouteCostAsAdjustedInteger();

      BOOST_CHECK((faceUri == ROUTER_B_FACE && cost == 0) ||
                  (faceUri == ROUTER_C_FACE && cost == applyHyperbolicFactorAndRound(expectedCost)));
    }

    RoutingTableEntry* entryC = routingTable.findRoutingTableEntry(ROUTER_C_NAME);

    // Router A should be able to get to C through C with cost 0 and to C through B
    NexthopList& cHopList = entryC->getNexthopList();
    BOOST_REQUIRE_EQUAL(cHopList.getNextHops().size(), 2);

    for (auto it = cHopList.begin(); it != cHopList.end(); ++it) {
      auto faceUri = it->getConnectingFaceUri();
      uint64_t cost = it->getRouteCostAsAdjustedInteger();

      BOOST_CHECK((faceUri == ROUTER_B_FACE && cost == applyHyperbolicFactorAndRound(expectedCost)) ||
                  (faceUri == ROUTER_C_FACE && cost == 0));
    }
  }

  uint64_t
  applyHyperbolicFactorAndRound(double d)
  {
    // Hyperbolic costs in the tests were calculated with 1*10^-9 precision.
    // A factor larger than 1*10^9 will cause the tests to fail.
    BOOST_REQUIRE(NextHop::HYPERBOLIC_COST_ADJUSTMENT_FACTOR <= 1000000000);
    return round(NextHop::HYPERBOLIC_COST_ADJUSTMENT_FACTOR*d);
  }

public:
  ndn::DummyClientFace face;
  ConfParameter conf;
  Nlsr nlsr;
  NameMap map;

  RoutingTable& routingTable;
  AdjacencyList& adjacencies;
  Lsdb& lsdb;
};

BOOST_FIXTURE_TEST_SUITE(TestRoutingCalculatorHyperbolic, HyperbolicCalculatorFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  std::vector<double> anglesA = {2.97},
                      anglesB = {3.0},
                      anglesC = {2.99};
  setUpTopology(anglesA, anglesB, anglesC);

  runTest(20.103356956);
}

BOOST_AUTO_TEST_CASE(BasicMultipleAngles)
{
  std::vector<double> anglesA = {2.97,1.22},
                      anglesB = {3.0, 0.09},
                      anglesC = {321, 2.99};
  setUpTopology(anglesA, anglesB, anglesC);

  runTest(30.655296361);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
