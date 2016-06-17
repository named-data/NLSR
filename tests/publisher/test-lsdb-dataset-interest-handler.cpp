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

#include "publisher/lsdb-dataset-interest-handler.hpp"
#include "tlv/tlv-nlsr.hpp"

#include "publisher-fixture.hpp"
#include "../boost-test.hpp"

#include <ndn-cxx/management/nfd-control-response.hpp>

namespace nlsr {
namespace test {

void
processDatasetInterest(shared_ptr<ndn::util::DummyClientFace> face,
                       std::function<bool(const ndn::Block&)> isSameType)
{
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_REQUIRE_EQUAL(face->sentData.size(), 1);
  ndn::Block parser(face->sentData[0].getContent());
  parser.parse();

  ndn::Block::element_const_iterator it = parser.elements_begin();
  BOOST_CHECK_EQUAL(isSameType(*it), true);
  ++it;

  BOOST_CHECK(it == parser.elements_end());

  face->sentData.clear();
}

void
checkErrorResponse(shared_ptr<ndn::util::DummyClientFace> face, uint64_t expectedCode)
{
  BOOST_REQUIRE_EQUAL(face->sentData.size(), 1);

  ndn::nfd::ControlResponse response(face->sentData[0].getContent().blockFromValue());
  BOOST_CHECK_EQUAL(response.getCode(), expectedCode);

  face->sentData.clear();
}

BOOST_FIXTURE_TEST_SUITE(PublisherTestLsdbDatasetInterestHandler, PublisherFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  // Install adjacency LSA
  AdjLsa adjLsa;
  adjLsa.setOrigRouter("/RouterA");
  addAdjacency(adjLsa, "/RouterA/adjacency1", "udp://face-1", 10);
  lsdb.installAdjLsa(adjLsa);

  // Install coordinate LSA
  CoordinateLsa coordinateLsa = createCoordinateLsa("/RouterA", 10.0, 20.0);
  lsdb.installCoordinateLsa(coordinateLsa);

  // Install Name LSA
  NameLsa nameLsa;
  nameLsa.setOrigRouter("/RouterA");
  nameLsa.addName("/RouterA/name1");
  lsdb.installNameLsa(nameLsa);

  ndn::Name thisRouter("/This/Router");
  LsdbDatasetInterestHandler publisher(lsdb, *face, keyChain);
  publisher.setRouterNameCommandPrefix(thisRouter);

  publisher.startListeningOnLocalhost();

  face->processEvents(ndn::time::milliseconds(10));

  // Localhost prefix
  ndn::Name localhostCommandPrefix = publisher.getLocalhostCommandPrefix();

  // Request adjacency LSAs
  face->receive(ndn::Interest(ndn::Name(localhostCommandPrefix).append("adjacencies")));
  processDatasetInterest(face,
    [] (const ndn::Block& block) { return block.type() == ndn::tlv::nlsr::AdjacencyLsa; });

  // Request coordinate LSAs
  face->receive(ndn::Interest(ndn::Name(localhostCommandPrefix).append("coordinates")));
  processDatasetInterest(face,
    [] (const ndn::Block& block) { return block.type() == ndn::tlv::nlsr::CoordinateLsa; });

  // Request Name LSAs
  face->receive(ndn::Interest(ndn::Name(localhostCommandPrefix).append("names")));
  processDatasetInterest(face,
    [] (const ndn::Block& block) { return block.type() == ndn::tlv::nlsr::NameLsa; });

  // Request LSDB Status
  face->receive(ndn::Interest(ndn::Name(localhostCommandPrefix).append("list")));
  processDatasetInterest(face,
    [] (const ndn::Block& block) { return block.type() == ndn::tlv::nlsr::LsdbStatus; });

  // Router name prefix
  ndn::Name routerCommandPrefix = publisher.getLocalhostCommandPrefix();

  // Request adjacency LSAs
  face->receive(ndn::Interest(ndn::Name(routerCommandPrefix).append("adjacencies")));
  processDatasetInterest(face,
    [] (const ndn::Block& block) { return block.type() == ndn::tlv::nlsr::AdjacencyLsa; });

  // Request coordinate LSAs
  face->receive(ndn::Interest(ndn::Name(routerCommandPrefix).append("coordinates")));
  processDatasetInterest(face,
    [] (const ndn::Block& block) { return block.type() == ndn::tlv::nlsr::CoordinateLsa; });

  // Request Name LSAs
  face->receive(ndn::Interest(ndn::Name(routerCommandPrefix).append("names")));
  processDatasetInterest(face,
    [] (const ndn::Block& block) { return block.type() == ndn::tlv::nlsr::NameLsa; });

  // Request LSDB Status
  face->receive(ndn::Interest(ndn::Name(routerCommandPrefix).append("list")));
  processDatasetInterest(face,
    [] (const ndn::Block& block) { return block.type() == ndn::tlv::nlsr::LsdbStatus; });
}

BOOST_AUTO_TEST_CASE(InvalidCommand)
{
  ndn::Name thisRouter("/This/Router");
  LsdbDatasetInterestHandler publisher(lsdb, *face, keyChain);
  publisher.setRouterNameCommandPrefix(thisRouter);

  // Localhost prefix
  publisher.startListeningOnLocalhost();
  face->processEvents(ndn::time::milliseconds(10));

  ndn::Name localhostCommandPrefix = publisher.getLocalhostCommandPrefix();

  // Unsupported command
  face->receive(ndn::Interest(ndn::Name(localhostCommandPrefix).append("unsupported")));
  face->processEvents(ndn::time::milliseconds(1));

  checkErrorResponse(face, LsdbDatasetInterestHandler::ERROR_CODE_UNSUPPORTED_COMMAND);

  // Long malformed command
  face->receive(
    ndn::Interest(ndn::Name(localhostCommandPrefix).append("extra").append("malformed")));
  face->processEvents(ndn::time::milliseconds(1));

  checkErrorResponse(face, LsdbDatasetInterestHandler::ERROR_CODE_MALFORMED_COMMAND);

  // Router name prefix
  publisher.startListeningOnRouterPrefix();
  face->processEvents(ndn::time::milliseconds(10));

  ndn::Name remoteCommandPrefix = publisher.getRouterNameCommandPrefix();

  // Unsupported command
  face->receive(ndn::Interest(ndn::Name(remoteCommandPrefix).append("unsupported")));
  face->processEvents(ndn::time::milliseconds(1));

  checkErrorResponse(face, LsdbDatasetInterestHandler::ERROR_CODE_UNSUPPORTED_COMMAND);

  // Long malformed command
  face->receive(ndn::Interest(ndn::Name(remoteCommandPrefix).append("extra").append("malformed")));
  face->processEvents(ndn::time::milliseconds(1));

  checkErrorResponse(face, LsdbDatasetInterestHandler::ERROR_CODE_MALFORMED_COMMAND);

  // Short malformed command
  face->receive(ndn::Interest(ndn::Name(thisRouter).append("malformed")));
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_CHECK_EQUAL(face->sentData.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
