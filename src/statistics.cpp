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

#include "statistics.hpp"
#include "nlsr.hpp"
#include "utility/name-helper.hpp"

namespace nlsr {

size_t
Statistics::get(PacketType type) const
{
  auto it = m_packetCounter.find(type);
  if (it != m_packetCounter.end()) {
    return it->second;
  }
  else {
    return 0;
  }
}

void
Statistics::increment(PacketType type)
{
  m_packetCounter[type]++;
}

void
Statistics::resetAll()
{
  for (auto& it : m_packetCounter) {
    it.second = 0;
  }
}

std::ostream&
operator<<(std::ostream& os, const Statistics& stats)
{
  using PacketType = Statistics::PacketType;

  os << "++++++++++++++++++++++++++++++++++++++++\n"
     << "+                                      +\n"
     << "+              Statistics              +\n"
     << "+                                      +\n"
     << "++++++++++++++++++++++++++++++++++++++++\n"
     << "HELLO PROTOCOL\n"
     << "    Sent Hello Interests: "              << stats.get(PacketType::SENT_HELLO_INTEREST) << "\n"
     << "    Sent Hello Data: "                   << stats.get(PacketType::SENT_HELLO_DATA) << "\n"
     << "\n"
     << "    Received Hello Interests: "          << stats.get(PacketType::RCV_HELLO_INTEREST) << "\n"
     << "    Received Hello Data: "               << stats.get(PacketType::RCV_HELLO_DATA) << "\n"
     << "\n"
     << "LSDB\n"
     << "    Total Sent LSA Interests: "          << stats.get(PacketType::SENT_LSA_INTEREST) << "\n"
     << "    Total Received LSA Interests: "      << stats.get(PacketType::RCV_LSA_INTEREST) << "\n"
     << "\n"
     << "    Total Sent LSA Data: "               << stats.get(PacketType::SENT_LSA_DATA) << "\n"
     << "    Total Received LSA Data: "           << stats.get(PacketType::RCV_LSA_DATA) << "\n"
     << "\n"
     << "    Sent Adjacency LSA Interests: "      << stats.get(PacketType::SENT_ADJ_LSA_INTEREST) << "\n"
     << "    Sent Coordinate LSA Interests: "     << stats.get(PacketType::SENT_COORD_LSA_INTEREST) << "\n"
     << "    Sent Name LSA Interests: "           << stats.get(PacketType::SENT_NAME_LSA_INTEREST) << "\n"
     << "\n"
     << "    Received Adjacency LSA Interests: "  << stats.get(PacketType::RCV_ADJ_LSA_INTEREST) << "\n"
     << "    Received Coordinate LSA Interests: " << stats.get(PacketType::RCV_COORD_LSA_INTEREST) << "\n"
     << "    Received Name LSA Interests: "       << stats.get(PacketType::RCV_NAME_LSA_INTEREST) << "\n"
     << "\n"
     << "    Sent Adjacency LSA Data: "           << stats.get(PacketType::SENT_ADJ_LSA_DATA) << "\n"
     << "    Sent Coordinate LSA Data: "          << stats.get(PacketType::SENT_COORD_LSA_DATA) << "\n"
     << "    Sent Name LSA Data: "                << stats.get(PacketType::SENT_NAME_LSA_DATA) << "\n"
     << "\n"
     << "    Received Adjacency LSA Data: "       << stats.get(PacketType::RCV_ADJ_LSA_DATA) << "\n"
     << "    Received Coordinate LSA Data: "      << stats.get(PacketType::RCV_COORD_LSA_DATA) << "\n"
     << "    Received Name LSA Data: "            << stats.get(PacketType::RCV_NAME_LSA_DATA) << "\n"
     << "++++++++++++++++++++++++++++++++++++++++\n";

  return os;
}

} // namespace nlsr
