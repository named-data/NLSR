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

#include "tlv/coordinate-lsa.hpp"

#include "../boost-test.hpp"

namespace nlsr {
namespace tlv  {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestCoordinateLsa)

const uint8_t CoordinateLsaData[] =
{
  // Header
  0x85, 0x2b,
  // LsaInfo
  0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
  0x80, 0x8b, 0x02, 0x27, 0x10,
  // HyperbolicRadius
  0x87, 0x0a, 0x86, 0x08, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xfa, 0x3f,
  // HyperbolicAngle
  0x88, 0x0a, 0x86, 0x08, 0x7b, 0x14, 0xae, 0x47, 0xe1, 0x7a, 0xfc, 0x3f
};

BOOST_AUTO_TEST_CASE(CoordinateLsaEncode)
{
  CoordinateLsa coordinateLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  coordinateLsa.setLsaInfo(lsaInfo);

  coordinateLsa.setHyperbolicRadius(1.65);
  coordinateLsa.setHyperbolicAngle(1.78);

  const ndn::Block& wire = coordinateLsa.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(CoordinateLsaData,
                                  CoordinateLsaData + sizeof(CoordinateLsaData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(CoordinateLsaDecode)
{
  CoordinateLsa coordinateLsa;

  coordinateLsa.wireDecode(ndn::Block(CoordinateLsaData, sizeof(CoordinateLsaData)));

  BOOST_REQUIRE_EQUAL(coordinateLsa.getLsaInfo().getOriginRouter(), "test");
  BOOST_REQUIRE_EQUAL(coordinateLsa.getLsaInfo().getSequenceNumber(), 128);
  BOOST_REQUIRE_EQUAL(coordinateLsa.getLsaInfo().getExpirationPeriod(),
                      ndn::time::milliseconds(10000));
  BOOST_REQUIRE_EQUAL(coordinateLsa.getHyperbolicRadius(), 1.65);
  BOOST_REQUIRE_EQUAL(coordinateLsa.getHyperbolicAngle(), 1.78);
}

BOOST_AUTO_TEST_CASE(CoordinateLsaOutputStream)
{
  CoordinateLsa coordinateLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  coordinateLsa.setLsaInfo(lsaInfo);

  coordinateLsa.setHyperbolicRadius(1.65);
  coordinateLsa.setHyperbolicAngle(1.78);

  std::ostringstream os;
  os << coordinateLsa;

  BOOST_CHECK_EQUAL(os.str(), "CoordinateLsa("
                                "LsaInfo(OriginRouter: /test, "
                                        "SequenceNumber: 128, "
                                        "ExpirationPeriod: 10000 milliseconds), "
                                "HyperbolicRadius: 1.65, "
                                "HyperbolicAngle: 1.78)");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
