/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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

#include "tlv/adjacency.hpp"

#include "../boost-test.hpp"

namespace nlsr {
namespace tlv  {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestAdjacency)

const uint8_t AdjacencyData[] =
{
  // Header
  0x84, 0x30,
  // Name
  0x07, 0x16, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x08, 0x09, 0x61, 0x64, 0x6a, 0x61,
  0x63, 0x65, 0x6e, 0x63, 0x79, 0x08, 0x03, 0x74, 0x6c, 0x76,
  // Uri
  0x8d, 0x13, 0x2f, 0x74, 0x65, 0x73, 0x74, 0x2f, 0x61, 0x64, 0x6a, 0x61, 0x63, 0x65,
  0x6e, 0x63, 0x79, 0x2f, 0x74, 0x6c, 0x76,
  // Cost
  0x8c, 0x01, 0x80
};

BOOST_AUTO_TEST_CASE(AdjacencyEncode)
{
  Adjacency adjacency;
  adjacency.setName("/test/adjacency/tlv");
  adjacency.setUri("/test/adjacency/tlv");
  adjacency.setCost(128);

  const ndn::Block& wire = adjacency.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(AdjacencyData,
                                  AdjacencyData + sizeof(AdjacencyData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(AdjacencyDecode)
{
  Adjacency adjacency;

  adjacency.wireDecode(ndn::Block(AdjacencyData, sizeof(AdjacencyData)));

  ndn::Name name("/test/adjacency/tlv");
  BOOST_REQUIRE_EQUAL(adjacency.getName(), name);
  BOOST_REQUIRE_EQUAL(adjacency.getUri(), "/test/adjacency/tlv");
  BOOST_REQUIRE_EQUAL(adjacency.getCost(), 128);
}

BOOST_AUTO_TEST_CASE(AdjacencyOutputStream)
{
  Adjacency adjacency;
  adjacency.setName("/test/adjacency/tlv");
  adjacency.setUri("/test/adjacency/tlv");
  adjacency.setCost(128);

  std::ostringstream os;
  os << adjacency;

  BOOST_CHECK_EQUAL(os.str(), "Adjacency(Name: /test/adjacency/tlv, "
                              "Uri: /test/adjacency/tlv, "
                              "Cost: 128)");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
