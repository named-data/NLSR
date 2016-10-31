/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
 *                           Regents of the University of California
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

#include "validator.hpp"

namespace nlsr {

void
Validator::checkPolicy(const ndn::Data& data, int nSteps, const ndn::OnDataValidated& onValidated,
                       const ndn::OnDataValidationFailed& onValidationFailed,
                       std::vector<shared_ptr<ndn::ValidationRequest>>& nextSteps)
{
  if (!m_shouldValidate) {
    onValidated(data.shared_from_this());
  }
  else {
    ValidatorConfig::checkPolicy(data, nSteps, onValidated, onValidationFailed, nextSteps);
  }
}

void
Validator::afterCheckPolicy(const NextSteps& nextSteps, const OnFailure& onFailure)
{
  if (m_face == nullptr) {
    onFailure("Require more information to validate the packet!");
    return;
  }

  for (const std::shared_ptr<ndn::ValidationRequest>& request : nextSteps) {

    ndn::Interest& interest = request->m_interest;

    // Look for certificate in permanent storage
    std::shared_ptr<const ndn::IdentityCertificate> cert = m_certStore.find(interest.getName());

    if (cert != nullptr) {
      // If the certificate is found, no reason to express interest
      std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>(interest.getName());
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
                              std::bind(&Validator::onData, this, _1, _2, request),
                              std::bind(&Validator::onTimeout, // Nack
                                        this, _1, request->m_nRetries,
                                        onFailure,
                                        request),
                              std::bind(&Validator::onTimeout,
                                        this, _1, request->m_nRetries,
                                        onFailure,
                                        request));
    }
  }
}

std::shared_ptr<const ndn::Data>
Validator::preCertificateValidation(const ndn::Data& data)
{
  std::shared_ptr<ndn::Data> internalData = std::make_shared<ndn::Data>();
  internalData->wireDecode(data.getContent().blockFromValue());
  return internalData;
}

} // namespace nlsr
