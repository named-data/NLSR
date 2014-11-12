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

#include "tests/test-common.hpp"
#include "tests/control-commands.hpp"
#include "tests/dummy-face.hpp"

#include "utility/face-controller.hpp"

#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/management/nfd-controller.hpp>

namespace nlsr {
namespace test {

using ndn::bind;
using ndn::shared_ptr;
using ndn::Interest;

class FaceControllerFixture : public BaseFixture
{
public:
  FaceControllerFixture()
    : face(ndn::makeDummyFace())
    , interests(face->m_sentInterests)
    , controller(*face, keyChain)
    , faceManager(g_ioService, controller)
  {
  }

  void
  expectCanonizeSuccess(const std::string& request, const std::string expectedUri)
  {
    faceManager.createFace(request, 0, bind(&FaceControllerFixture::onFailure, this, _1, _2));
    expectedUris.push_back(expectedUri);
  }

  void
  expectCanonizeFailure(const std::string& request)
  {
    faceManager.createFace(request, 0, bind(&FaceControllerFixture::onFailure, this, _1, _2));
  }

  void
  onFailure(uint32_t code, const std::string& reason)
  {
    BOOST_CHECK_EQUAL(code, 408);
  }

public:
  shared_ptr<ndn::DummyFace> face;
  ndn::KeyChain keyChain;
  std::vector<Interest>& interests;
  ndn::nfd::Controller controller;
  util::FaceController faceManager;

  std::list<std::string> expectedUris;
};

BOOST_FIXTURE_TEST_SUITE(TestFaceController, FaceControllerFixture)

BOOST_AUTO_TEST_CASE(FaceCreateCanonize)
{
  expectCanonizeSuccess("udp4://192.0.2.1:6363", "udp4://192.0.2.1:6363");
  expectCanonizeSuccess("tcp4://192.0.2.1:6363", "tcp4://192.0.2.1:6363");

  expectCanonizeSuccess("udp://192.0.2.1", "udp4://192.0.2.1:6363");
  expectCanonizeSuccess("udp://203.0.113.1:6363", "udp4://203.0.113.1:6363");
  expectCanonizeSuccess("udp4://google-public-dns-a.google.com", "udp4://8.8.8.8:6363");

  expectCanonizeSuccess("tcp://192.0.2.1", "tcp4://192.0.2.1:6363");
  expectCanonizeSuccess("tcp://203.0.113.1:6363", "tcp4://203.0.113.1:6363");
  expectCanonizeSuccess("tcp4://google-public-dns-a.google.com", "tcp4://8.8.8.8:6363");

  expectCanonizeSuccess("udp6://[2001:4860:4860::8888]:6363", "udp6://[2001:4860:4860::8888]:6363");
  expectCanonizeSuccess("tcp6://[2001:4860:4860::8888]:6363", "tcp6://[2001:4860:4860::8888]:6363");

  expectCanonizeSuccess("udp://[2001:4860:4860::8888]", "udp6://[2001:4860:4860::8888]:6363");
  expectCanonizeSuccess("udp://[2001:4860:4860::8888]:6363", "udp6://[2001:4860:4860::8888]:6363");
  expectCanonizeSuccess("udp6://google-public-dns-a.google.com",
                        "udp6://[2001:4860:4860::8888]:6363");

  expectCanonizeSuccess("tcp://[2001:4860:4860::8888]", "tcp6://[2001:4860:4860::8888]:6363");
  expectCanonizeSuccess("tcp://[2001:4860:4860::8888]:6363", "tcp6://[2001:4860:4860::8888]:6363");
  expectCanonizeSuccess("tcp6://google-public-dns-a.google.com",
                        "tcp6://[2001:4860:4860::8888]:6363");

  g_ioService.run();
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_CHECK_EQUAL(interests.size(), expectedUris.size());

  for (std::vector<ndn::Interest>::iterator it = interests.begin(); it != interests.end(); ++it) {

    ndn::nfd::ControlParameters extractedParameters;
    ndn::Name::Component verb;

    extractFaceCommandParameters(*it, verb, extractedParameters);

    BOOST_CHECK_EQUAL(verb, ndn::Name::Component("create"));

    std::list<std::string>::iterator uri = std::find(expectedUris.begin(),
                                                     expectedUris.end(),
                                                     extractedParameters.getUri());

    BOOST_REQUIRE(uri != expectedUris.end());
    expectedUris.erase(uri);
  }
}

BOOST_AUTO_TEST_CASE(FaceCreateFailure)
{
  expectCanonizeFailure("udp4://not-a-valid.uri");
  expectCanonizeFailure("udp4://256.0.0.1:6363");

  g_ioService.run();
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_CHECK_EQUAL(interests.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
