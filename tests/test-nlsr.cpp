/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 *
 **/

 #include "test-common.hpp"
 #include "dummy-face.hpp"

 #include "nlsr.hpp"

namespace nlsr {
namespace test {

using ndn::DummyFace;
using ndn::shared_ptr;

BOOST_FIXTURE_TEST_SUITE(TestNlsr, BaseFixture)

BOOST_AUTO_TEST_CASE(HyperbolicOn_ZeroCostNeighbors)
{
  shared_ptr<DummyFace> face = ndn::makeDummyFace();
  Nlsr nlsr(g_ioService, g_scheduler, ndn::ref(*face));

  // Simulate loading configuration file
  AdjacencyList& neighbors = nlsr.getAdjacencyList();

  Adjacent neighborA("/ndn/neighborA", "uri://faceA", 25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", "uri://faceB", 10, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  Adjacent neighborC("/ndn/neighborC", "uri://faceC", 17, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborC);

  nlsr.getConfParameter().setHyperbolicState(HYPERBOLIC_STATE_ON);

  nlsr.initialize();

  std::list<Adjacent> neighborList = neighbors.getAdjList();
  for (std::list<Adjacent>::iterator it = neighborList.begin(); it != neighborList.end(); ++it) {
    BOOST_CHECK_EQUAL(it->getLinkCost(), 0);
  }
}

BOOST_AUTO_TEST_CASE(HyperbolicOff_LinkStateCost)
{
  shared_ptr<DummyFace> face = ndn::makeDummyFace();
  Nlsr nlsr(g_ioService, g_scheduler, ndn::ref(*face));

  // Simulate loading configuration file
  AdjacencyList& neighbors = nlsr.getAdjacencyList();

  Adjacent neighborA("/ndn/neighborA", "uri://faceA", 25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", "uri://faceB", 10, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  Adjacent neighborC("/ndn/neighborC", "uri://faceC", 17, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborC);

  nlsr.initialize();

  std::list<Adjacent> neighborList = neighbors.getAdjList();
  for (std::list<Adjacent>::iterator it = neighborList.begin(); it != neighborList.end(); ++it) {
    BOOST_CHECK(it->getLinkCost() != 0);
  }
}

BOOST_AUTO_TEST_CASE(SetEventIntervals)
{
  shared_ptr<DummyFace> face = ndn::makeDummyFace();
  Nlsr nlsr(g_ioService, g_scheduler, ndn::ref(*face));

  // Simulate loading configuration file
  ConfParameter& conf = nlsr.getConfParameter();
  conf.setAdjLsaBuildInterval(3);
  conf.setFirstHelloInterval(6);
  conf.setRoutingCalcInterval(9);

  nlsr.initialize();

  const Lsdb& lsdb = nlsr.getLsdb();
  const RoutingTable& rt = nlsr.getRoutingTable();

  BOOST_CHECK_EQUAL(lsdb.getAdjLsaBuildInterval(), ndn::time::seconds(3));
  BOOST_CHECK_EQUAL(nlsr.getFirstHelloInterval(), 6);
  BOOST_CHECK_EQUAL(rt.getRoutingCalcInterval(), ndn::time::seconds(9));
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
