/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023,  The University of Memphis,
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

#ifndef NLSR_STATS_COLLECTOR_HPP
#define NLSR_STATS_COLLECTOR_HPP

#include "statistics.hpp"
#include "lsdb.hpp"
#include "hello-protocol.hpp"
#include <ndn-cxx/util/signal.hpp>

namespace nlsr {

/**
 * \brief a class designed to handle statistical signals in nlsr
 */
class StatsCollector
{
public:

  StatsCollector(Lsdb& lsdb, HelloProtocol& hp);

  ~StatsCollector();

  Statistics&
  getStatistics()
  {
    return m_stats;
  }

private:

  /*!
   * \brief: increments a Statistics::PacketType
   *
   * \param: pType is an enum value corresponding to a Statistics::PacketType
   *
   * This takes in a Statistics::PacketType emitted by a signal and increments
   * the value in m_stats.
   */
  void
  statsIncrement(Statistics::PacketType pType);

private:

  Lsdb& m_lsdb;
  HelloProtocol& m_hp;
  Statistics m_stats;

  ndn::signal::ScopedConnection m_lsaIncrementConn;
  ndn::signal::ScopedConnection m_helloIncrementConn;
};

} // namespace nlsr

#endif // NLSR_STATS_COLLECTOR_HPP
