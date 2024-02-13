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

#ifndef NLSR_TESTS_PUBLISHER_FIXTURE_HPP
#define NLSR_TESTS_PUBLISHER_FIXTURE_HPP

#include "publisher/dataset-interest-handler.hpp"
#include "nlsr.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

namespace nlsr::tests {

class PublisherFixture : public IoKeyChainFixture
{
public:
  PublisherFixture()
    : face(m_io, m_keyChain, {true, true})
    , conf(face, m_keyChain)
    , confProcessor(conf)
    , nlsr(face, m_keyChain, conf)
    , lsdb(nlsr.m_lsdb)
    , rt1(nlsr.m_routingTable)
  {
    routerId = m_keyChain.createIdentity(conf.getRouterPrefix());
    advanceClocks(100_ms);
  }

  void
  addAdjacency(AdjLsa& lsa, const std::string& name, const std::string& faceUri, double cost)
  {
    Adjacent adjacency(name, ndn::FaceUri(faceUri), cost, Adjacent::STATUS_ACTIVE, 0, 0);
    lsa.addAdjacent(std::move(adjacency));
  }

  NextHop
  createNextHop(const std::string& faceUri, double cost)
  {
    return {ndn::FaceUri(faceUri), cost};
  }

  CoordinateLsa
  createCoordinateLsa(const std::string& origin, double radius, std::vector<double> angle)
  {
    return {origin, 1, time::system_clock::now(), radius, angle};
  }

  void
  processDatasetInterest(std::function<bool(const ndn::Block&)> isSameType)
  {
    advanceClocks(30_ms);

    BOOST_REQUIRE_EQUAL(face.sentData.size(), 1);
    ndn::Block parser(face.sentData[0].getContent());
    parser.parse();
    face.sentData.clear();

    auto it = parser.elements_begin();
    BOOST_CHECK(isSameType(*it++));
    BOOST_CHECK(it == parser.elements_end());
  }

public:
  ndn::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  Nlsr nlsr;
  Lsdb& lsdb;

  ndn::security::pib::Identity routerId;
  RoutingTable& rt1;
};

} // namespace nlsr::tests

#endif // NLSR_TESTS_PUBLISHER_FIXTURE_HPP
