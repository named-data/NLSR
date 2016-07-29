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

#include "tests/test-common.hpp"
#include "tests/control-commands.hpp"

#include "utility/face-controller.hpp"

#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/management/nfd-controller.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

using ndn::bind;
using ndn::shared_ptr;
using ndn::Interest;

class FaceControllerFixture : public BaseFixture
{
public:
  FaceControllerFixture()
    : face(make_shared<ndn::util::DummyClientFace>())
    , interests(face->sentInterests)
    , controller(*face, keyChain)
    , faceController(g_ioService, controller)
  {
  }

  void
  onFailure(uint32_t code, const std::string& reason)
  {
    BOOST_CHECK_EQUAL(code, 408);
  }

public:
  shared_ptr<ndn::util::DummyClientFace> face;
  ndn::KeyChain keyChain;
  std::vector<Interest>& interests;
  ndn::nfd::Controller controller;
  util::FaceController faceController;
};

BOOST_FIXTURE_TEST_SUITE(TestFaceController, FaceControllerFixture)

BOOST_AUTO_TEST_CASE(FaceCreateCanonizeSuccess)
{
  const std::string uri("udp4://192.0.2.1:6363");
  faceController.createFace(uri, nullptr, nullptr);

  face->processEvents(ndn::time::milliseconds(1));

  BOOST_REQUIRE_EQUAL(interests.size(), 1);
  Interest interest = interests.front();

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;

  extractFaceCommandParameters(interest, verb, extractedParameters);

  BOOST_CHECK_EQUAL(verb, ndn::Name::Component("create"));
  BOOST_CHECK_EQUAL(uri, extractedParameters.getUri());
}

BOOST_AUTO_TEST_CASE(FaceCreateCanonizeFailure)
{
  faceController.createFace("invalid://256.0.0.1:6363",
                            nullptr,
                            bind(&FaceControllerFixture::onFailure, this, _1, _2));

  face->processEvents(ndn::time::milliseconds(1));

  BOOST_CHECK_EQUAL(interests.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
