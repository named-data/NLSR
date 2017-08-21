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

#include "identity-management-fixture.hpp"

#include <ndn-cxx/util/io.hpp>
#include <ndn-cxx/security/v2/additional-description.hpp>

#include <boost/filesystem.hpp>

namespace nlsr {
namespace tests {

namespace v2 = ndn::security::v2;
namespace io = ndn::io;
namespace time = ndn::time;

IdentityManagementBaseFixture::~IdentityManagementBaseFixture()
{
  boost::system::error_code ec;
  for (const auto& certFile : m_certFiles) {
    boost::filesystem::remove(certFile, ec); // ignore error
  }
}

bool
IdentityManagementBaseFixture::saveCertToFile(const ndn::Data& obj,
                                              const std::string& filename)
{
  m_certFiles.insert(filename);
  try {
    io::save(obj, filename);
    return true;
  }
  catch (const io::Error&) {
    return false;
  }
}

IdentityManagementFixture::IdentityManagementFixture()
  : m_keyChain("pib-memory:", "tpm-memory:")
{
}

ndn::security::Identity
IdentityManagementFixture::addIdentity(const ndn::Name& identityName,
                                       const ndn::KeyParams& params)
{
  auto identity = m_keyChain.createIdentity(identityName, params);
  m_identities.insert(identityName);
  return identity;
}

bool
IdentityManagementFixture::saveCertificate(const ndn::security::Identity& identity,
                                           const std::string& filename)
{
  try {
    auto cert = identity.getDefaultKey().getDefaultCertificate();
    return saveCertToFile(cert, filename);
  }
  catch (const ndn::security::Pib::Error&) {
    return false;
  }
}

ndn::security::Identity
IdentityManagementFixture::addSubCertificate(const ndn::Name& subIdentityName,
                                             const ndn::security::Identity& issuer,
                                             const ndn::KeyParams& params)
{
  auto subIdentity = addIdentity(subIdentityName, params);

  v2::Certificate request = subIdentity.getDefaultKey().getDefaultCertificate();

  request.setName(request.getKeyName().append("parent").appendVersion());

  ndn::SignatureInfo info;
  info.setValidityPeriod(ndn::security::ValidityPeriod(time::system_clock::now(),
                                                       time::system_clock::now()
                                                       + time::days(7300)));

  v2::AdditionalDescription description;
  description.set("type", "sub-certificate");
  info.appendTypeSpecificTlv(description.wireEncode());

  m_keyChain.sign(request, ndn::signingByIdentity(issuer).setSignatureInfo(info));
  m_keyChain.setDefaultCertificate(subIdentity.getDefaultKey(), request);

  return subIdentity;
}

v2::Certificate
IdentityManagementFixture::addCertificate(const ndn::security::Key& key,
                                          const std::string& issuer)
{
  ndn::Name certificateName = key.getName();
  certificateName
    .append(issuer)
    .appendVersion();
  v2::Certificate certificate;
  certificate.setName(certificateName);

  // set metainfo
  certificate.setContentType(ndn::tlv::ContentType_Key);
  certificate.setFreshnessPeriod(time::hours(1));

  // set content
  certificate.setContent(key.getPublicKey().data(), key.getPublicKey().size());

  // set signature-info
  ndn::SignatureInfo info;
  info.setValidityPeriod(ndn::security::ValidityPeriod(time::system_clock::now(),
                                                       time::system_clock::now() + time::days(10)));

  m_keyChain.sign(certificate, ndn::signingByKey(key).setSignatureInfo(info));
  return certificate;
}

} // namespace tests
} // namespace nlsr
