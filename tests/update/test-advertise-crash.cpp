/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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

#include "../test-common.hpp"
#include "nlsr.hpp"

namespace nlsr {
namespace update {
namespace test {

class AdvertiseCrashFixture : public nlsr::test::UnitTestTimeFixture
{
public:
  AdvertiseCrashFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , nlsr(m_ioService, m_scheduler, face, m_keyChain)
    , namePrefixList(nlsr.getNamePrefixList())
    , updatePrefixUpdateProcessor(nlsr.getPrefixUpdateProcessor())
  {
    // Add an adjacency to nlsr
    Adjacent adj("/ndn/edu/test-site-2/%C1.Router/test",
                 ndn::FaceUri("udp://1.0.0.2"), 10, Adjacent::STATUS_INACTIVE, 0, 0);
    nlsr.getAdjacencyList().insert(adj);

    // Create a face dataset response with the face having the same uri as
    // the adjacent
    ndn::nfd::FaceStatus payload1;
    payload1.setFaceId(25401);
    payload1.setRemoteUri("udp://1.0.0.2");

    std::vector<ndn::nfd::FaceStatus> faces{payload1};

    // Set the network so the LSA prefix is constructed
    nlsr.getConfParameter().setNetwork("/ndn");
    nlsr.getConfParameter().setRouterName(ndn::Name("/This/router"));

    addIdentity(ndn::Name("/ndn/This/router"));

    // So that NLSR starts listening on prefixes
    nlsr.initialize();

    // Simulate a callback with fake response
    // This will trigger the undefined behavior found
    // in fib.cpp if an operation is done on non-existent faceUri
    nlsr.processFaceDataset(faces);

    this->advanceClocks(ndn::time::milliseconds(1), 10);

    face.sentInterests.clear();
  }

public:
  ndn::util::DummyClientFace face;

  Nlsr nlsr;
  NamePrefixList& namePrefixList;
  PrefixUpdateProcessor& updatePrefixUpdateProcessor;
};

BOOST_FIXTURE_TEST_CASE(TestAdvertiseCrash, AdvertiseCrashFixture)
{
  // Note that this test does not setup any security
  // and the interest will be rejected by NLSR.
  // This is because the bug triggers upon merely receiving
  // an advertise interest so security was not needed.

  // Advertise
  ndn::nfd::ControlParameters parameters;
  parameters.setName("/prefix/to/advertise/");
  ndn::Name advertiseCommand("/localhost/nlsr/prefix-update/advertise");
  advertiseCommand.append(parameters.wireEncode());

  std::shared_ptr<ndn::Interest> advertiseInterest = std::make_shared<ndn::Interest>(advertiseCommand);

  face.receive(*advertiseInterest);
  this->advanceClocks(ndn::time::milliseconds(10), 10);
}

} // namespace test
} // namespace update
} // namespace nlsr
