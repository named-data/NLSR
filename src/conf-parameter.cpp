/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
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
  _LOG_DEBUG("Router Name: " << m_routerName);
  _LOG_DEBUG("Site Name: " << m_siteName);
  _LOG_DEBUG("Network: " << m_network);
  _LOG_DEBUG("Router Prefix: " << m_routerPrefix);
  _LOG_DEBUG("ChronoSync sync Prefix: " << m_chronosyncPrefix);
  _LOG_DEBUG("ChronoSync LSA prefix: " << m_lsaPrefix);
  _LOG_DEBUG("Hello Interest retry number: " << m_interestRetryNumber);
  _LOG_DEBUG("Hello Interest resend second: " << m_interestResendTime);
  _LOG_DEBUG("Info Interest interval: " << m_infoInterestInterval);
  _LOG_DEBUG("LSA refresh time: " << m_lsaRefreshTime);
  _LOG_DEBUG("LSA Interest lifetime: " << getLsaInterestLifetime());
  _LOG_DEBUG("Router dead interval: " << getRouterDeadInterval());
  _LOG_DEBUG("Max Faces Per Prefix: " << m_maxFacesPerPrefix);
  _LOG_DEBUG("Hyperbolic Routing: " << m_hyperbolicState);
  _LOG_DEBUG("Hyp R: " << m_corR);
  _LOG_DEBUG("Hyp theta: " << m_corTheta);
  _LOG_DEBUG("Log Directory: " << m_logDir);
  _LOG_DEBUG("Seq Directory: " << m_seqFileDir);

  // Event Intervals
  _LOG_DEBUG("Adjacency LSA build interval:  " << m_adjLsaBuildInterval);
  _LOG_DEBUG("First Hello Interest interval: " << m_firstHelloInterval);
  _LOG_DEBUG("Routing calculation interval:  " << m_routingCalcInterval);
}

} // namespace nlsr
