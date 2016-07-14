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

#include "publisher/lsa-publisher.hpp"
#include "tlv/adjacency.hpp"
#include "tlv/adjacency-lsa.hpp"
#include "tlv/tlv-nlsr.hpp"

#include "publisher-fixture.hpp"
#include "../boost-test.hpp"

namespace nlsr {
namespace test {

BOOST_FIXTURE_TEST_SUITE(PublisherTestLsaPublisher, PublisherFixture)

BOOST_AUTO_TEST_CASE(AdjacencyLsaPublisherBasic)
{
  ndn::Name thisRouter("/RouterA");

  // Adjacency LSA for RouterA
  AdjLsa routerALsa;
  routerALsa.setOrigRouter(thisRouter);
  addAdjacency(routerALsa, "/RouterA/adjacency1", "udp://face-1", 10);
  lsdb.installAdjLsa(routerALsa);

  // Adjacency LSA for RouterB
  AdjLsa routerBLsa;
  routerBLsa.setOrigRouter("/RouterB");
  routerBLsa.setLsSeqNo(5);
  addAdjacency(routerBLsa, "/RouterB/adjacency1", "udp://face-1", 10);
  addAdjacency(routerBLsa, "/RouterB/adjacency2", "udp://face-2", 20);
  addAdjacency(routerBLsa, "/RouterB/adjacency3", "udp://face-3", 30);
  lsdb.installAdjLsa(routerBLsa);

  AdjacencyLsaPublisher publisher(lsdb, *face, keyChain);

  ndn::Name publishingPrefix = ndn::Name(thisRouter);
  publishingPrefix.append(Lsdb::NAME_COMPONENT).append(AdjacencyLsaPublisher::DATASET_COMPONENT);

  publisher.publish(publishingPrefix);
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_REQUIRE_EQUAL(face->sentData.size(), 1);
  ndn::Block parser = face->sentData[0].getContent();
  parser.parse();

  // Check RouterB LSA
  ndn::Block::element_const_iterator it = parser.elements_begin();
  checkTlvAdjLsa(*it, routerBLsa);

  // Check RouterA LSA
  it++;
  checkTlvAdjLsa(*it, routerALsa);
}

BOOST_AUTO_TEST_CASE(CoordinateLsaBasic)
{
  ndn::Name thisRouter("/RouterA");

  CoordinateLsa routerALsa = createCoordinateLsa(thisRouter.toUri(), 10.0, 20.0);
  lsdb.installCoordinateLsa(routerALsa);

  CoordinateLsa routerBLsa = createCoordinateLsa("/RouterB", 123.45, 543.21);
  lsdb.installCoordinateLsa(routerBLsa);

  CoordinateLsa routerCLsa = createCoordinateLsa("/RouterC", 0.01, 0.02);
  lsdb.installCoordinateLsa(routerCLsa);

  CoordinateLsaPublisher publisher(lsdb, *face, keyChain);

  ndn::Name publishingPrefix = ndn::Name(thisRouter);
  publishingPrefix.append(Lsdb::NAME_COMPONENT).append(CoordinateLsaPublisher::DATASET_COMPONENT);

  publisher.publish(publishingPrefix);
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_REQUIRE_EQUAL(face->sentData.size(), 1);
  ndn::Block parser = face->sentData[0].getContent();
  parser.parse();

  // Check RouterC LSA
  ndn::Block::element_const_iterator it = parser.elements_begin();
  checkTlvCoordinateLsa(*it, routerCLsa);

  // Check RouterB LSA
  ++it;
  checkTlvCoordinateLsa(*it, routerBLsa);

  // Check RouterA LSA
  ++it;
  checkTlvCoordinateLsa(*it, routerALsa);
}

BOOST_AUTO_TEST_CASE(NameLsaBasic)
{
  ndn::Name thisRouter("/RouterA");

  // Name LSA for RouterA
  NameLsa routerALsa;
  routerALsa.setOrigRouter(thisRouter.toUri());
  routerALsa.addName(ndn::Name(thisRouter).append("name1"));
  lsdb.installNameLsa(routerALsa);

  // Name LSA for RouterB
  NameLsa routerBLsa;
  routerBLsa.setOrigRouter("/RouterB");
  routerBLsa.addName("/RouterB/name1");
  routerBLsa.addName("/RouterB/name2");
  routerBLsa.addName("/RouterB/name3");
  lsdb.installNameLsa(routerBLsa);

  NameLsaPublisher publisher(lsdb, *face, keyChain);

  ndn::Name publishingPrefix = ndn::Name(thisRouter);
  publishingPrefix.append(Lsdb::NAME_COMPONENT).append(NameLsaPublisher::DATASET_COMPONENT);

  publisher.publish(publishingPrefix);
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_REQUIRE_EQUAL(face->sentData.size(), 1);
  ndn::Block parser = face->sentData[0].getContent();
  parser.parse();

  // Check RouterB LSA
  ndn::Block::element_const_iterator it = parser.elements_begin();
  checkTlvNameLsa(*it, routerBLsa);

  // Check RouterA LSA
  it++;
  checkTlvNameLsa(*it, routerALsa);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
