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

#include "communication/sync-protocol-adapter.hpp"

#include "tests/boost-test.hpp"
#include "tests/io-key-chain-fixture.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr::tests {

using ndn::Name;

class SyncProtocolAdapterFixture : public IoKeyChainFixture
{
public:
  SyncProtocolAdapterFixture()
    : syncPrefix("/localhop/ndn/nlsr/sync")
    , nameLsaUserPrefix("/localhop/ndn/nlsr/LSA/NAME")
    , syncInterestLifetime(60_s)
  {
    syncPrefix.appendVersion(4);
  }

  void
  addNodes()
  {
    for (int i = 0; i < 2; i++) {
      faces[i] = std::make_shared<ndn::DummyClientFace>(m_io, ndn::DummyClientFace::Options{true, true});
      userPrefixes[i] = Name(nameLsaUserPrefix).appendNumber(i);
      nodes[i] = std::make_shared<SyncProtocolAdapter>(
        *faces[i], m_keyChain, SyncProtocol::PSYNC, syncPrefix, userPrefixes[i],
        syncInterestLifetime,
        [i, this] (const Name& updateName, uint64_t highSeq, uint64_t incomingFaceId) {
          prefixToSeq[i].emplace(updateName, highSeq);
        });
    }

    faces[0]->linkTo(*faces[1]);
    advanceClocks(10_ms, 10);
  }

public:
  Name syncPrefix;
  Name nameLsaUserPrefix;
  Name userPrefixes[2];
  time::milliseconds syncInterestLifetime;
  std::shared_ptr<ndn::DummyClientFace> faces[2];
  std::shared_ptr<SyncProtocolAdapter> nodes[2];
  std::map<Name, uint64_t> prefixToSeq[2];
};

BOOST_AUTO_TEST_SUITE(TestSyncProtocolAdapter)

BOOST_FIXTURE_TEST_CASE(Basic, SyncProtocolAdapterFixture)
{
  addNodes();

  nodes[0]->publishUpdate(userPrefixes[0], 10);
  advanceClocks(1_s, 100);

  auto it = prefixToSeq[1].find(userPrefixes[0]);
  BOOST_CHECK(it != prefixToSeq[1].end());
  BOOST_CHECK_EQUAL(it->first, userPrefixes[0]);
  BOOST_CHECK_EQUAL(it->second, 10);

  nodes[1]->publishUpdate(userPrefixes[1], 100);
  advanceClocks(1_s, 100);

  it = prefixToSeq[0].find(userPrefixes[1]);
  BOOST_CHECK(it != prefixToSeq[0].end());
  BOOST_CHECK_EQUAL(it->first, userPrefixes[1]);
  BOOST_CHECK_EQUAL(it->second, 100);

  Name adjLsaUserPrefix("/localhop/ndn/nlsr/LSA/ADJACENCY");
  nodes[0]->addUserNode(adjLsaUserPrefix);
  advanceClocks(1_s, 100);
  nodes[0]->publishUpdate(adjLsaUserPrefix, 10);
  advanceClocks(1_s, 100);

  it = prefixToSeq[1].find(adjLsaUserPrefix);
  BOOST_CHECK(it != prefixToSeq[1].end());
  BOOST_CHECK_EQUAL(it->first, adjLsaUserPrefix);
  BOOST_CHECK_EQUAL(it->second, 10);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
