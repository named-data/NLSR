/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  Regents of the University of California,
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

#ifndef NLSR_TESTS_KEY_CHAIN_FIXTURE_HPP
#define NLSR_TESTS_KEY_CHAIN_FIXTURE_HPP

#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>

namespace nlsr::tests {

/**
 * @brief A fixture providing an in-memory KeyChain.
 *
 * Test cases can use this fixture to create identities. Identities, certificates, and
 * saved certificates are automatically removed during test teardown.
 */
class KeyChainFixture
{
protected:
  using Certificate = ndn::security::Certificate;
  using Identity    = ndn::security::Identity;
  using Key         = ndn::security::Key;

public:
  /**
   * @brief Saves an NDN certificate to a file
   * @return true if successful, false otherwise
   */
  bool
  saveCert(const ndn::Data& cert, const std::string& filename);

  /**
   * @brief Saves the default certificate of @p identity to a file
   * @return true if successful, false otherwise
   */
  bool
  saveIdentityCert(const Identity& identity, const std::string& filename);

  /**
   * @brief Saves the default certificate of the identity named @p identityName to a file
   * @param identityName Name of the identity
   * @param filename File name, must be writable
   * @param allowCreate If true, create the identity if it does not exist
   * @return true if successful, false otherwise
   */
  bool
  saveIdentityCert(const ndn::Name& identityName, const std::string& filename,
                   bool allowCreate = false);

  /**
   * @brief Issue a certificate for \p subidentityName signed by \p issuer
   *
   * If identity does not exist, it is created.
   * A new key is generated as the default key for identity.
   * A default certificate for the key is signed by the issuer using its default certificate.
   *
   * @return the sub identity
   */
  Identity
  addSubCertificate(const ndn::Name& identityName, const Identity& issuer,
                    const ndn::KeyParams& params = ndn::KeyChain::getDefaultKeyParams());

protected:
  KeyChainFixture();

  ~KeyChainFixture();

protected:
  ndn::KeyChain m_keyChain;

private:
  std::vector<std::string> m_certFiles;
};

} // namespace nlsr::tests

#endif // NLSR_TESTS_KEY_CHAIN_FIXTURE_HPP
