/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
 *                           Regents of the University of California
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

#include "route/fib.hpp"
#include "adjacency-list.hpp"
#include "conf-parameter.hpp"

#include "tests/boost-test.hpp"
#include "tests/io-key-chain-fixture.hpp"

#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr::tests {

static const ndn::Name router1Name = "/ndn/router1";
static const ndn::Name router2Name = "/ndn/router2";
static const ndn::Name router3Name = "/ndn/router3";
static const ndn::FaceUri router1FaceUri("udp4://10.0.0.1:6363");
static const ndn::FaceUri router2FaceUri("udp4://10.0.0.2:6363");
static const ndn::FaceUri router3FaceUri("udp4://10.0.0.3:6363");
constexpr uint32_t router1FaceId = 1;
constexpr uint32_t router2FaceId = 2;
constexpr uint32_t router3FaceId = 3;

class FibFixture : public IoKeyChainFixture
{
public:
  FibFixture()
    : adjacencies(conf.getAdjacencyList())
    , fib(face, m_scheduler, adjacencies, conf, m_keyChain)
    , interests(face.sentInterests)
  {
    advanceClocks(1_s);

    Adjacent neighbor1(router1Name, router1FaceUri, 0, Adjacent::STATUS_ACTIVE, 0, router1FaceId);
    adjacencies.insert(neighbor1);

    Adjacent neighbor2(router2Name, router2FaceUri, 0, Adjacent::STATUS_ACTIVE, 0, router2FaceId);
    adjacencies.insert(neighbor2);

    Adjacent neighbor3(router3Name, router3FaceUri, 0, Adjacent::STATUS_ACTIVE, 0, router3FaceId);
    adjacencies.insert(neighbor3);

    conf.setMaxFacesPerPrefix(2);

    fib.setEntryRefreshTime(1);
  }

  static void
  extractRibCommandParameters(const ndn::Interest& interest,
                              ndn::Name::Component& verb,
                              ndn::nfd::ControlParameters& params)
  {
    const auto& name = interest.getName();
    verb = name.at(RIB_COMMAND_PREFIX.size());
    params.wireDecode(name.at(RIB_COMMAND_PREFIX.size() + 1).blockFromValue());
  }

private:
  static inline const ndn::Name RIB_COMMAND_PREFIX{"/localhost/nfd/rib"};

  ndn::Scheduler m_scheduler{m_io};

public:
  ndn::DummyClientFace face{m_io, m_keyChain, [] {
    ndn::DummyClientFace::Options opts;
    opts.enableRegistrationReply = true;
    opts.registrationReplyFaceId = 128;
    return opts;
  } ()};
  ConfParameter conf{face, m_keyChain};
  AdjacencyList& adjacencies;
  Fib fib;
  std::vector<ndn::Interest>& interests;
};

BOOST_FIXTURE_TEST_SUITE(TestFib, FibFixture)

BOOST_AUTO_TEST_CASE(NextHopsAdd)
{
  NextHop hop1(router1FaceUri, 10);
  NextHop hop2(router2FaceUri, 20);

  NexthopList hops;
  hops.addNextHop(hop1);
  hops.addNextHop(hop2);

  fib.update("/ndn/name", hops);
  face.processEvents(ndn::time::milliseconds(-1));

  // Should register faces 1 and 2 for /ndn/name
  BOOST_REQUIRE_EQUAL(interests.size(), 2);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  auto it = interests.begin();

  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router1FaceId &&
              verb == ndn::Name::Component("register"));

  ++it;
  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router2FaceId &&
              verb == ndn::Name::Component("register"));
}

BOOST_AUTO_TEST_CASE(NextHopsNoChange)
{
  NextHop hop1(router1FaceUri, 10);
  NextHop hop2(router2FaceUri, 20);

  NexthopList oldHops;
  oldHops.addNextHop(hop1);
  oldHops.addNextHop(hop2);

  fib.update("/ndn/name", oldHops);
  face.processEvents(ndn::time::milliseconds(-1));

  BOOST_REQUIRE_EQUAL(interests.size(), 2);
  interests.clear();

  fib.update("/ndn/name", oldHops);
  face.processEvents(ndn::time::milliseconds(-1));

  // Should register face 1 and 2 for /ndn/name
  BOOST_REQUIRE_EQUAL(interests.size(), 2);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  auto it = interests.begin();

  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router1FaceId &&
              verb == ndn::Name::Component("register"));

  ++it;
  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router2FaceId &&
              verb == ndn::Name::Component("register"));
}

BOOST_AUTO_TEST_CASE(NextHopsRemoveAll)
{
  NextHop hop1(router1FaceUri, 10);
  NextHop hop2(router2FaceUri, 20);

  NexthopList oldHops;
  oldHops.addNextHop(hop1);
  oldHops.addNextHop(hop2);

  fib.update("/ndn/name", oldHops);
  face.processEvents(ndn::time::milliseconds(-1));

  BOOST_REQUIRE_EQUAL(interests.size(), 2);
  interests.clear();

  NexthopList empty;

  fib.update("/ndn/name", empty);
  face.processEvents(ndn::time::milliseconds(-1));

  // Should unregister faces 1 and 2 for /ndn/name
  BOOST_CHECK_EQUAL(interests.size(), 2);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  auto it = interests.begin();

  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router1FaceId &&
              verb == ndn::Name::Component("unregister"));

  ++it;
  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router2FaceId &&
              verb == ndn::Name::Component("unregister"));
}

BOOST_AUTO_TEST_CASE(NextHopsMaxPrefixes)
{
  NextHop hop1(router1FaceUri, 10);
  NextHop hop2(router2FaceUri, 20);
  NextHop hop3(router3FaceUri, 30);

  NexthopList hops;
  hops.addNextHop(hop1);
  hops.addNextHop(hop2);
  hops.addNextHop(hop3);

  fib.update("/ndn/name", hops);
  face.processEvents(ndn::time::milliseconds(-1));

  // Should only register faces 1 and 2 for /ndn/name
  BOOST_CHECK_EQUAL(interests.size(), 2);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  auto it = interests.begin();

  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router1FaceId &&
              verb == ndn::Name::Component("register"));

  ++it;
  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router2FaceId &&
              verb == ndn::Name::Component("register"));
}

BOOST_AUTO_TEST_CASE(NextHopsMaxPrefixesAfterRecalculation)
{
  NextHop hop1(router1FaceUri, 10);
  NextHop hop2(router2FaceUri, 20);

  NexthopList hops;
  hops.addNextHop(hop1);
  hops.addNextHop(hop2);

  fib.update("/ndn/name", hops);
  face.processEvents(ndn::time::milliseconds(-1));

  // FIB
  // Name        NextHops
  // /ndn/name   (faceId=1, cost=10), (faceId=2, cost=20)
  BOOST_REQUIRE_EQUAL(interests.size(), 2);
  interests.clear();

  // Routing table is recalculated; a new more optimal path is found
  NextHop hop3(router3FaceUri, 5);
  hops.addNextHop(hop3);

  fib.update("/ndn/name", hops);
  face.processEvents(ndn::time::milliseconds(-1));

  // To maintain a max 2 face requirement, face 3 should be registered and face 2 should be
  // unregistered. Face 1 will also be re-registered.
  //
  // FIB
  // Name         NextHops
  // /ndn/name    (faceId=3, cost=5), (faceId=1, cost=10)

  BOOST_CHECK_EQUAL(interests.size(), 3);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  auto it = interests.begin();

  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router1FaceId &&
              verb == ndn::Name::Component("register"));

  ++it;
  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router3FaceId &&
              verb == ndn::Name::Component("register"));

  ++it;
  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router2FaceId &&
              verb == ndn::Name::Component("unregister"));
}

BOOST_FIXTURE_TEST_CASE(ScheduleFibEntryRefresh, FibFixture)
{
  ndn::Name name1("/name/1");
  FibEntry fe;
  fe.name = name1;
  int origSeqNo = fe.seqNo;
  fib.m_table.emplace(name1, std::move(fe));

  fib.scheduleEntryRefresh(fe, [&] (auto& entry) { BOOST_CHECK_EQUAL(origSeqNo + 1, entry.seqNo); });
  this->advanceClocks(ndn::time::milliseconds(10), 1);

  // avoid "test case [...] did not check any assertions" message from Boost.Test
  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(ShouldNotRefreshNeighborRoute) // #4799
{
  NextHop hop1;
  hop1.setConnectingFaceUri(router1FaceUri);

  NexthopList hops;
  hops.addNextHop(hop1);

  // Simulate update for this neighbor from name prefix table
  fib.update(router1Name, hops);
  this->advanceClocks(ndn::time::seconds(1));

  // Should not send the register interest
  BOOST_CHECK_EQUAL(face.sentInterests.size(), 0);
}

BOOST_AUTO_TEST_CASE(PrefixWithdrawalFibUpdateBug) // #5179
{
  fib.setEntryRefreshTime(3600);
  conf.setMaxFacesPerPrefix(3);

  // Three adjacencies of Neu
  Adjacent neighbor1("/ndn/memphis/router-memphis",
                     ndn::FaceUri("udp4://10.0.0.9:6363"), 21, Adjacent::STATUS_ACTIVE, 0, router1FaceId);
  adjacencies.insert(neighbor1);

  Adjacent neighbor2("/ndn/michigan/router-michigan",
                     ndn::FaceUri("udp4://10.0.0.13:6363"), 14, Adjacent::STATUS_ACTIVE, 0, router2FaceId);
  adjacencies.insert(neighbor2);

  Adjacent neighbor3("/ndn/savi/router-savi",
                     ndn::FaceUri("udp4://10.0.0.26:6363"), 15, Adjacent::STATUS_ACTIVE, 0, router3FaceId);
  adjacencies.insert(neighbor3);

  // Wustl advertises /test
  NexthopList nhl1;
  nhl1.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.13:6363"), 28));
  nhl1.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.9:6363"), 38));
  nhl1.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.26:6363"), 44));
  fib.update("/test", nhl1);

  // Memphis advertises /test
  NexthopList nhl2;
  nhl2.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.9:6363"), 21));
  nhl2.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.13:6363"), 26));
  nhl2.addNextHop(NextHop(ndn::FaceUri("udp4://10.0.0.26:6363"), 42));
  fib.update("/test", nhl2);

  advanceClocks(10_ms);
  face.sentInterests.clear();
  // Memphis withdraws /test
  // NamePrefixTable calls this saying we need to install the Wu routes
  // instead of the existing Memphis' cheaper routes
  fib.update("/test", nhl1);

  advanceClocks(10_ms);
  int numRegister = 0;
  // Do not expect any unregisters, just registers which will update the cost in NFD
  for (const auto& i : face.sentInterests) {
    if (i.getName().getPrefix(4) == "/localhost/nfd/rib/unregister") {
      BOOST_CHECK(false);
    }
    else {
      ++numRegister;
    }
  }
  BOOST_CHECK_EQUAL(numRegister, 3);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
