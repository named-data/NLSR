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

#ifndef NLSR_VALIDATOR_HPP
#define NLSR_VALIDATOR_HPP

#include "common.hpp"
#include "security/certificate-store.hpp"

#include <ndn-cxx/security/validator-config.hpp>

namespace nlsr {

class Validator : public ndn::ValidatorConfig
{
public:
  class Error : public ndn::ValidatorConfig::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : ndn::ValidatorConfig::Error(what)
    {
    }
  };

  explicit
  Validator(ndn::Face& face,
            const ndn::Name broadcastPrefix,
            const ndn::shared_ptr<ndn::CertificateCache>& cache,
            security::CertificateStore& certStore,
            const int stepLimit = 10)
    : ndn::ValidatorConfig(face, cache, ndn::ValidatorConfig::DEFAULT_GRACE_INTERVAL, stepLimit)
    , m_broadcastPrefix(broadcastPrefix)
    , m_certStore(certStore)
  {
    m_broadcastPrefix.append("KEYS");
  }

  virtual
  ~Validator()
  {
  }

  const ndn::Name&
  getBroadcastPrefix()
  {
    return m_broadcastPrefix;
  }

  void
  setBroadcastPrefix(const ndn::Name& broadcastPrefix)
  {
    m_broadcastPrefix = broadcastPrefix;
  }

protected:
  typedef std::vector<ndn::shared_ptr<ndn::ValidationRequest>> NextSteps;

  virtual void
  afterCheckPolicy(const NextSteps& nextSteps,
                   const OnFailure& onFailure)
  {
    if (m_face == nullptr) {
      onFailure("Require more information to validate the packet!");
      return;
    }

    for (const shared_ptr<ndn::ValidationRequest>& request : nextSteps) {

      ndn::Interest& interest = request->m_interest;

      // Look for certificate in permanent storage
      shared_ptr<const ndn::IdentityCertificate> cert = m_certStore.find(interest.getName());

      if (cert != nullptr) {
        // If the certificate is found, no reason to express interest
        shared_ptr<ndn::Data> data = make_shared<ndn::Data>(interest.getName());
        data->setContent(cert->wireEncode());

        Validator::onData(interest, *data, request);
      }
      else {
        // Prepend broadcast prefix to interest name
        ndn::Name broadcastName = m_broadcastPrefix;
        broadcastName.append(interest.getName());
        interest.setName(broadcastName);

        // Attempt to fetch the certificate
        m_face->expressInterest(interest,
                                bind(&Validator::onData, this, _1, _2, request),
                                bind(&Validator::onTimeout,
                                     this, _1, request->m_nRetries,
                                     onFailure,
                                     request));
      }
    }
  }

  virtual ndn::shared_ptr<const ndn::Data>
  preCertificateValidation(const ndn::Data& data)
  {
    ndn::shared_ptr<ndn::Data> internalData = ndn::make_shared<ndn::Data>();
    internalData->wireDecode(data.getContent().blockFromValue());
    return internalData;
  }

private:
  ndn::Name m_broadcastPrefix;
  security::CertificateStore& m_certStore;
};

} // namespace nlsr

#endif // NLSR_VALIDATOR_HPP
