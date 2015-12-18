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
 **/

#include "test-common.hpp"
#include "control-commands.hpp"

#include "route/fib.hpp"

#include "adjacency-list.hpp"
#include "conf-parameter.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

using ndn::shared_ptr;

class FibFixture : public BaseFixture
{
public:
  FibFixture()
    : face(make_shared<ndn::util::DummyClientFace>())
    , interests(face->sentInterests)
  {
    INIT_LOGGERS("/tmp", "DEBUG");

    Adjacent neighbor1(router1Name, router1FaceUri, 0, Adjacent::STATUS_ACTIVE, 0, router1FaceId);
    adjacencies.insert(neighbor1);

    Adjacent neighbor2(router2Name, router2FaceUri, 0, Adjacent::STATUS_ACTIVE, 0, router2FaceId);
    adjacencies.insert(neighbor2);

    Adjacent neighbor3(router3Name, router3FaceUri, 0, Adjacent::STATUS_ACTIVE, 0, router3FaceId);
    adjacencies.insert(neighbor3);

    conf.setMaxFacesPerPrefix(2);

    fib = ndn::make_shared<Fib>(ndn::ref(*face), ndn::ref(g_scheduler), ndn::ref(adjacencies),
                                ndn::ref(conf), keyChain);

    fib->m_faceMap.update(router1FaceUri, router1FaceId);
    fib->m_faceMap.update(router2FaceUri, router2FaceId);
    fib->m_faceMap.update(router3FaceUri, router3FaceId);
  }

public:
  shared_ptr<ndn::util::DummyClientFace> face;
  ndn::KeyChain keyChain;
  shared_ptr<Fib> fib;

  AdjacencyList adjacencies;
  ConfParameter conf;
  std::vector<ndn::Interest>& interests;

  static const ndn::Name router1Name;
  static const ndn::Name router2Name;
  static const ndn::Name router3Name;

  static const std::string router1FaceUri;
  static const std::string router2FaceUri;
  static const std::string router3FaceUri;

  static const uint32_t router1FaceId;
  static const uint32_t router2FaceId;
  static const uint32_t router3FaceId;
};

const ndn::Name FibFixture::router1Name = "/ndn/router1";
const ndn::Name FibFixture::router2Name = "/ndn/router2";
const ndn::Name FibFixture::router3Name = "/ndn/router3";

const std::string FibFixture::router1FaceUri = "uri://face1";
const std::string FibFixture::router2FaceUri = "uri://face2";
const std::string FibFixture::router3FaceUri = "uri://face3";

const uint32_t FibFixture::router1FaceId = 1;
const uint32_t FibFixture::router2FaceId = 2;
const uint32_t FibFixture::router3FaceId = 3;

BOOST_FIXTURE_TEST_SUITE(TestFib, FibFixture)

BOOST_AUTO_TEST_CASE(NextHopsAdd)
{
  NextHop hop1(router1FaceUri, 10);
  NextHop hop2(router2FaceUri, 20);

  NexthopList hops;
  hops.addNextHop(hop1);
  hops.addNextHop(hop2);

  fib->update("/ndn/name", hops);
  face->processEvents(ndn::time::milliseconds(1));

  // Should register faces 1 and 2 for /ndn/name
  BOOST_REQUIRE_EQUAL(interests.size(), 2);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  std::vector<ndn::Interest>::iterator it = interests.begin();

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

  fib->update("/ndn/name", oldHops);
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_REQUIRE_EQUAL(interests.size(), 2);
  interests.clear();

  fib->update("/ndn/name", oldHops);
  face->processEvents(ndn::time::milliseconds(1));

  // Should register face 1 and 2 for /ndn/name
  BOOST_REQUIRE_EQUAL(interests.size(), 2);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  std::vector<ndn::Interest>::iterator it = interests.begin();

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

  fib->update("/ndn/name", oldHops);
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_REQUIRE_EQUAL(interests.size(), 2);
  interests.clear();

  NexthopList empty;

  fib->update("/ndn/name", empty);
  face->processEvents(ndn::time::milliseconds(1));

  // Should unregister faces 1 and 2 for /ndn/name
  BOOST_CHECK_EQUAL(interests.size(), 2);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  std::vector<ndn::Interest>::iterator it = interests.begin();

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

  fib->update("/ndn/name", hops);
  face->processEvents(ndn::time::milliseconds(1));

  // Should only register faces 1 and 2 for /ndn/name
  BOOST_CHECK_EQUAL(interests.size(), 2);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  std::vector<ndn::Interest>::iterator it = interests.begin();

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

  fib->update("/ndn/name", hops);
  face->processEvents(ndn::time::milliseconds(1));

  // FIB
  // Name        NextHops
  // /ndn/name   (faceId=1, cost=10), (faceId=2, cost=20)
  BOOST_REQUIRE_EQUAL(interests.size(), 2);
  interests.clear();

  // Routing table is recalculated; a new more optimal path is found
  NextHop hop3(router3FaceUri, 5);
  hops.addNextHop(hop3);

  fib->update("/ndn/name", hops);
  face->processEvents(ndn::time::milliseconds(1));

  // To maintain a max 2 face requirement, face 3 should be registered and face 2 should be
  // unregistered. Face 1 will also be re-registered.
  //
  // FIB
  // Name         NextHops
  // /ndn/name    (faceId=3, cost=5), (faceId=1, cost=10)

  BOOST_CHECK_EQUAL(interests.size(), 3);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  std::vector<ndn::Interest>::iterator it = interests.begin();

  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router3FaceId &&
              verb == ndn::Name::Component("register"));

  ++it;
  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router1FaceId &&
              verb == ndn::Name::Component("register"));

  ++it;
  extractRibCommandParameters(*it, verb, extractedParameters);

  BOOST_CHECK(extractedParameters.getName() == "/ndn/name" &&
              extractedParameters.getFaceId() == router2FaceId &&
              verb == ndn::Name::Component("unregister"));
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
