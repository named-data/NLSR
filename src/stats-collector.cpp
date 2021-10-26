/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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
 */

#include "stats-collector.hpp"
#include "logger.hpp"

namespace nlsr {

StatsCollector::StatsCollector(Lsdb& lsdb, HelloProtocol& hp)
  : m_lsdb(lsdb)
  , m_hp(hp)
{
  m_lsaIncrementConn = m_lsdb.lsaIncrementSignal.connect(std::bind(&StatsCollector::statsIncrement,
                                                                   this, _1));
  m_helloIncrementConn = m_hp.hpIncrementSignal.connect(std::bind(&StatsCollector::statsIncrement,
                                                                  this, _1));
}

StatsCollector::~StatsCollector()
{
  m_lsaIncrementConn.disconnect();
  m_helloIncrementConn.disconnect();
}

void
StatsCollector::statsIncrement(Statistics::PacketType pType)
{
  m_stats.increment(pType);
}

} // namespace nlsr
