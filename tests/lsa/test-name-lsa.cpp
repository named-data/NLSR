/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023,  The University of Memphis,
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
 */

#include "lsa/name-lsa.hpp"

#include "tests/boost-test.hpp"

namespace nlsr::test {

BOOST_AUTO_TEST_SUITE(TestNameLsa)

const uint8_t NAME_LSA1[] = {
  0x89, 0x37, 0x80, 0x23, 0x07, 0x09, 0x08, 0x07, 0x72, 0x6F, 0x75, 0x74, 0x65, 0x72, 0x31,
  0x82, 0x01, 0x0C, 0x8B, 0x13, 0x32, 0x30, 0x32, 0x30, 0x2D, 0x30, 0x33, 0x2D, 0x32, 0x36,
  0x20, 0x30, 0x34, 0x3A, 0x31, 0x33, 0x3A, 0x33, 0x34, 0x07, 0x07, 0x08, 0x05, 0x6E, 0x61,
  0x6D, 0x65, 0x31, 0x07, 0x07, 0x08, 0x05, 0x6E, 0x61, 0x6D, 0x65, 0x32
};

const uint8_t NAME_LSA_EXTRA_NAME[] = {
  0x89, 0x40, 0x80, 0x23, 0x07, 0x09, 0x08, 0x07, 0x72, 0x6F, 0x75, 0x74, 0x65, 0x72, 0x31,
  0x82, 0x01, 0x0C, 0x8B, 0x13, 0x32, 0x30, 0x32, 0x30, 0x2D, 0x30, 0x33, 0x2D, 0x32, 0x36,
  0x20, 0x30, 0x34, 0x3A, 0x31, 0x33, 0x3A, 0x33, 0x34, 0x07, 0x07, 0x08, 0x05, 0x6E, 0x61,
  0x6D, 0x65, 0x31, 0x07, 0x07, 0x08, 0x05, 0x6E, 0x61, 0x6D, 0x65, 0x32, 0x07, 0x07, 0x08,
  0x05, 0x6E, 0x61, 0x6D, 0x65, 0x33
};

const uint8_t NAME_LSA_DIFF_SEQ[] = {
  0x89, 0x40, 0x80, 0x23, 0x07, 0x09, 0x08, 0x07, 0x72, 0x6F, 0x75, 0x74, 0x65, 0x72, 0x31,
  0x82, 0x01, 0x0E, 0x8B, 0x13, 0x32, 0x30, 0x32, 0x30, 0x2D, 0x30, 0x33, 0x2D, 0x32, 0x36,
  0x20, 0x30, 0x34, 0x3A, 0x31, 0x33, 0x3A, 0x33, 0x34, 0x07, 0x07, 0x08, 0x05, 0x6E, 0x61,
  0x6D, 0x65, 0x31, 0x07, 0x07, 0x08, 0x05, 0x6E, 0x61, 0x6D, 0x65, 0x32, 0x07, 0x07, 0x08,
  0x05, 0x6E, 0x61, 0x6D, 0x65, 0x33
};

const uint8_t NAME_LSA_DIFF_TS[] = {
  0x89, 0x40, 0x80, 0x23, 0x07, 0x09, 0x08, 0x07, 0x72, 0x6F, 0x75, 0x74, 0x65, 0x72, 0x31,
  0x82, 0x01, 0x0E, 0x8B, 0x13, 0x32, 0x30, 0x32, 0x30, 0x2D, 0x30, 0x33, 0x2D, 0x32, 0x36,
  0x20, 0x30, 0x34, 0x3A, 0x31, 0x33, 0x3A, 0x34, 0x34, 0x07, 0x07, 0x08, 0x05, 0x6E, 0x61,
  0x6D, 0x65, 0x31, 0x07, 0x07, 0x08, 0x05, 0x6E, 0x61, 0x6D, 0x65, 0x32, 0x07, 0x07, 0x08,
  0x05, 0x6E, 0x61, 0x6D, 0x65, 0x33
};

BOOST_AUTO_TEST_CASE(Basic)
{
  ndn::Name s1{"name1"};
  ndn::Name s2{"name2"};
  NamePrefixList npl1{s1, s2};

  auto testTimePoint = ndn::time::fromUnixTimestamp(ndn::time::milliseconds(1585196014943));

  // 3rd argument is seqNo
  NameLsa nlsa1("router1", 12, testTimePoint, npl1);
  NameLsa nlsa2("router2", 12, testTimePoint, npl1);

  BOOST_CHECK_EQUAL(nlsa1.getType(), Lsa::Type::NAME);
  BOOST_CHECK(nlsa1.getExpirationTimePoint() == nlsa2.getExpirationTimePoint());

  auto wire = nlsa1.wireEncode();
  BOOST_TEST(wire == NAME_LSA1, boost::test_tools::per_element());

  nlsa1.addName("name3");
  wire = nlsa1.wireEncode();
  BOOST_TEST(wire == NAME_LSA_EXTRA_NAME, boost::test_tools::per_element());

  nlsa1.setSeqNo(14);
  wire = nlsa1.wireEncode();
  BOOST_TEST(wire == NAME_LSA_DIFF_SEQ, boost::test_tools::per_element());

  testTimePoint =
    ndn::time::fromUnixTimestamp(ndn::time::milliseconds(1585196024993));
  nlsa1.setExpirationTimePoint(testTimePoint);
  wire = nlsa1.wireEncode();
  BOOST_TEST(wire == NAME_LSA_DIFF_TS, boost::test_tools::per_element());

  // Not testing router name as not sure if that will ever change once set
}

BOOST_AUTO_TEST_CASE(InitializeFromContent)
{
  auto testTimePoint = ndn::time::system_clock::now();
  ndn::Name s1{"name1"};
  ndn::Name s2{"name2"};
  NamePrefixList npl1{s1, s2};

  NameLsa nlsa1("router1", 1, testTimePoint, npl1);
  NameLsa nlsa2(nlsa1.wireEncode());
  BOOST_CHECK_EQUAL(nlsa1.wireEncode(), nlsa2.wireEncode());
}

BOOST_AUTO_TEST_CASE(OperatorEquals)
{
  NameLsa lsa1;
  NameLsa lsa2;
  ndn::Name name1("/ndn/test/name1");
  ndn::Name name2("/ndn/test/name2");
  ndn::Name name3("/ndn/some/other/name1");

  lsa1.addName(name1);
  lsa1.addName(name2);
  lsa1.addName(name3);

  lsa2.addName(name1);
  lsa2.addName(name2);
  lsa2.addName(name3);

  BOOST_CHECK(lsa1.isEqualContent(lsa2));
}

BOOST_AUTO_TEST_CASE(Update)
{
  NameLsa knownNameLsa;
  knownNameLsa.m_originRouter = ndn::Name("/yoursunny/_/%C1.Router/dal");
  knownNameLsa.m_seqNo = 2683;
  knownNameLsa.setExpirationTimePoint(ndn::time::system_clock::now() + 3561_ms);
  knownNameLsa.addName("/yoursunny/_/dal");
  knownNameLsa.addName("/ndn");

  auto rcvdLsa = std::make_shared<NameLsa>();
  rcvdLsa->m_originRouter = ndn::Name("/yoursunny/_/%C1.Router/dal");
  rcvdLsa->m_seqNo = 2684;
  rcvdLsa->setExpirationTimePoint(ndn::time::system_clock::now() + 3600_ms);

  auto nlsa = std::static_pointer_cast<NameLsa>(rcvdLsa);
  nlsa->addName("/ndn");
  nlsa->addName("/yoursunny/_/dal");
  ndn::Name addedName1("/yoursunny/video/ndn-dpdk_acmicn20_20200917");
  ndn::Name addedName2("/yoursunny/pushups");
  nlsa->addName(addedName1);
  nlsa->addName(addedName2);

  auto [updated, namesToAdd, namesToRemove] = knownNameLsa.update(rcvdLsa);

  BOOST_CHECK_EQUAL(updated, true);
  BOOST_CHECK_EQUAL(namesToAdd.size(), 2);
  BOOST_CHECK_EQUAL(namesToRemove.size(), 0);
  auto it = std::find(namesToAdd.begin(), namesToAdd.end(), addedName1);
  BOOST_CHECK(it != namesToAdd.end());
  it = std::find(namesToAdd.begin(), namesToAdd.end(), addedName2);
  BOOST_CHECK(it != namesToAdd.end());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::test
