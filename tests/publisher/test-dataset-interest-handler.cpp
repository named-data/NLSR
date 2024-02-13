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

#include "publisher/dataset-interest-handler.hpp"
#include "tlv-nlsr.hpp"

#include "tests/publisher/publisher-fixture.hpp"

namespace nlsr::tests {

BOOST_FIXTURE_TEST_SUITE(TestDatasetInterestHandler, PublisherFixture)

BOOST_AUTO_TEST_CASE(Localhost)
{
  checkPrefixRegistered(face, Nlsr::LOCALHOST_PREFIX);

  // Install adjacency LSA
  AdjLsa adjLsa;
  adjLsa.m_expirationTimePoint = ndn::time::system_clock::now() + 3600_s;
  adjLsa.m_originRouter = "/RouterA";
  addAdjacency(adjLsa, "/RouterA/adjacency1", "udp://face-1", 10);
  lsdb.installLsa(std::make_shared<AdjLsa>(adjLsa));

  std::vector<double> angles = {20.00, 30.00};

  // Install coordinate LSA
  CoordinateLsa coordinateLsa = createCoordinateLsa("/RouterA", 10.0, angles);
  lsdb.installLsa(std::make_shared<CoordinateLsa>(coordinateLsa));

  // Install routing table
  RoutingTableEntry rte1("desrouter1");
  const ndn::Name& DEST_ROUTER = rte1.getDestination();

  NextHop nh = createNextHop("udp://face-test1", 10);
  rt1.addNextHop(DEST_ROUTER, nh);

  // Request adjacency LSAs
  face.receive(ndn::Interest("/localhost/nlsr/lsdb/adjacencies").setCanBePrefix(true));
  processDatasetInterest([] (const ndn::Block& block) { return block.type() == nlsr::tlv::AdjacencyLsa; });

  // Request coordinate LSAs
  face.receive(ndn::Interest("/localhost/nlsr/lsdb/coordinates").setCanBePrefix(true));
  processDatasetInterest([] (const ndn::Block& block) { return block.type() == nlsr::tlv::CoordinateLsa; });

  // Request Name LSAs
  face.receive(ndn::Interest("/localhost/nlsr/lsdb/names").setCanBePrefix(true));
  processDatasetInterest([] (const ndn::Block& block) { return block.type() == nlsr::tlv::NameLsa; });

  // Request Routing Table
  face.receive(ndn::Interest("/localhost/nlsr/routing-table").setCanBePrefix(true));
  processDatasetInterest([] (const ndn::Block& block) { return block.type() == nlsr::tlv::RoutingTable; });
}

BOOST_AUTO_TEST_CASE(RouterName)
{
  ndn::Name regRouterPrefix(conf.getRouterPrefix());
  regRouterPrefix.append("nlsr");
  // Should already be added to dispatcher
  BOOST_CHECK_THROW(nlsr.m_dispatcher.addTopPrefix(regRouterPrefix), std::out_of_range);

  checkPrefixRegistered(face,regRouterPrefix);

  // Install adjacencies LSA
  AdjLsa adjLsa;
  adjLsa.m_originRouter = "/RouterA";
  addAdjacency(adjLsa, "/RouterA/adjacency1", "udp://face-1", 10);
  lsdb.installLsa(std::make_shared<AdjLsa>(adjLsa));

  std::vector<double> angles = {20.00, 30.00};

  // Install coordinate LSA
  CoordinateLsa coordinateLsa = createCoordinateLsa("/RouterA", 10.0, angles);
  lsdb.installLsa(std::make_shared<CoordinateLsa>(coordinateLsa));

  // Install routing table
  RoutingTableEntry rte1("desrouter1");
  NextHop nh = createNextHop("udp://face-test1", 10);
  rt1.addNextHop(rte1.getDestination(), nh);

  ndn::Name routerName(conf.getRouterPrefix());
  routerName.append("nlsr");

  // Request adjacency LSAs
  face.receive(ndn::Interest(ndn::Name(routerName).append("lsdb").append("adjacencies")).setCanBePrefix(true));
  processDatasetInterest([] (const auto& block) { return block.type() == nlsr::tlv::AdjacencyLsa; });

  // Request coordinate LSAs
  face.receive(ndn::Interest(ndn::Name(routerName).append("lsdb").append("coordinates")).setCanBePrefix(true));
  processDatasetInterest([] (const auto& block) { return block.type() == nlsr::tlv::CoordinateLsa; });

  // Request Name LSAs
  face.receive(ndn::Interest(ndn::Name(routerName).append("lsdb").append("names")).setCanBePrefix(true));
  processDatasetInterest([] (const auto& block) { return block.type() == nlsr::tlv::NameLsa; });

  // Request Routing Table
  face.receive(ndn::Interest(ndn::Name(routerName).append("routing-table")).setCanBePrefix(true));
  processDatasetInterest([] (const auto& block) { return block.type() == nlsr::tlv::RoutingTable; });
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
