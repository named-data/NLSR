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

#include "tlv/lsa-info.hpp"

#include "../boost-test.hpp"

namespace nlsr {
namespace tlv  {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestLsaInfo)

const uint8_t LsaInfoData[] =
{
  // Header
  0x80, 0x21,
  // OriginRouter
  0x81, 0x18, 0x07, 0x16, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x08, 0x03, 0x6c, 0x73,
  0x61, 0x08, 0x04, 0x69, 0x6e, 0x66, 0x6f, 0x08, 0x03, 0x74, 0x6c, 0x76,
  // SequenceNumber
  0x82, 0x01, 0x80,
  // ExpirationPeriod
  0x8b, 0x02, 0x27, 0x10
};

const uint8_t LsaInfoDataInfiniteExpirationPeriod[] =
{
  // Header
  0x80, 0x1d,
  // OriginRouter
  0x81, 0x18, 0x07, 0x16, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x08, 0x03, 0x6c, 0x73,
  0x61, 0x08, 0x04, 0x69, 0x6e, 0x66, 0x6f, 0x08, 0x03, 0x74, 0x6c, 0x76,
  // SequenceNumber
  0x82, 0x1, 0x80
};

BOOST_AUTO_TEST_CASE(LsaInfoEncode)
{
  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("/test/lsa/info/tlv");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));

  const ndn::Block& wire = lsaInfo.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(LsaInfoData,
                                  LsaInfoData + sizeof(LsaInfoData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(LsaInfoDecode)
{
  LsaInfo lsaInfo;

  lsaInfo.wireDecode(ndn::Block(LsaInfoData, sizeof(LsaInfoData)));

  ndn::Name originRouter("/test/lsa/info/tlv");
  BOOST_REQUIRE_EQUAL(lsaInfo.getOriginRouter(), originRouter);
  BOOST_REQUIRE_EQUAL(lsaInfo.getSequenceNumber(), 128);
  BOOST_REQUIRE_EQUAL(lsaInfo.getExpirationPeriod(), ndn::time::milliseconds(10000));
  BOOST_REQUIRE_EQUAL(lsaInfo.hasInfiniteExpirationPeriod(), false);
}

BOOST_AUTO_TEST_CASE(LsaInfoInfiniteExpirationPeriodEncode)
{
  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("/test/lsa/info/tlv");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(LsaInfo::INFINITE_EXPIRATION_PERIOD);

  const ndn::Block& wire = lsaInfo.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(LsaInfoDataInfiniteExpirationPeriod,
                                  LsaInfoDataInfiniteExpirationPeriod +
                                    sizeof(LsaInfoDataInfiniteExpirationPeriod),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(LsaInfoInfiniteExpirationPeriodDecode)
{
  LsaInfo lsaInfo;

  lsaInfo.wireDecode(ndn::Block(LsaInfoDataInfiniteExpirationPeriod,
                                sizeof(LsaInfoDataInfiniteExpirationPeriod)));

  ndn::Name originRouter("/test/lsa/info/tlv");
  BOOST_REQUIRE_EQUAL(lsaInfo.getOriginRouter(), originRouter);
  BOOST_REQUIRE_EQUAL(lsaInfo.getSequenceNumber(), 128);
  BOOST_REQUIRE_EQUAL(lsaInfo.getExpirationPeriod(), LsaInfo::INFINITE_EXPIRATION_PERIOD);
  BOOST_REQUIRE_EQUAL(lsaInfo.hasInfiniteExpirationPeriod(), true);
}

BOOST_AUTO_TEST_CASE(LsaInfoOutputStream)
{
  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("/test/lsa/info/tlv");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));

  std::ostringstream os;
  os << lsaInfo;

  BOOST_CHECK_EQUAL(os.str(), "LsaInfo(OriginRouter: /test/lsa/info/tlv, SequenceNumber: 128, "
                              "ExpirationPeriod: 10000 milliseconds)");
}

BOOST_AUTO_TEST_CASE(LsaInfoMake)
{
  Lsa lsa("lsa-type");
  lsa.setOrigRouter("/test/lsa/info/tlv");
  lsa.setLsSeqNo(128);
  lsa.setExpirationTimePoint(ndn::time::system_clock::now());

  std::shared_ptr<LsaInfo> lsaInfo = makeLsaInfo(lsa);
  BOOST_CHECK_EQUAL(lsaInfo->getOriginRouter(), lsa.getOrigRouter());
  BOOST_CHECK_EQUAL(lsaInfo->getSequenceNumber(), lsa.getLsSeqNo());
  BOOST_CHECK_LE(lsaInfo->getExpirationPeriod(), ndn::time::milliseconds(0));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
