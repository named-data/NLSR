/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2020,  The University of Memphis,
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

#include "certificate-store.hpp"
#include "conf-parameter.hpp"
#include "logger.hpp"

#include <ndn-cxx/util/io.hpp>

namespace nlsr {
namespace security {

INIT_LOGGER(CertificateStore);

CertificateStore::CertificateStore(ndn::Face& face, ConfParameter& confParam, Lsdb& lsdb)
  : m_face(face)
  , m_confParam(confParam)
  , m_lsdb(lsdb)
  , m_validator(m_confParam.getValidator())
  , m_afterSegmentValidatedConnection(m_lsdb.afterSegmentValidatedSignal.connect(
                                      std::bind(&CertificateStore::afterFetcherSignalEmitted,
                                                this, _1)))
{
  for (const auto& x: confParam.getIdCerts()) {
    auto idCert = ndn::io::load<ndn::security::v2::Certificate>(x);
    insert(*idCert);
  }

  registerKeyPrefixes();
}

void
CertificateStore::insert(const ndn::security::v2::Certificate& certificate)
{
  m_certificates[certificate.getKeyName()] = certificate;
  NLSR_LOG_TRACE("Certificate inserted successfully");
}

const ndn::security::v2::Certificate*
CertificateStore::find(const ndn::Name& keyName) const
{
  auto it = m_certificates.find(keyName);
  return it != m_certificates.end() ? &it->second : nullptr;
}

void
CertificateStore::clear()
{
  m_certificates.clear();
}

void
CertificateStore::setInterestFilter(const ndn::Name& prefix, bool loopback)
{
  m_face.setInterestFilter(ndn::InterestFilter(prefix).allowLoopback(loopback),
                           std::bind(&CertificateStore::onKeyInterest, this, _1, _2),
                           std::bind(&CertificateStore::onKeyPrefixRegSuccess, this, _1),
                           std::bind(&CertificateStore::registrationFailed, this, _1),
                           m_confParam.getSigningInfo(), ndn::nfd::ROUTE_FLAG_CAPTURE);
}

void
CertificateStore::registerKeyPrefixes()
{
  std::vector<ndn::Name> prefixes;

  // Router's NLSR certificate
  ndn::Name nlsrKeyPrefix = m_confParam.getRouterPrefix();
  nlsrKeyPrefix.append("nlsr");
  nlsrKeyPrefix.append("KEY");
  prefixes.push_back(nlsrKeyPrefix);

  // Router's certificate
  ndn::Name routerKeyPrefix = m_confParam.getRouterPrefix();
  routerKeyPrefix.append("KEY");
  prefixes.push_back(routerKeyPrefix);

  // Router's operator's certificate
  ndn::Name operatorKeyPrefix = m_confParam.getNetwork();
  operatorKeyPrefix.append(m_confParam.getSiteName());
  operatorKeyPrefix.append(std::string("%C1.Operator"));
  prefixes.push_back(operatorKeyPrefix);

  // Router's site's certificate
  ndn::Name siteKeyPrefix = m_confParam.getNetwork();
  siteKeyPrefix.append(m_confParam.getSiteName());
  siteKeyPrefix.append("KEY");
  prefixes.push_back(siteKeyPrefix);

  // Start listening for interest of this router's NLSR certificate,
  // router's certificate and site's certificate
  for (const auto& i : prefixes) {
    setInterestFilter(i);
  }
}

void
CertificateStore::onKeyInterest(const ndn::Name& name, const ndn::Interest& interest)
{
  NLSR_LOG_DEBUG("Got interest for certificate. Interest: " << interest.getName());

  const auto* cert = find(interest.getName());

  if (!cert) {
    NLSR_LOG_TRACE("Certificate is not found for: " << interest);
    return;
  }
  m_face.put(*cert);
}

void
CertificateStore::onKeyPrefixRegSuccess(const ndn::Name& name)
{
  NLSR_LOG_DEBUG("KEY prefix: " << name << " registration is successful.");
}

void
CertificateStore::registrationFailed(const ndn::Name& name)
{
  NLSR_LOG_ERROR("ERROR: Failed to register prefix " << name);
  BOOST_THROW_EXCEPTION(std::runtime_error("Prefix registration failed"));
}

void
CertificateStore::publishCertFromCache(const ndn::Name& keyName)
{
  const auto* cert = m_validator.getUnverifiedCertCache().find(keyName);

  if (cert) {
    insert(*cert);
    NLSR_LOG_TRACE(*cert);
    ndn::Name certName = ndn::security::v2::extractKeyNameFromCertName(cert->getName());
    NLSR_LOG_TRACE("Setting interest filter for: " << certName);

    setInterestFilter(certName);

    if (cert->getKeyName() != cert->getSignature().getKeyLocator().getName()) {
      publishCertFromCache(cert->getSignature().getKeyLocator().getName());
    }
  }
  else {
    // Happens for root cert
    NLSR_LOG_TRACE("Cert for " << keyName << " was not found in the Validator's cache. ");
  }
}

void
CertificateStore::afterFetcherSignalEmitted(const ndn::Data& lsaSegment)
{
  const auto keyName = lsaSegment.getSignature().getKeyLocator().getName();
  if (!find(keyName)) {
    NLSR_LOG_TRACE("Publishing certificate for: " << keyName);
    publishCertFromCache(keyName);
  }
  else {
    NLSR_LOG_TRACE("Certificate is already in the store: " << keyName);
  }
}

} // namespace security
} // namespace nlsr
