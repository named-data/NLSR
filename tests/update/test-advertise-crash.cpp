/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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

#include "nlsr.hpp"

#include "../test-common.hpp"

namespace nlsr {
namespace test {

class AdvertiseCrashFixture : public nlsr::test::UnitTestTimeFixture
{
public:
  AdvertiseCrashFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , conf(face, m_keyChain)
    , confProcessor(conf)
    , nlsr(face, m_keyChain, conf)
    , namePrefixList(conf.getNamePrefixList())
  {
    // Add an adjacency to nlsr
    Adjacent adj("/ndn/edu/test-site-2/%C1.Router/test",
                 ndn::FaceUri("udp://1.0.0.2"), 10, Adjacent::STATUS_INACTIVE, 0, 0);
    conf.getAdjacencyList().insert(adj);

    // Create a face dataset response with the face having the same uri as
    // the adjacent
    ndn::nfd::FaceStatus payload1;
    payload1.setFaceId(25401);
    payload1.setRemoteUri("udp://1.0.0.2");

    std::vector<ndn::nfd::FaceStatus> faces{payload1};

    addIdentity(conf.getRouterPrefix());

    // Simulate a callback with fake response
    // This will trigger the undefined behavior found
    // in fib.cpp if an operation is done on non-existent faceUri
    nlsr.processFaceDataset(faces);

    this->advanceClocks(ndn::time::milliseconds(1), 10);

    face.sentInterests.clear();
  }

public:
  ndn::util::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;

  Nlsr nlsr;
  NamePrefixList& namePrefixList;
};

BOOST_FIXTURE_TEST_CASE(TestAdvertiseCrash, AdvertiseCrashFixture)
{
  // Note that this test does not setup any security
  // and the interest will be rejected by NLSR.
  // This is because the bug triggers upon merely receiving
  // an advertise interest so security was not needed.

  // Advertise
  ndn::nfd::ControlParameters parameters;
  parameters.setName("/prefix/to/advertise");
  ndn::Name advertiseCommand("/localhost/nlsr/prefix-update/advertise");
  advertiseCommand.append(parameters.wireEncode());

  auto advertiseInterest = std::make_shared<ndn::Interest>(advertiseCommand);
  advertiseInterest->setCanBePrefix(false);
  face.receive(*advertiseInterest);
  this->advanceClocks(ndn::time::milliseconds(10), 10);
}

} // namespace test
} // namespace nlsr
