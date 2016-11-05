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
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/

#include "conf-parameter.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("ConfParameter");

void
ConfParameter::writeLog()
{
  _LOG_INFO("Router Name: " << m_routerName);
  _LOG_INFO("Site Name: " << m_siteName);
  _LOG_INFO("Network: " << m_network);
  _LOG_INFO("Router Prefix: " << m_routerPrefix);
  _LOG_INFO("ChronoSync sync Prefix: " << m_chronosyncPrefix);
  _LOG_INFO("ChronoSync LSA prefix: " << m_lsaPrefix);
  _LOG_INFO("Hello Interest retry number: " << m_interestRetryNumber);
  _LOG_INFO("Hello Interest resend second: " << m_interestResendTime);
  _LOG_INFO("Info Interest interval: " << m_infoInterestInterval);
  _LOG_INFO("LSA refresh time: " << m_lsaRefreshTime);
  _LOG_INFO("FIB Entry refresh time: " << m_lsaRefreshTime * 2);
  _LOG_INFO("LSA Interest lifetime: " << getLsaInterestLifetime());
  _LOG_INFO("Router dead interval: " << getRouterDeadInterval());
  _LOG_INFO("Max Faces Per Prefix: " << m_maxFacesPerPrefix);
  _LOG_INFO("Hyperbolic Routing: " << m_hyperbolicState);
  _LOG_INFO("Hyp R: " << m_corR);
  int i=0;
  for (auto const& value: m_corTheta) {
    _LOG_INFO("Hyp Angle " << i++ << ": "<< value);
  }
  _LOG_INFO("Log Directory: " << m_logDir);
  _LOG_INFO("Seq Directory: " << m_seqFileDir);

  // Event Intervals
  _LOG_INFO("Adjacency LSA build interval:  " << m_adjLsaBuildInterval);
  _LOG_INFO("First Hello Interest interval: " << m_firstHelloInterval);
  _LOG_INFO("Routing calculation interval:  " << m_routingCalcInterval);
}

} // namespace nlsr
