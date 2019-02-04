/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2019,  The University of Memphis,
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

#ifndef NLSR_CONF_PARAMETER_HPP
#define NLSR_CONF_PARAMETER_HPP

#include "common.hpp"
#include "logger.hpp"
#include "test-access-control.hpp"
#include "adjacency-list.hpp"
#include "name-prefix-list.hpp"
#include "security/certificate-store.hpp"

#include <iostream>
#include <boost/cstdint.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/security/v2/certificate-fetcher-direct-fetch.hpp>
#include <ndn-cxx/util/time.hpp>

namespace nlsr {

enum {
  LSA_REFRESH_TIME_MIN = 240,
  LSA_REFRESH_TIME_DEFAULT = 1800,
  LSA_REFRESH_TIME_MAX = 7200
};

enum {
  SYNC_PROTOCOL_CHRONOSYNC = 0,
  SYNC_PROTOCOL_PSYNC = 1
};

enum {
  LSA_INTEREST_LIFETIME_MIN = 1,
  LSA_INTEREST_LIFETIME_DEFAULT = 4,
  LSA_INTEREST_LIFETIME_MAX = 60
};

enum {
  ADJ_LSA_BUILD_INTERVAL_MIN = 0,
  ADJ_LSA_BUILD_INTERVAL_DEFAULT = 5,
  ADJ_LSA_BUILD_INTERVAL_MAX = 5
};

enum {
  FIRST_HELLO_INTERVAL_MIN = 0,
  FIRST_HELLO_INTERVAL_DEFAULT = 10,
  FIRST_HELLO_INTERVAL_MAX = 10
};

enum {
  ROUTING_CALC_INTERVAL_MIN = 0,
  ROUTING_CALC_INTERVAL_DEFAULT = 15,
  ROUTING_CALC_INTERVAL_MAX = 15
};


enum {
  FACE_DATASET_FETCH_TRIES_MIN = 1,
  FACE_DATASET_FETCH_TRIES_MAX = 10,
  FACE_DATASET_FETCH_TRIES_DEFAULT = 3
};

enum {
  FACE_DATASET_FETCH_INTERVAL_MIN = 1800,
  FACE_DATASET_FETCH_INTERVAL_MAX = 5400,
  FACE_DATASET_FETCH_INTERVAL_DEFAULT = 3600
};

enum {
  HELLO_RETRIES_MIN = 1,
  HELLO_RETRIES_DEFAULT = 3,
  HELLO_RETRIES_MAX = 15
};

enum {
  HELLO_TIMEOUT_MIN = 1,
  HELLO_TIMEOUT_DEFAULT = 3,
  HELLO_TIMEOUT_MAX = 15
};

enum {
  HELLO_INTERVAL_MIN = 30,
  HELLO_INTERVAL_DEFAULT = 60,
  HELLO_INTERVAL_MAX =90
};

enum {
  MAX_FACES_PER_PREFIX_MIN = 0,
  MAX_FACES_PER_PREFIX_DEFAULT = 0,
  MAX_FACES_PER_PREFIX_MAX = 60
};

enum HyperbolicState {
  HYPERBOLIC_STATE_OFF = 0,
  HYPERBOLIC_STATE_ON = 1,
  HYPERBOLIC_STATE_DRY_RUN = 2,
  HYPERBOLIC_STATE_DEFAULT = 0
};

enum {
  SYNC_INTEREST_LIFETIME_MIN = 1000,
  SYNC_INTEREST_LIFETIME_DEFAULT = 60000,
  SYNC_INTEREST_LIFETIME_MAX = 120000,
};

/*! \brief A class to house all the configuration parameters for NLSR.
 *
 * This class is conceptually a singleton (but not mechanically) which
 * is just a collection of attributes that serve as a
 * separation-of-data for NLSR's configuration variables. NLSR refers
 * to an instance of this class for all its configuration
 * parameters. This object is typically populated by a
 * ConfFileProcessor reading a configuration file.
 *
 * \sa nlsr::ConfFileProcessor
 */
class ConfParameter
{

public:
  ConfParameter(ndn::Face& face, const std::string& confFileName = "nlsr.conf");

  const std::string&
  getConfFileName()
  {
    return m_confFileName;
  }

  void
  setNetwork(const ndn::Name& networkName);

  const ndn::Name&
  getNetwork() const
  {
    return m_network;
  }

  void
  setRouterName(const ndn::Name& routerName)
  {
    m_routerName = routerName;
  }

  const ndn::Name&
  getRouterName() const
  {
    return m_routerName;
  }

  void
  setSiteName(const ndn::Name& siteName)
  {
    m_siteName = siteName;
  }

  const ndn::Name&
  getSiteName() const
  {
    return m_siteName;
  }

  void
  buildRouterPrefix()
  {
    m_routerPrefix = m_network;
    m_routerPrefix.append(m_siteName);
    m_routerPrefix.append(m_routerName);
  }

  const ndn::Name&
  getRouterPrefix() const
  {
    return m_routerPrefix;
  }

  const ndn::Name&
  getSyncPrefix() const
  {
    return m_syncPrefix;
  }

  const ndn::Name&
  getLsaPrefix() const
  {
    return m_lsaPrefix;
  }

  void
  setLsaRefreshTime(uint32_t lrt)
  {
    m_lsaRefreshTime = lrt;
  }

  uint32_t
  getSyncProtocol() const
  {
    return m_syncProtocol;
  }

  void
  setSyncProtocol(int32_t syncProtocol)
  {
    if (syncProtocol == SYNC_PROTOCOL_CHRONOSYNC || syncProtocol == SYNC_PROTOCOL_PSYNC) {
      m_syncProtocol = syncProtocol;
    }
  }

  uint32_t
  getLsaRefreshTime() const
  {
    return m_lsaRefreshTime;
  }

  void
  setLsaInterestLifetime(const ndn::time::seconds& lifetime)
  {
    m_lsaInterestLifetime = lifetime;
  }

  const ndn::time::seconds&
  getLsaInterestLifetime() const
  {
    return m_lsaInterestLifetime;
  }

  void
  setAdjLsaBuildInterval(uint32_t interval)
  {
    m_adjLsaBuildInterval = interval;
  }

  uint32_t
  getAdjLsaBuildInterval() const
  {
    return m_adjLsaBuildInterval;
  }

  void
  setFirstHelloInterval(uint32_t interval)
  {
    m_firstHelloInterval = interval;
  }

  uint32_t
  getFirstHelloInterval() const
  {
    return m_firstHelloInterval;
  }

  void
  setRoutingCalcInterval(uint32_t interval)
  {
    m_routingCalcInterval = interval;
  }

  uint32_t
  getRoutingCalcInterval() const
  {
    return m_routingCalcInterval;
  }

  void
  setRouterDeadInterval(uint32_t rdt)
  {
    m_routerDeadInterval = rdt;
  }

  uint32_t
  getRouterDeadInterval() const
  {
    return m_routerDeadInterval;
  }

  void
  setFaceDatasetFetchTries(uint32_t count)
  {
    m_faceDatasetFetchTries = count;
  }

  uint32_t
  getFaceDatasetFetchTries() const
  {
    return m_faceDatasetFetchTries;
  }

  void
  setFaceDatasetFetchInterval(uint32_t interval)
  {
    m_faceDatasetFetchInterval = ndn::time::seconds(interval);
  }

  const ndn::time::seconds
  getFaceDatasetFetchInterval() const
  {
    return m_faceDatasetFetchInterval;
  }

  void
  setInterestRetryNumber(uint32_t irn)
  {
    m_interestRetryNumber = irn;
  }

  uint32_t
  getInterestRetryNumber() const
  {
    return m_interestRetryNumber;
  }

  void
  setInterestResendTime(uint32_t irt)
  {
    m_interestResendTime = irt;
  }

  uint32_t
  getInterestResendTime() const
  {
    return m_interestResendTime;
  }

  uint32_t
  getInfoInterestInterval() const
  {
    return m_infoInterestInterval;
  }

  void
  setInfoInterestInterval(uint32_t iii)
  {
    m_infoInterestInterval = iii;
  }

  void
  setHyperbolicState(int32_t ihc)
  {
    m_hyperbolicState = ihc;
  }

  int32_t
  getHyperbolicState() const
  {
    return m_hyperbolicState;
  }

  bool
  setCorR(double cr)
  {
    if ( cr >= 0 ) {
     m_corR = cr;
     return true;
    }
    return false;
  }

  double
  getCorR() const
  {
    return m_corR;
  }

  void
  setCorTheta(const std::vector<double>& ct)
  {
    m_corTheta = ct;
  }

  std::vector<double>
  getCorTheta() const
  {
    return m_corTheta;
  }

  void
  setMaxFacesPerPrefix(uint32_t mfpp)
  {
    m_maxFacesPerPrefix = mfpp;
  }

  uint32_t
  getMaxFacesPerPrefix() const
  {
    return m_maxFacesPerPrefix;
  }

  void
  setStateFileDir(const std::string& ssfd)
  {
    m_stateFileDir = ssfd;
  }

  const std::string&
  getStateFileDir() const
  {
    return m_stateFileDir;
  }

  void setConfFileNameDynamic(const std::string& confFileDynamic)
  {
    m_confFileNameDynamic = confFileDynamic;
  }

  const std::string&
  getConfFileNameDynamic() const
  {
    return m_confFileNameDynamic;
  }

  void
  setSyncInterestLifetime(uint32_t syncInterestLifetime)
  {
    m_syncInterestLifetime = ndn::time::milliseconds(syncInterestLifetime);
  }

  const ndn::time::milliseconds&
  getSyncInterestLifetime() const
  {
    return m_syncInterestLifetime;
  }

  AdjacencyList&
  getAdjacencyList()
  {
    return m_adjl;
  }

  NamePrefixList&
  getNamePrefixList()
  {
    return m_npl;
  }

  ndn::security::ValidatorConfig&
  getValidator()
  {
    return m_validator;
  }

  ndn::security::ValidatorConfig&
  getPrefixUpdateValidator()
  {
    return m_prefixUpdateValidator;
  }

  security::CertificateStore&
  getCertStore()
  {
    return m_certStore;
  }

  /*! \brief Dump the current state of all attributes to the log.
   */
  void
  writeLog();

private:
  std::string m_confFileName;
  ndn::Name m_routerName;
  ndn::Name m_siteName;
  ndn::Name m_network;

  ndn::Name m_routerPrefix;
  ndn::Name m_lsaRouterPrefix;

  ndn::Name m_syncPrefix;
  ndn::Name m_lsaPrefix;

  uint32_t  m_lsaRefreshTime;

  uint32_t m_adjLsaBuildInterval;
  uint32_t m_firstHelloInterval;
  uint32_t m_routingCalcInterval;

  uint32_t m_faceDatasetFetchTries;
  ndn::time::seconds m_faceDatasetFetchInterval;

  ndn::time::seconds m_lsaInterestLifetime;
  uint32_t  m_routerDeadInterval;

  uint32_t m_interestRetryNumber;
  uint32_t m_interestResendTime;

  uint32_t m_infoInterestInterval;

  int32_t m_hyperbolicState;
  double m_corR;
  std::vector<double> m_corTheta;

  uint32_t m_maxFacesPerPrefix;

  std::string m_stateFileDir;

  ndn::time::milliseconds m_syncInterestLifetime;

  int32_t m_syncProtocol;

  std::string m_confFileNameDynamic;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  static const uint64_t SYNC_VERSION;

  AdjacencyList m_adjl;
  NamePrefixList m_npl;
  ndn::security::ValidatorConfig m_validator;
  ndn::security::ValidatorConfig m_prefixUpdateValidator;
  security::CertificateStore m_certStore;
};

} // namespace nlsr

#endif // NLSR_CONF_PARAMETER_HPP
