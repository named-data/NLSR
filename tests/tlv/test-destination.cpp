/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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

#include "tlv/destination.hpp"

#include "../boost-test.hpp"

namespace nlsr {
namespace tlv {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestDes)

const uint8_t DesData[] =
{
  // Header
  0x8e, 0x13,
  // Routername 746573742f646573742f746c76
  0x07, 0x11, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x08, 0x04, 0x64, 0x65, 0x73, 0x74,
  0x08, 0x03, 0x74, 0x6c, 0x76
};

BOOST_AUTO_TEST_CASE(DesEncode)
{
  Destination des1;
  des1.setName("/test/dest/tlv");

  const ndn::Block& wire = des1.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(DesData,
                                  DesData + sizeof(DesData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(DesDecode)
{
  Destination des1;

  des1.wireDecode(ndn::Block(DesData, sizeof(DesData)));

  ndn::Name DEST_NAME = ndn::Name("/test/dest/tlv");
  BOOST_REQUIRE_EQUAL(des1.getName(), DEST_NAME);
}

BOOST_AUTO_TEST_CASE(DesOutputStream)
{
  Destination des1;
  des1.setName("/test/dest/tlv");

  std::ostringstream os;
  os << des1;

  BOOST_CHECK_EQUAL(os.str(), "Destination: /test/dest/tlv");
}

BOOST_AUTO_TEST_CASE(DesMake)
{
  RoutingTableEntry rte("/test/dest/tlv");

  std::shared_ptr<Destination> des = makeDes(rte);
  BOOST_CHECK_EQUAL(des->getName(), rte.getDestination());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
