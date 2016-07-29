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

#include "tlv/lsdb-status.hpp"

#include "../boost-test.hpp"

namespace nlsr {
namespace tlv  {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestLsdbStatus)

const uint8_t LsdbStatusData1[] =
{
  // Header
  0x8a, 0x7f,
  // AdjacencyLsa
  0x83, 0x32,
    // LsaInfo
    0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
    0x80, 0x8b, 0x02, 0x27, 0x10,
    // Adjacency
    0x84, 0x1d, 0x07, 0x0c, 0x08, 0x0a, 0x61, 0x64, 0x6a, 0x61, 0x63, 0x65, 0x6e, 0x63,
    0x79, 0x31, 0x8d, 0x0a, 0x61, 0x64, 0x6a, 0x61, 0x63, 0x65, 0x6e, 0x63, 0x79, 0x31,
    0x8c, 0x01, 0x80,
  // CoordianteLsa
  0x85, 0x2b,
    // LsaInfo
    0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
    0x80, 0x8b, 0x02, 0x27, 0x10,
    // HyperbolicRadius
    0x87, 0x0a, 0x86, 0x08, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xfa, 0x3f,
    // HyperbolicAngle
    0x88, 0x0a, 0x86, 0x08, 0x7b, 0x14, 0xae, 0x47, 0xe1, 0x7a, 0xfc, 0x3f,
  // NameLsa
  0x89, 0x1c,
    // LsaInfo
    0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
    0x80, 0x8b, 0x02, 0x27, 0x10,
    // Name
    0x07, 0x07, 0x08, 0x05, 0x6e, 0x61, 0x6d, 0x65, 0x31
};

const uint8_t LsdbStatusData2[] =
{
  // Header
  0x8a, 0x00
};

const uint8_t LsdbStatusData3[] =
{
  // Header
  0x8a, 0x7f,
  // CoordianteLsa
  0x85, 0x2b,
    // LsaInfo
    0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
    0x80, 0x8b, 0x02, 0x27, 0x10,
    // HyperbolicRadius
    0x87, 0x0a, 0x86, 0x08, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xfa, 0x3f,
    // HyperbolicAngle
    0x88, 0x0a, 0x86, 0x08, 0x7b, 0x14, 0xae, 0x47, 0xe1, 0x7a, 0xfc, 0x3f,
  // NameLsa
  0x89, 0x1c,
    // LsaInfo
    0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
    0x80, 0x8b, 0x02, 0x27, 0x10,
    // Name
    0x07, 0x07, 0x08, 0x05, 0x6e, 0x61, 0x6d, 0x65, 0x31,
  // AdjacencyLsa
   0x83, 0x32,
     // LsaInfo
     0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
     0x80, 0x8b, 0x02, 0x27, 0x10,
     // Adjacency
     0x84, 0x1d, 0x07, 0x0c, 0x08, 0x0a, 0x61, 0x64, 0x6a, 0x61, 0x63, 0x65, 0x6e, 0x63,
     0x79, 0x31, 0x8d, 0x0a, 0x61, 0x64, 0x6a, 0x61, 0x63, 0x65, 0x6e, 0x63, 0x79, 0x31,
     0x8c, 0x01, 0x80
};

BOOST_AUTO_TEST_CASE(LsdbStatusEncode1)
{
  LsdbStatus lsdbStatus;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));

  // AdjacencyLsa
  AdjacencyLsa adjacencyLsa;
  adjacencyLsa.setLsaInfo(lsaInfo);

  Adjacency adjacency1;
  adjacency1.setName("adjacency1");
  adjacency1.setUri("adjacency1");
  adjacency1.setCost(128);
  adjacencyLsa.addAdjacency(adjacency1);

  lsdbStatus.addAdjacencyLsa(adjacencyLsa);

  // CoordinateLsa
  CoordinateLsa coordinateLsa;
  coordinateLsa.setLsaInfo(lsaInfo);

  coordinateLsa.setHyperbolicRadius(1.65);
  coordinateLsa.setHyperbolicAngle(1.78);

  lsdbStatus.addCoordinateLsa(coordinateLsa);

  // NameLsa
  NameLsa nameLsa;
  nameLsa.setLsaInfo(lsaInfo);
  nameLsa.addName("name1");

  lsdbStatus.addNameLsa(nameLsa);

  const ndn::Block& wire = lsdbStatus.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(LsdbStatusData1,
                                  LsdbStatusData1 + sizeof(LsdbStatusData1),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(LsdbStatusEncode2)
{
  LsdbStatus lsdbStatus;

  const ndn::Block& wire = lsdbStatus.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(LsdbStatusData2,
                                  LsdbStatusData2 + sizeof(LsdbStatusData2),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(LsdbStatusDecode1)
{
  LsdbStatus lsdbStatus;

  lsdbStatus.wireDecode(ndn::Block(LsdbStatusData1, sizeof(LsdbStatusData1)));

  std::list<AdjacencyLsa> adjacencyLsas = lsdbStatus.getAdjacencyLsas();
  std::list<AdjacencyLsa>::const_iterator it1 = adjacencyLsas.begin();

  LsaInfo lsaInfo = it1->getLsaInfo();
  BOOST_CHECK_EQUAL(lsaInfo.getOriginRouter(), "test");
  BOOST_CHECK_EQUAL(lsaInfo.getSequenceNumber(), 128);
  BOOST_CHECK_EQUAL(lsaInfo.getExpirationPeriod(), ndn::time::milliseconds(10000));

  std::list<Adjacency> adjacencies = it1->getAdjacencies();
  std::list<Adjacency>::const_iterator it2 = adjacencies.begin();
  BOOST_CHECK_EQUAL(it2->getName(), "adjacency1");
  BOOST_CHECK_EQUAL(it2->getUri(), "adjacency1");
  BOOST_CHECK_EQUAL(it2->getCost(), 128);

  BOOST_CHECK_EQUAL(lsdbStatus.hasAdjacencyLsas(), true);

  std::list<CoordinateLsa> coordinateLsas = lsdbStatus.getCoordinateLsas();
  std::list<CoordinateLsa>::const_iterator it3 = coordinateLsas.begin();

  lsaInfo = it3->getLsaInfo();
  BOOST_CHECK_EQUAL(lsaInfo.getOriginRouter(), "test");
  BOOST_CHECK_EQUAL(lsaInfo.getSequenceNumber(), 128);
  BOOST_CHECK_EQUAL(lsaInfo.getExpirationPeriod(), ndn::time::milliseconds(10000));

  BOOST_REQUIRE_EQUAL(it3->getHyperbolicRadius(), 1.65);
  BOOST_REQUIRE_EQUAL(it3->getHyperbolicAngle(), 1.78);

  BOOST_CHECK_EQUAL(lsdbStatus.hasCoordinateLsas(), true);

  std::list<NameLsa> nameLsas = lsdbStatus.getNameLsas();
  std::list<NameLsa>::const_iterator it4 = nameLsas.begin();

  lsaInfo = it4->getLsaInfo();
  BOOST_CHECK_EQUAL(lsaInfo.getOriginRouter(), "test");
  BOOST_CHECK_EQUAL(lsaInfo.getSequenceNumber(), 128);
  BOOST_CHECK_EQUAL(lsaInfo.getExpirationPeriod(), ndn::time::milliseconds(10000));

  std::list<ndn::Name> names = it4->getNames();
  std::list<ndn::Name>::const_iterator it5 = names.begin();
  BOOST_CHECK_EQUAL(*it5, "name1");

  BOOST_CHECK_EQUAL(lsdbStatus.hasNameLsas(), true);
}

BOOST_AUTO_TEST_CASE(LsdbStatusDecode2)
{
  LsdbStatus lsdbStatus;

  lsdbStatus.wireDecode(ndn::Block(LsdbStatusData2, sizeof(LsdbStatusData2)));

  BOOST_CHECK_EQUAL(lsdbStatus.hasAdjacencyLsas(), false);
  BOOST_CHECK_EQUAL(lsdbStatus.hasCoordinateLsas(), false);
  BOOST_CHECK_EQUAL(lsdbStatus.hasNameLsas(), false);
}

BOOST_AUTO_TEST_CASE(LsdbStatusDecode3)
{
  LsdbStatus lsdbStatus;

  BOOST_CHECK_THROW(lsdbStatus.wireDecode(ndn::Block(LsdbStatusData3, sizeof(LsdbStatusData3))),
                    LsdbStatus::Error);
}

BOOST_AUTO_TEST_CASE(LsdbStatusClear)
{
  LsdbStatus lsdbStatus;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));

  // AdjacencyLsa
  AdjacencyLsa adjacencyLsa;
  adjacencyLsa.setLsaInfo(lsaInfo);

  Adjacency adjacency1;
  adjacency1.setName("adjacency1");
  adjacency1.setUri("adjacency1");
  adjacency1.setCost(128);
  adjacencyLsa.addAdjacency(adjacency1);

  lsdbStatus.addAdjacencyLsa(adjacencyLsa);
  BOOST_CHECK_EQUAL(lsdbStatus.hasAdjacencyLsas(), true);
  lsdbStatus.clearAdjacencyLsas();
  BOOST_CHECK_EQUAL(lsdbStatus.hasAdjacencyLsas(), false);

  // CoordinateLsa
  CoordinateLsa coordinateLsa;
  coordinateLsa.setLsaInfo(lsaInfo);

  coordinateLsa.setHyperbolicRadius(1.65);
  coordinateLsa.setHyperbolicAngle(1.78);

  lsdbStatus.addCoordinateLsa(coordinateLsa);
  BOOST_CHECK_EQUAL(lsdbStatus.hasCoordinateLsas(), true);
  lsdbStatus.clearCoordinateLsas();
  BOOST_CHECK_EQUAL(lsdbStatus.hasCoordinateLsas(), false);

  // NameLsa
  NameLsa nameLsa;
  nameLsa.setLsaInfo(lsaInfo);
  nameLsa.addName("name1");

  lsdbStatus.addNameLsa(nameLsa);
  BOOST_CHECK_EQUAL(lsdbStatus.hasNameLsas(), true);
  lsdbStatus.clearNameLsas();
  BOOST_CHECK_EQUAL(lsdbStatus.hasNameLsas(), false);
}

BOOST_AUTO_TEST_CASE(LsdbStatusOutputStream)
{
  LsdbStatus lsdbStatus;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));

  // AdjacencyLsa
  AdjacencyLsa adjacencyLsa;
  adjacencyLsa.setLsaInfo(lsaInfo);

  Adjacency adjacency1;
  adjacency1.setName("adjacency1");
  adjacency1.setUri("adjacency1");
  adjacency1.setCost(128);
  adjacencyLsa.addAdjacency(adjacency1);

  lsdbStatus.addAdjacencyLsa(adjacencyLsa);

  // CoordinateLsa
  CoordinateLsa coordinateLsa;
  coordinateLsa.setLsaInfo(lsaInfo);

  coordinateLsa.setHyperbolicRadius(1.65);
  coordinateLsa.setHyperbolicAngle(1.78);

  lsdbStatus.addCoordinateLsa(coordinateLsa);

  // NameLsa
  NameLsa nameLsa;
  nameLsa.setLsaInfo(lsaInfo);
  nameLsa.addName("name1");

  lsdbStatus.addNameLsa(nameLsa);

  std::ostringstream os;
  os << lsdbStatus;

  BOOST_CHECK_EQUAL(os.str(), "LsdbStatus("
                                "AdjacencyLsa("
                                  "LsaInfo("
                                    "OriginRouter: /test, "
                                    "SequenceNumber: 128, "
                                    "ExpirationPeriod: 10000 milliseconds), "
                                  "Adjacency(Name: /adjacency1, Uri: adjacency1, Cost: 128)), "
                                "CoordinateLsa("
                                  "LsaInfo("
                                    "OriginRouter: /test, "
                                    "SequenceNumber: 128, "
                                    "ExpirationPeriod: 10000 milliseconds), "
                                  "HyperbolicRadius: 1.65, "
                                  "HyperbolicAngle: 1.78), "
                                "NameLsa("
                                  "LsaInfo("
                                    "OriginRouter: /test, "
                                    "SequenceNumber: 128, "
                                    "ExpirationPeriod: 10000 milliseconds), "
                                  "Name: /name1))");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
