/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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

#ifndef NLSR_CERTIFICATE_STORE_HPP
#define NLSR_CERTIFICATE_STORE_HPP

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/security/certificate.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/util/signal/scoped-connection.hpp>

namespace nlsr {

class ConfParameter;
class Lsdb;

namespace security {

/*! \brief Store certificates for names.
 *
 * Stores certificates that this router claims to be authoritative
 * for. That is, this stores only the certificates that we will reply
 * to KEY interests with, e.g. when other routers are verifying data
 * we have sent.
 */
class CertificateStore
{
public:
  CertificateStore(ndn::Face& face, ConfParameter& confParam, Lsdb& lsdb);

  void
  insert(const ndn::security::Certificate& certificate);

  /*!
   * \brief Find a certificate
   * \param name Either key name or certificate name.
   *
   * Find a certificate that NLSR has. First it checks against the
   * certificates this NLSR claims to be authoritative for, usually
   * something like this specific router's certificate, and then
   * checks the cache of certificates it has already fetched. If none
   * can be found, it will return an null pointer.
   */
  const ndn::security::Certificate*
  find(const ndn::Name& name) const;

  /*! \brief Retrieves the chain of certificates from Validator's cache and
   *   store them in Nlsr's own CertificateStore.
   * \param keyName Name of the first key in the certificate chain.
  */
  void
  publishCertFromCache(const ndn::Name& keyName);

private:
  const ndn::security::Certificate*
  findByKeyName(const ndn::Name& keyName) const;

  const ndn::security::Certificate*
  findByCertName(const ndn::Name& certName) const;

  void
  setInterestFilter(const ndn::Name& prefix);

  void
  registerKeyPrefixes();

  void
  onKeyInterest(const ndn::Name& name, const ndn::Interest& interest);

  void
  onKeyPrefixRegSuccess(const ndn::Name& name);

  void
  registrationFailed(const ndn::Name& name);

private:
  std::map<ndn::Name, ndn::security::Certificate> m_certificates;
  ndn::Face& m_face;
  ConfParameter& m_confParam;
  ndn::security::ValidatorConfig& m_validator;
  ndn::signal::ScopedConnection m_afterSegmentValidatedConn;
};

} // namespace security
} // namespace nlsr

#endif // NLSR_CERTIFICATE_STORE_HPP
