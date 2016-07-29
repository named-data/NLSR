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

#include "security/certificate-store.hpp"

#include "../test-common.hpp"

#include <ndn-cxx/security/key-chain.hpp>

namespace nlsr {
namespace security {
namespace test {

using std::shared_ptr;

class CertificateStoreFixture
{
public:
  CertificateStoreFixture()
  {
    // Create certificate
    ndn::Name identity("/TestNLSR/identity");
    identity.appendVersion();

    ndn::KeyChain keyChain;
    keyChain.createIdentity(identity);
    ndn::Name certName = keyChain.getDefaultCertificateNameForIdentity(identity);
    certificate = keyChain.getCertificate(certName);

    BOOST_REQUIRE(certificate != nullptr);

    certificateKey = certificate->getName().getPrefix(-1);
  }

public:
  shared_ptr<ndn::IdentityCertificate> certificate;
  ndn::Name certificateKey;
};

BOOST_FIXTURE_TEST_SUITE(TestSecurityCertificateStore, CertificateStoreFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  CertificateStore store;

  BOOST_REQUIRE(store.find(certificateKey) == nullptr);
  store.insert(certificate);

  BOOST_CHECK(*store.find(certificateKey) == *certificate);

  store.clear();
  BOOST_REQUIRE(store.find(certificateKey) == nullptr);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace security
} // namespace nlsr
