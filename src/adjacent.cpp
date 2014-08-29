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
#include <iostream>
#include <string>
#include <cmath>
#include <limits>


#include "adjacent.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("Adjacent");

using namespace std;

const float Adjacent::DEFAULT_LINK_COST = 10.0;

Adjacent::Adjacent()
    : m_name()
    , m_connectingFaceUri()
    , m_linkCost(DEFAULT_LINK_COST)
    , m_status(STATUS_INACTIVE)
    , m_interestTimedOutNo(0)
    , m_faceId(0)
{
}

Adjacent::Adjacent(const ndn::Name& an)
    : m_name(an)
    , m_connectingFaceUri()
    , m_linkCost(DEFAULT_LINK_COST)
    , m_status(STATUS_INACTIVE)
    , m_interestTimedOutNo(0)
    , m_faceId(0)
  {
  }

Adjacent::Adjacent(const ndn::Name& an, const std::string& cfu,  double lc,
                   Status s, uint32_t iton, uint64_t faceId)
    : m_name(an)
    , m_connectingFaceUri(cfu)
    , m_linkCost(lc)
    , m_status(s)
    , m_interestTimedOutNo(iton)
    , m_faceId(faceId)
  {

  }

bool
Adjacent::operator==(const Adjacent& adjacent) const
{
  return (m_name == adjacent.getName()) &&
         (m_connectingFaceUri == adjacent.getConnectingFaceUri()) &&
         (std::abs(m_linkCost - adjacent.getLinkCost()) <
          std::numeric_limits<double>::epsilon()) ;
}

void
Adjacent::writeLog()
{
  _LOG_DEBUG("Adjacent : " << m_name);
  _LOG_DEBUG("Connecting FaceUri: " << m_connectingFaceUri);
  _LOG_DEBUG("Link Cost: " << m_linkCost);
  _LOG_DEBUG("Status: " << m_status);
  _LOG_DEBUG("Interest Timed out: " << m_interestTimedOutNo);
}

} //namespace nlsr
