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

#ifndef NLSR_CERTIFICATE_STORE_HPP
#define NLSR_CERTIFICATE_STORE_HPP

#include "../common.hpp"
#include "../test-access-control.hpp"

#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/security/v2/certificate.hpp>

namespace nlsr {
namespace security {

/*! \brief Store certificates for names
 *
 * Stores certificates that this router claims to be authoritative
 * for. That is, this stores only the certificates that we will reply
 * to KEY interests with, e.g. when other routers are verifying data
 * we have sent.
 */
class CertificateStore
{
public:
  void
  insert(const ndn::security::v2::Certificate& certificate)
  {
    m_certificates[certificate.getKeyName()] = certificate;
  }

  const ndn::security::v2::Certificate*
  find(const ndn::Name keyName)
  {
    CertMap::iterator it = m_certificates.find(keyName);

    if (it != m_certificates.end()) {
      return &it->second;
    }

    return nullptr;
  }

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  void
  clear()
  {
    m_certificates.clear();
  }

private:
  typedef std::map<ndn::Name, ndn::security::v2::Certificate> CertMap;
  CertMap m_certificates;
};

} // namespace security
} // namespace nlsr

#endif // NLSR_CERTIFICATE_STORE_HPP
