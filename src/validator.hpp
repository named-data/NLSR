/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  The University of Memphis,
 *                     Regents of the University of California
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
 *
 * @author Yingdi Yu <yingdi@cs.ucla.edu>
 **/

#ifndef NLSR_VALIDATOR_HPP
#define NLSR_VALIDATOR_HPP

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
            const int stepLimit = 10)
    : ndn::ValidatorConfig(face, cache, ndn::ValidatorConfig::DEFAULT_GRACE_INTERVAL, stepLimit)
    , m_broadcastPrefix(broadcastPrefix)
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
  typedef std::vector<ndn::shared_ptr<ndn::ValidationRequest> > NextSteps;

  virtual void
  checkPolicy(const ndn::Data& data,
              int nSteps,
              const ndn::OnDataValidated& onValidated,
              const ndn::OnDataValidationFailed& onValidationFailed,
              NextSteps& nextSteps)
  {
    ndn::ValidatorConfig::checkPolicy(data, nSteps,
                                      onValidated, onValidationFailed,
                                      nextSteps);

    for (NextSteps::iterator it = nextSteps.begin(); it != nextSteps.end(); it++)
      {
        ndn::Name broadcastName = m_broadcastPrefix;
        broadcastName.append((*it)->m_interest.getName());

        (*it)->m_interest.setName(broadcastName);
      }
  }

  virtual void
  checkPolicy(const ndn::Interest& interest,
              int nSteps,
              const ndn::OnInterestValidated& onValidated,
              const ndn::OnInterestValidationFailed& onValidationFailed,
              NextSteps& nextSteps)
  {
    ndn::ValidatorConfig::checkPolicy(interest, nSteps,
                                      onValidated, onValidationFailed,
                                      nextSteps);

    for (NextSteps::iterator it = nextSteps.begin(); it != nextSteps.end(); it++)
      {
        ndn::Name broadcastName = m_broadcastPrefix;
        broadcastName.append((*it)->m_interest.getName());

        (*it)->m_interest.setName(broadcastName);
      }

  }

  void
  onData(const ndn::Interest& interest,
         const ndn::Data& data,
         const ndn::shared_ptr<ndn::ValidationRequest>& nextStep)
  {

    ndn::shared_ptr<ndn::Data> internalData = ndn::make_shared<ndn::Data>();
    internalData->wireDecode(data.getContent().blockFromValue());
    validate(*internalData,
             nextStep->m_onValidated, nextStep->m_onDataValidated, nextStep->m_nSteps);
  }

private:
  ndn::Name m_broadcastPrefix;
};


} // namespace nlsr

#endif // NLSR_VALIDATOR_HPP
