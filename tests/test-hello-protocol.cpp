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

#include "hello-protocol.hpp"
#include "nlsr.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

namespace nlsr::tests {

class HelloProtocolFixture : public IoKeyChainFixture
{
public:
  HelloProtocolFixture()
    : face(m_io, m_keyChain, {true, true})
    , conf(face, m_keyChain)
    , confProcessor(conf)
    , adjList(conf.getAdjacencyList())
    , nlsr(face, m_keyChain, conf)
    , helloProtocol(nlsr.m_helloProtocol)
  {
    ndn::FaceUri faceUri("udp4://10.0.0.1:6363");
    Adjacent adj1(ACTIVE_NEIGHBOR, faceUri, 10, Adjacent::STATUS_ACTIVE, 0, 300);
    adjList.insert(adj1);
  }

  int
  checkHelloInterests(ndn::Name name)
  {
    int sent = 0;
    for (const auto& i : face.sentInterests) {
      if (name == i.getName().getPrefix(4)) {
        sent++;
      }
    }
    return sent;
  }

  void
  checkHelloInterestTimeout()
  {
    helloProtocol.sendHelloInterest(ndn::Name(ACTIVE_NEIGHBOR));
    this->advanceClocks(10_ms);
    BOOST_CHECK_EQUAL(checkHelloInterests(ACTIVE_NEIGHBOR), 1);
    this->advanceClocks(4_s);
    BOOST_CHECK_EQUAL(checkHelloInterests(ACTIVE_NEIGHBOR), 2);
    this->advanceClocks(4_s);
    BOOST_CHECK_EQUAL(checkHelloInterests(ACTIVE_NEIGHBOR), 3);
    if (conf.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
      BOOST_CHECK_EQUAL(nlsr.m_routingTable.m_isRouteCalculationScheduled, false);
    }
    else {
      BOOST_CHECK_EQUAL(nlsr.m_lsdb.m_isBuildAdjLsaScheduled, false);
    }
    BOOST_CHECK_EQUAL(adjList.findAdjacent(ndn::Name(ACTIVE_NEIGHBOR))->getStatus(),
                      Adjacent::STATUS_ACTIVE);

    this->advanceClocks(4_s);
    BOOST_CHECK_EQUAL(checkHelloInterests(ACTIVE_NEIGHBOR), 3);
    if (conf.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
      BOOST_CHECK_EQUAL(nlsr.m_routingTable.m_isRouteCalculationScheduled, true);
    }
    else {
      BOOST_CHECK_EQUAL(nlsr.m_lsdb.m_isBuildAdjLsaScheduled, true);
    }
    BOOST_CHECK_EQUAL(adjList.findAdjacent(ndn::Name(ACTIVE_NEIGHBOR))->getStatus(),
                      Adjacent::STATUS_INACTIVE);
  }

public:
  ndn::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  AdjacencyList& adjList;
  Nlsr nlsr;
  HelloProtocol& helloProtocol;
  const std::string ACTIVE_NEIGHBOR = "/ndn/site/%C1.Router/router-active";
};

BOOST_FIXTURE_TEST_SUITE(TestHelloProtocol, HelloProtocolFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  this->advanceClocks(10_s);
  checkPrefixRegistered(face, "/ndn/site/%C1.Router/this-router/nlsr/INFO");
  face.sentInterests.clear();
}

BOOST_AUTO_TEST_CASE(HelloInterestTimeoutLS) // #5139
{
  checkHelloInterestTimeout();
}

BOOST_AUTO_TEST_CASE(HelloInterestTimeoutHR) // #5139
{
  conf.setHyperbolicState(HYPERBOLIC_STATE_ON);
  checkHelloInterestTimeout();
}

BOOST_AUTO_TEST_CASE(CheckHelloDataValidatedSignal) // # 5157
{
  int numOnInitialHelloDataValidates = 0;
  helloProtocol.onInitialHelloDataValidated.connect(
    [&] (const ndn::Name& neighbor) {
      ++numOnInitialHelloDataValidates;
    }
  );

  ndn::FaceUri faceUri("udp4://10.0.0.2:6363");
  Adjacent adj1("/ndn/site/%C1.Router/router-other", faceUri, 10,
                Adjacent::STATUS_INACTIVE, 0, 300);
  adjList.insert(adj1);

  ndn::Name dataName = adj1.getName();
  dataName.append(HelloProtocol::NLSR_COMPONENT);
  dataName.append(HelloProtocol::INFO_COMPONENT);
  dataName.append(ndn::tlv::GenericNameComponent, conf.getRouterPrefix().wireEncode());

  ndn::Data data(ndn::Name(dataName).appendVersion());
  BOOST_CHECK_EQUAL(numOnInitialHelloDataValidates, 0);
  helloProtocol.onContentValidated(data);
  BOOST_CHECK_EQUAL(numOnInitialHelloDataValidates, 1);
  BOOST_CHECK_EQUAL(adjList.getStatusOfNeighbor(adj1.getName()), Adjacent::STATUS_ACTIVE);

  // No state change of neighbor so no signal:
  ndn::Data data2(ndn::Name(dataName).appendVersion());
  helloProtocol.onContentValidated(data2);
  BOOST_CHECK_EQUAL(numOnInitialHelloDataValidates, 1);
  BOOST_CHECK_EQUAL(adjList.getStatusOfNeighbor(adj1.getName()), Adjacent::STATUS_ACTIVE);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
