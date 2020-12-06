/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  The University of Memphis,
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

#ifndef NLSR_STATISTICS_HPP
#define NLSR_STATISTICS_HPP

#include <map>
#include <ostream>

namespace nlsr {

class Statistics
{
public:
  enum class PacketType {
    SENT_HELLO_INTEREST,
    SENT_HELLO_DATA,
    RCV_HELLO_INTEREST,
    RCV_HELLO_DATA,
    SENT_LSA_INTEREST,
    SENT_ADJ_LSA_INTEREST,
    SENT_COORD_LSA_INTEREST,
    SENT_NAME_LSA_INTEREST,
    SENT_LSA_DATA,
    SENT_ADJ_LSA_DATA,
    SENT_COORD_LSA_DATA,
    SENT_NAME_LSA_DATA,
    RCV_LSA_INTEREST,
    RCV_ADJ_LSA_INTEREST,
    RCV_COORD_LSA_INTEREST,
    RCV_NAME_LSA_INTEREST,
    RCV_LSA_DATA,
    RCV_ADJ_LSA_DATA,
    RCV_COORD_LSA_DATA,
    RCV_NAME_LSA_DATA
  };

  size_t
  get(PacketType) const;

  void
  increment(PacketType);

  void
  resetAll();

  const std::map<PacketType,int>&
  getCounter() const
  {
    return m_packetCounter;
  }

private:
  std::map<PacketType, int> m_packetCounter;
};

std::ostream&
operator<<(std::ostream&, const Statistics& stats);

} // namespace nlsr

#endif // NLSR_STATISTICS_HPP
