/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2019,  The University of Memphis,
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
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/

#include "conf-parameter.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER(ConfParameter);

// To be changed when breaking changes are made to sync
const uint64_t ConfParameter::SYNC_VERSION = 6;

static std::unique_ptr<ndn::security::v2::CertificateFetcherDirectFetch>
makeCertificateFetcher(ndn::Face& face)
{
  auto fetcher = std::make_unique<ndn::security::v2::CertificateFetcherDirectFetch>(face);
  fetcher->setSendDirectInterestOnly(true);
  return fetcher;
}

ConfParameter::ConfParameter(ndn::Face& face, const std::string& confFileName)
  : m_confFileName(confFileName)
  , m_lsaRefreshTime(LSA_REFRESH_TIME_DEFAULT)
  , m_adjLsaBuildInterval(ADJ_LSA_BUILD_INTERVAL_DEFAULT)
  , m_firstHelloInterval(FIRST_HELLO_INTERVAL_DEFAULT)
  , m_routingCalcInterval(ROUTING_CALC_INTERVAL_DEFAULT)
  , m_faceDatasetFetchInterval(ndn::time::seconds(static_cast<int>(FACE_DATASET_FETCH_INTERVAL_DEFAULT)))
  , m_lsaInterestLifetime(ndn::time::seconds(static_cast<int>(LSA_INTEREST_LIFETIME_DEFAULT)))
  , m_routerDeadInterval(2 * LSA_REFRESH_TIME_DEFAULT)
  , m_interestRetryNumber(HELLO_RETRIES_DEFAULT)
  , m_interestResendTime(HELLO_TIMEOUT_DEFAULT)
  , m_infoInterestInterval(HELLO_INTERVAL_DEFAULT)
  , m_hyperbolicState(HYPERBOLIC_STATE_OFF)
  , m_corR(0)
  , m_maxFacesPerPrefix(MAX_FACES_PER_PREFIX_MIN)
  , m_syncInterestLifetime(ndn::time::milliseconds(SYNC_INTEREST_LIFETIME_DEFAULT))
  , m_syncProtocol(SYNC_PROTOCOL_CHRONOSYNC)
  , m_adjl()
  , m_npl()
  , m_validator(makeCertificateFetcher(face))
  , m_prefixUpdateValidator(std::make_unique<ndn::security::v2::CertificateFetcherDirectFetch>(face))
{
}

void
ConfParameter::writeLog()
{
  NLSR_LOG_INFO("Router Name: " << m_routerName);
  NLSR_LOG_INFO("Site Name: " << m_siteName);
  NLSR_LOG_INFO("Network: " << m_network);
  NLSR_LOG_INFO("Router Prefix: " << m_routerPrefix);
  NLSR_LOG_INFO("Sync Prefix: " << m_syncPrefix);
  NLSR_LOG_INFO("Sync LSA prefix: " << m_lsaPrefix);
  NLSR_LOG_INFO("Hello Interest retry number: " << m_interestRetryNumber);
  NLSR_LOG_INFO("Hello Interest resend second: " << m_interestResendTime);
  NLSR_LOG_INFO("Info Interest interval: " << m_infoInterestInterval);
  NLSR_LOG_INFO("LSA refresh time: " << m_lsaRefreshTime);
  NLSR_LOG_INFO("FIB Entry refresh time: " << m_lsaRefreshTime * 2);
  NLSR_LOG_INFO("LSA Interest lifetime: " << getLsaInterestLifetime());
  NLSR_LOG_INFO("Router dead interval: " << getRouterDeadInterval());
  NLSR_LOG_INFO("Max Faces Per Prefix: " << m_maxFacesPerPrefix);
  NLSR_LOG_INFO("Hyperbolic Routing: " << m_hyperbolicState);
  NLSR_LOG_INFO("Hyp R: " << m_corR);
  int i=0;
  for (auto const& value: m_corTheta) {
    NLSR_LOG_INFO("Hyp Angle " << i++ << ": "<< value);
  }
  NLSR_LOG_INFO("State Directory: " << m_stateFileDir);

  // Event Intervals
  NLSR_LOG_INFO("Adjacency LSA build interval:  " << m_adjLsaBuildInterval);
  NLSR_LOG_INFO("First Hello Interest interval: " << m_firstHelloInterval);
  NLSR_LOG_INFO("Routing calculation interval:  " << m_routingCalcInterval);
}

void
ConfParameter::setNetwork(const ndn::Name& networkName)
{
  m_network = networkName;

  m_syncPrefix.append("localhop");
  m_syncPrefix.append(m_network);
  m_syncPrefix.append("nlsr");
  m_syncPrefix.append("sync");
  m_syncPrefix.appendVersion(SYNC_VERSION);

  m_lsaPrefix.append("localhop");
  m_lsaPrefix.append(m_network);
  m_lsaPrefix.append("nlsr");
  m_lsaPrefix.append("LSA");
}

} // namespace nlsr
