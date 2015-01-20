/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
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

#ifndef CONF_PARAMETER_HPP
#define CONF_PARAMETER_HPP

#include <iostream>
#include <boost/cstdint.hpp>
#include <ndn-cxx/common.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/util/time.hpp>

#include "logger.hpp"

namespace nlsr {

enum {
  LSA_REFRESH_TIME_MIN = 240,
  LSA_REFRESH_TIME_DEFAULT = 1800,
  LSA_REFRESH_TIME_MAX = 7200
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

enum {
  HYPERBOLIC_STATE_OFF = 0,
  HYPERBOLIC_STATE_ON = 1,
  HYPERBOLIC_STATE_DRY_RUN = 2,
  HYPERBOLIC_STATE_DEFAULT = 0
};

class ConfParameter
{

public:
  ConfParameter()
    : m_lsaRefreshTime(LSA_REFRESH_TIME_DEFAULT)
    , m_adjLsaBuildInterval(ADJ_LSA_BUILD_INTERVAL_DEFAULT)
    , m_firstHelloInterval(FIRST_HELLO_INTERVAL_DEFAULT)
    , m_routingCalcInterval(ROUTING_CALC_INTERVAL_DEFAULT)
    , m_lsaInterestLifetime(ndn::time::seconds(static_cast<int>(LSA_INTEREST_LIFETIME_DEFAULT)))
    , m_routerDeadInterval(2 * LSA_REFRESH_TIME_DEFAULT)
    , m_logLevel("INFO")
    , m_interestRetryNumber(HELLO_RETRIES_DEFAULT)
    , m_interestResendTime(HELLO_TIMEOUT_DEFAULT)
    , m_infoInterestInterval(HELLO_INTERVAL_DEFAULT)
    , m_hyperbolicState(HYPERBOLIC_STATE_OFF)
    , m_corR(0)
    , m_corTheta(0)
    , m_maxFacesPerPrefix(MAX_FACES_PER_PREFIX_MIN)
    , m_isLog4cxxConfAvailable(false)
  {
  }

  void
  setNetwork(const ndn::Name& networkName)
  {
    m_network = networkName;
    m_chronosyncPrefix = m_network;
    m_chronosyncPrefix.append("NLSR");
    m_chronosyncPrefix.append("sync");

    m_lsaPrefix = m_network;
    m_lsaPrefix.append("NLSR");
    m_lsaPrefix.append("LSA");
  }

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
  getChronosyncPrefix() const
  {
    return m_chronosyncPrefix;
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
  setLogLevel(const std::string& logLevel)
  {
    m_logLevel = logLevel;
  }

  const std::string&
  getLogLevel() const
  {
    return m_logLevel;
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
  setCorTheta(double ct)
  {
    m_corTheta = ct;
  }

  double
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
  setLogDir(const std::string& logDir)
  {
    m_logDir = logDir;
  }

  const std::string&
  getLogDir() const
  {
    return m_logDir;
  }

  void
  setSeqFileDir(const std::string& ssfd)
  {
    m_seqFileDir = ssfd;
  }

  const std::string&
  getSeqFileDir() const
  {
    return m_seqFileDir;
  }

  bool
  isLog4CxxConfAvailable() const
  {
    return m_isLog4cxxConfAvailable;
  }

  void
  setLog4CxxConfPath(const std::string& path)
  {
    m_log4CxxConfPath = path;
    m_isLog4cxxConfAvailable = true;
  }

  const std::string&
  getLog4CxxConfPath() const
  {
    return m_log4CxxConfPath;
  }

  void
  writeLog();

private:
  ndn::Name m_routerName;
  ndn::Name m_siteName;
  ndn::Name m_network;

  ndn::Name m_routerPrefix;
  ndn::Name m_lsaRouterPrefix;

  ndn::Name m_chronosyncPrefix;
  ndn::Name m_lsaPrefix;

  uint32_t  m_lsaRefreshTime;

  uint32_t m_adjLsaBuildInterval;
  uint32_t m_firstHelloInterval;
  uint32_t m_routingCalcInterval;

  ndn::time::seconds m_lsaInterestLifetime;
  uint32_t  m_routerDeadInterval;
  std::string m_logLevel;

  uint32_t m_interestRetryNumber;
  uint32_t m_interestResendTime;

  uint32_t m_infoInterestInterval;

  int32_t m_hyperbolicState;
  double m_corR;
  double m_corTheta;

  uint32_t m_maxFacesPerPrefix;

  std::string m_logDir;
  std::string m_seqFileDir;

  bool m_isLog4cxxConfAvailable;
  std::string m_log4CxxConfPath;
};

} // namespace nlsr

#endif //CONF_PARAMETER_HPP
