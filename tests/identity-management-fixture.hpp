/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
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

#ifndef NLSR_TESTS_IDENTITY_MANAGEMENT_FIXTURE_HPP
#define NLSR_TESTS_IDENTITY_MANAGEMENT_FIXTURE_HPP

#include "boost-test.hpp"
#include "test-home-fixture.hpp"

#include <vector>

#include <ndn-cxx/security/v2/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>

namespace nlsr {
namespace tests {

class IdentityManagementBaseFixture : public TestHomeFixture<DefaultPibDir>
{
public:
  ~IdentityManagementBaseFixture();

  bool
  saveCertToFile(const ndn::Data& obj, const std::string& filename);

protected:
  std::set<ndn::Name> m_identities;
  std::set<std::string> m_certFiles;
};

/**
 * @brief A test suite level fixture to help with identity management
 *
 * Test cases in the suite can use this fixture to create identities.  Identities,
 * certificates, and saved certificates are automatically removed during test teardown.
 */
class IdentityManagementFixture : public IdentityManagementBaseFixture
{
public:
  IdentityManagementFixture();

  /**
   * @brief Add identity @p identityName
   * @return name of the created self-signed certificate
   */
  ndn::security::Identity
  addIdentity(const ndn::Name& identityName, const ndn::KeyParams& params = ndn::KeyChain::getDefaultKeyParams());

  /**
   *  @brief Save identity certificate to a file
   *  @param identity identity
   *  @param filename file name, should be writable
   *  @return whether successful
   */
  bool
  saveCertificate(const ndn::security::Identity& identity, const std::string& filename);

  /**
   * @brief Issue a certificate for \p subidentityName signed by \p issuer
   *
   *  If identity does not exist, it is created.
   *  A new key is generated as the default key for identity.
   *  A default certificate for the key is signed by the issuer using its default certificate.
   *
   *  @return the sub identity
   */
  ndn::security::Identity
  addSubCertificate(const ndn::Name& identityName, const ndn::security::Identity& issuer,
                    const ndn::KeyParams& params = ndn::KeyChain::getDefaultKeyParams());

  /**
   * @brief Add a self-signed certificate to @p key with issuer ID @p issuer
   */
  ndn::security::v2::Certificate
  addCertificate(const ndn::security::Key& key, const std::string& issuer);

protected:
  ndn::KeyChain m_keyChain;
};

} // namespace tests
} // namespace nlsr

#endif // NLSR_TESTS_IDENTITY_MANAGEMENT_FIXTURE_HPP
