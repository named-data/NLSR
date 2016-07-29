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

#include "tlv/name-lsa.hpp"

#include "../boost-test.hpp"

namespace nlsr {
namespace tlv  {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestNameLsa)

const uint8_t NameLsaWithNamesData[] =
{
  // Header
  0x89, 0x25,
  // LsaInfo
  0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
  0x80, 0x8b, 0x02, 0x27, 0x10,
  // Name
  0x07, 0x07, 0x08, 0x05, 0x74, 0x65, 0x73, 0x74, 0x31,
  // Name
  0x07, 0x07, 0x08, 0x05, 0x74, 0x65, 0x73, 0x74, 0x32
};

const uint8_t NameLsaWithoutNamesData[] =
{
  // Header
  0x89, 0x13,
  // LsaInfo
  0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
  0x80, 0x8b, 0x02, 0x27, 0x10,
};

BOOST_AUTO_TEST_CASE(NameLsaEncodeWithNames)
{
  NameLsa nameLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  nameLsa.setLsaInfo(lsaInfo);

  nameLsa.addName("test1");
  nameLsa.addName("test2");

  const ndn::Block& wire = nameLsa.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(NameLsaWithNamesData,
                                  NameLsaWithNamesData + sizeof(NameLsaWithNamesData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(NameLsaDecodeWithNames)
{
  NameLsa nameLsa;

  nameLsa.wireDecode(ndn::Block(NameLsaWithNamesData, sizeof(NameLsaWithNamesData)));

  LsaInfo lsaInfo = nameLsa.getLsaInfo();
  BOOST_CHECK_EQUAL(lsaInfo.getOriginRouter(), "test");
  BOOST_CHECK_EQUAL(lsaInfo.getSequenceNumber(), 128);
  BOOST_CHECK_EQUAL(lsaInfo.getExpirationPeriod(), ndn::time::milliseconds(10000));

  BOOST_CHECK_EQUAL(nameLsa.hasNames(), true);
  std::list<ndn::Name> names = nameLsa.getNames();
  std::list<ndn::Name>::const_iterator it = names.begin();
  BOOST_CHECK_EQUAL(*it, "test1");

  it++;
  BOOST_CHECK_EQUAL(*it, "test2");
}

BOOST_AUTO_TEST_CASE(NameLsaEncodeWithoutNames)
{
  NameLsa nameLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  nameLsa.setLsaInfo(lsaInfo);

  const ndn::Block& wire = nameLsa.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(NameLsaWithoutNamesData,
                                  NameLsaWithoutNamesData + sizeof(NameLsaWithoutNamesData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(NameLsaDecodeWithoutNames)
{
  NameLsa nameLsa;

  nameLsa.wireDecode(ndn::Block(NameLsaWithoutNamesData, sizeof(NameLsaWithoutNamesData)));

  LsaInfo lsaInfo = nameLsa.getLsaInfo();
  BOOST_CHECK_EQUAL(lsaInfo.getOriginRouter(), "test");
  BOOST_CHECK_EQUAL(lsaInfo.getSequenceNumber(), 128);
  BOOST_CHECK_EQUAL(lsaInfo.getExpirationPeriod(), ndn::time::milliseconds(10000));

  BOOST_CHECK_EQUAL(nameLsa.hasNames(), false);
}

BOOST_AUTO_TEST_CASE(NameLsaClear)
{
  NameLsa nameLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  nameLsa.setLsaInfo(lsaInfo);

  nameLsa.addName("test1");
  BOOST_CHECK_EQUAL(nameLsa.getNames().size(), 1);

  std::list<ndn::Name> names = nameLsa.getNames();
  std::list<ndn::Name>::const_iterator it = names.begin();
  BOOST_CHECK_EQUAL(*it, "test1");

  nameLsa.clearNames();
  BOOST_CHECK_EQUAL(nameLsa.getNames().size(), 0);

  nameLsa.addName("test2");
  BOOST_CHECK_EQUAL(nameLsa.getNames().size(), 1);

  names = nameLsa.getNames();
  it = names.begin();
  BOOST_CHECK_EQUAL(*it, "test2");
}

BOOST_AUTO_TEST_CASE(AdjacencyLsaOutputStream)
{
  NameLsa nameLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  nameLsa.setLsaInfo(lsaInfo);

  nameLsa.addName("test1");
  nameLsa.addName("test2");

  std::ostringstream os;
  os << nameLsa;

  BOOST_CHECK_EQUAL(os.str(), "NameLsa("
                                "LsaInfo("
                                  "OriginRouter: /test, "
                                  "SequenceNumber: 128, "
                                  "ExpirationPeriod: 10000 milliseconds), "
                                "Name: /test1, "
                                "Name: /test2)");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
