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

#include "tests/key-chain-fixture.hpp"

#include <ndn-cxx/util/io.hpp>

#include <filesystem>
#include <system_error>

namespace nlsr::tests {

using namespace ndn::security;

KeyChainFixture::KeyChainFixture()
  : m_keyChain("pib-memory:", "tpm-memory:")
{
}

KeyChainFixture::~KeyChainFixture()
{
  std::error_code ec;
  for (const auto& certFile : m_certFiles) {
    std::filesystem::remove(certFile, ec); // ignore error
  }
}

bool
KeyChainFixture::saveCert(const ndn::Data& cert, const std::string& filename)
{
  m_certFiles.push_back(filename);
  try {
    ndn::io::save(cert, filename);
    return true;
  }
  catch (const ndn::io::Error&) {
    return false;
  }
}

bool
KeyChainFixture::saveIdentityCert(const Identity& identity, const std::string& filename)
{
  Certificate cert;
  try {
    cert = identity.getDefaultKey().getDefaultCertificate();
  }
  catch (const Pib::Error&) {
    return false;
  }

  return saveCert(cert, filename);
}

bool
KeyChainFixture::saveIdentityCert(const ndn::Name& identityName, const std::string& filename,
                                  bool allowCreate)
{
  Identity id;
  try {
    id = m_keyChain.getPib().getIdentity(identityName);
  }
  catch (const Pib::Error&) {
    if (allowCreate) {
      id = m_keyChain.createIdentity(identityName);
    }
  }

  if (!id) {
    return false;
  }

  return saveIdentityCert(id, filename);
}

Identity
KeyChainFixture::addSubCertificate(const ndn::Name& subIdentityName,
                                   const Identity& issuer,
                                   const ndn::KeyParams& params)
{
  auto subIdentity = m_keyChain.createIdentity(subIdentityName, params);

  auto request = subIdentity.getDefaultKey().getDefaultCertificate();
  ndn::security::MakeCertificateOptions opts;
  opts.issuerId = ndn::name::Component::fromUri("parent");
  m_keyChain.makeCertificate(request, ndn::signingByIdentity(issuer), opts);

  m_keyChain.setDefaultCertificate(subIdentity.getDefaultKey(), request);

  return subIdentity;
}

} // namespace nlsr::tests
