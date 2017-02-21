/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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
#include "face-map.hpp"
#include "common.hpp"
#include "logger.hpp"

#include <iostream>
#include <utility>

namespace nlsr {

INIT_LOGGER(route.FaceMap);

void
FaceMap::writeLog()
{
  NLSR_LOG_DEBUG("------- Face Map-----------");
  for (const auto& it : m_table) {
    NLSR_LOG_DEBUG("Face Map Entry (FaceUri: " << (it.second).getFaceUri()
               << " Face Id: " << (it.second).getFaceId() << ")");
  }
}

} // namespace nlsr
