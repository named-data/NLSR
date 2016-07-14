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

#include "publisher/lsdb-status-publisher.hpp"
#include "tlv/lsdb-status.hpp"
#include "tlv/tlv-nlsr.hpp"

#include "publisher-fixture.hpp"
#include "../boost-test.hpp"

namespace nlsr {
namespace test {

BOOST_FIXTURE_TEST_SUITE(PublisherTestLsdbStatusPublisher, PublisherFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  // Install adjacency LSAs
  // Adjacency LSA for RouterA
  AdjLsa routerAAdjLsa;
  routerAAdjLsa.setOrigRouter("/RouterA");
  addAdjacency(routerAAdjLsa, "/RouterA/adjacency1", "udp://face-1", 10);
  lsdb.installAdjLsa(routerAAdjLsa);

  // Adjacency LSA for RouterB
  AdjLsa routerBAdjLsa;
  routerBAdjLsa.setOrigRouter("/RouterB");
  routerBAdjLsa.setLsSeqNo(5);
  addAdjacency(routerBAdjLsa, "/RouterB/adjacency1", "udp://face-1", 10);
  addAdjacency(routerBAdjLsa, "/RouterB/adjacency2", "udp://face-2", 20);
  addAdjacency(routerBAdjLsa, "/RouterB/adjacency3", "udp://face-3", 30);
  lsdb.installAdjLsa(routerBAdjLsa);

  // Install coordinate LSAs
  CoordinateLsa routerACorLsa = createCoordinateLsa("/RouterA", 10.0, 20.0);
  lsdb.installCoordinateLsa(routerACorLsa);

  CoordinateLsa routerBCorLsa = createCoordinateLsa("/RouterB", 123.45, 543.21);
  lsdb.installCoordinateLsa(routerBCorLsa);

  CoordinateLsa routerCCorLsa = createCoordinateLsa("/RouterC", 0.01, 0.02);
  lsdb.installCoordinateLsa(routerCCorLsa);

  // Install Name LSAs
  // Name LSA for RouterA
  NameLsa routerANameLsa;
  routerANameLsa.setOrigRouter("/RouterA");
  routerANameLsa.addName("/RouterA/name1");
  lsdb.installNameLsa(routerANameLsa);

  // Name LSA for RouterB
  NameLsa routerBNameLsa;
  routerBNameLsa.setOrigRouter("/RouterB");
  routerBNameLsa.addName("/RouterB/name1");
  routerBNameLsa.addName("/RouterB/name2");
  routerBNameLsa.addName("/RouterB/name3");
  lsdb.installNameLsa(routerBNameLsa);

  ndn::Name thisRouter("/This/Router");
  AdjacencyLsaPublisher adjacencyLsaPublisher(lsdb, *face, keyChain);
  CoordinateLsaPublisher coordinateLsaPublisher(lsdb, *face, keyChain);
  NameLsaPublisher nameLsaPublisher(lsdb, *face, keyChain);

  LsdbStatusPublisher publisher(lsdb, *face, keyChain,
                                adjacencyLsaPublisher,
                                coordinateLsaPublisher,
                                nameLsaPublisher);

  ndn::Name publishingPrefix = ndn::Name(thisRouter);
  publishingPrefix.append(Lsdb::NAME_COMPONENT).append(LsdbStatusPublisher::DATASET_COMPONENT);

  publisher.publish(publishingPrefix);
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_REQUIRE_EQUAL(face->sentData.size(), 1);

  ndn::Block parser = face->sentData[0].getContent();
  parser.parse();

  ndn::Block::element_const_iterator it = parser.elements_begin();

  BOOST_CHECK_EQUAL(it->type(), ndn::tlv::nlsr::LsdbStatus);

  tlv::LsdbStatus lsdbStatusTlv;
  BOOST_REQUIRE_NO_THROW(lsdbStatusTlv.wireDecode(*it));

  BOOST_CHECK_EQUAL(lsdbStatusTlv.hasAdjacencyLsas(), true);

  // Check adjacency LSAs
  std::list<tlv::AdjacencyLsa>::const_iterator adjLsaIt = lsdbStatusTlv.getAdjacencyLsas().begin();
  checkTlvAdjLsa(*adjLsaIt, routerAAdjLsa);

  ++adjLsaIt;
  checkTlvAdjLsa(*adjLsaIt, routerBAdjLsa);

  // Check coordinate LSAs
  std::list<tlv::CoordinateLsa>::const_iterator corLsaIt =
    lsdbStatusTlv.getCoordinateLsas().begin();
  checkTlvCoordinateLsa(*corLsaIt, routerACorLsa);

  ++corLsaIt;
  checkTlvCoordinateLsa(*corLsaIt, routerBCorLsa);

  ++corLsaIt;
  checkTlvCoordinateLsa(*corLsaIt, routerCCorLsa);

  // Check Name LSAs
  std::list<tlv::NameLsa>::const_iterator nameLsaIt = lsdbStatusTlv.getNameLsas().begin();
  checkTlvNameLsa(*nameLsaIt, routerANameLsa);

  ++nameLsaIt;
  checkTlvNameLsa(*nameLsaIt, routerBNameLsa);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
