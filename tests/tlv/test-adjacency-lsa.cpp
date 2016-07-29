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

#include "tlv/adjacency-lsa.hpp"

#include "../boost-test.hpp"

namespace nlsr {
namespace tlv  {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestAdjacencyLsa)

const uint8_t AdjacencyLsaWithAdjacenciesData[] =
{
  // Header
  0x83, 0x3d,
  // LsaInfo
  0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
  0x80, 0x8b, 0x02, 0x27, 0x10,
  // Adjacency
  0x84, 0x13, 0x07, 0x07, 0x08, 0x05, 0x74, 0x65, 0x73, 0x74, 0x31, 0x8d, 0x05, 0x74,
  0x65, 0x73, 0x74, 0x31, 0x8c, 0x01, 0x80,
  // Adjacency
  0x84, 0x13, 0x07, 0x07, 0x08, 0x05, 0x74, 0x65, 0x73, 0x74, 0x32, 0x8d, 0x05, 0x74,
  0x65, 0x73, 0x74, 0x32, 0x8c, 0x01, 0x80
};

const uint8_t AdjacencyLsaWithoutAdjacenciesData[] =
{
  // Header
  0x83, 0x13,
  // LsaInfo
  0x80, 0x11, 0x81, 0x08, 0x07, 0x06, 0x08, 0x04, 0x74, 0x65, 0x73, 0x74, 0x82, 0x01,
  0x80, 0x8b, 0x02, 0x27, 0x10,
};

BOOST_AUTO_TEST_CASE(AdjacencyLsaEncodeWithAdjacencies)
{
  AdjacencyLsa adjacencyLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  adjacencyLsa.setLsaInfo(lsaInfo);

  Adjacency adjacency1;
  adjacency1.setName("test1");
  adjacency1.setUri("test1");
  adjacency1.setCost(128);
  adjacencyLsa.addAdjacency(adjacency1);

  Adjacency adjacency2;
  adjacency2.setName("test2");
  adjacency2.setUri("test2");
  adjacency2.setCost(128);
  adjacencyLsa.addAdjacency(adjacency2);

  const ndn::Block& wire = adjacencyLsa.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(AdjacencyLsaWithAdjacenciesData,
                                  AdjacencyLsaWithAdjacenciesData +
                                    sizeof(AdjacencyLsaWithAdjacenciesData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(AdjacencyLsaDecodeWithAdjacencies)
{
  AdjacencyLsa adjacencyLsa;

  adjacencyLsa.wireDecode(ndn::Block(AdjacencyLsaWithAdjacenciesData,
                                      sizeof(AdjacencyLsaWithAdjacenciesData)));

  LsaInfo lsaInfo = adjacencyLsa.getLsaInfo();
  BOOST_CHECK_EQUAL(lsaInfo.getOriginRouter(), "test");
  BOOST_CHECK_EQUAL(lsaInfo.getSequenceNumber(), 128);
  BOOST_CHECK_EQUAL(lsaInfo.getExpirationPeriod(), ndn::time::milliseconds(10000));

  BOOST_CHECK_EQUAL(adjacencyLsa.hasAdjacencies(), true);
  std::list<Adjacency> adjacencies = adjacencyLsa.getAdjacencies();
  std::list<Adjacency>::const_iterator it = adjacencies.begin();
  BOOST_CHECK_EQUAL(it->getName(), "test1");
  BOOST_CHECK_EQUAL(it->getUri(), "test1");
  BOOST_CHECK_EQUAL(it->getCost(), 128);

  it++;
  BOOST_CHECK_EQUAL(it->getName(), "test2");
  BOOST_CHECK_EQUAL(it->getUri(), "test2");
  BOOST_CHECK_EQUAL(it->getCost(), 128);
}

BOOST_AUTO_TEST_CASE(AdjacencyLsaEncodeWithoutAdjacencies)
{
  AdjacencyLsa adjacencyLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  adjacencyLsa.setLsaInfo(lsaInfo);

  const ndn::Block& wire = adjacencyLsa.wireEncode();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(AdjacencyLsaWithoutAdjacenciesData,
                                  AdjacencyLsaWithoutAdjacenciesData +
                                    sizeof(AdjacencyLsaWithoutAdjacenciesData),
                                  wire.begin(), wire.end());
}

BOOST_AUTO_TEST_CASE(AdjacencyLsaDecodeWithoutAdjacencies)
{
  AdjacencyLsa adjacencyLsa;

  adjacencyLsa.wireDecode(ndn::Block(AdjacencyLsaWithoutAdjacenciesData,
                                      sizeof(AdjacencyLsaWithoutAdjacenciesData)));

  LsaInfo lsaInfo = adjacencyLsa.getLsaInfo();
  BOOST_CHECK_EQUAL(lsaInfo.getOriginRouter(), "test");
  BOOST_CHECK_EQUAL(lsaInfo.getSequenceNumber(), 128);
  BOOST_CHECK_EQUAL(lsaInfo.getExpirationPeriod(), ndn::time::milliseconds(10000));

  BOOST_CHECK_EQUAL(adjacencyLsa.hasAdjacencies(), false);
}


BOOST_AUTO_TEST_CASE(AdjacencyLsaClear)
{
  AdjacencyLsa adjacencyLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  adjacencyLsa.setLsaInfo(lsaInfo);

  Adjacency adjacency1;
  adjacency1.setName("test1");
  adjacency1.setUri("test1");
  adjacency1.setCost(128);
  adjacencyLsa.addAdjacency(adjacency1);
  BOOST_CHECK_EQUAL(adjacencyLsa.getAdjacencies().size(), 1);

  std::list<Adjacency> adjacencies = adjacencyLsa.getAdjacencies();
  std::list<Adjacency>::const_iterator it = adjacencies.begin();
  BOOST_CHECK_EQUAL(it->getName(), "test1");
  BOOST_CHECK_EQUAL(it->getUri(), "test1");
  BOOST_CHECK_EQUAL(it->getCost(), 128);

  adjacencyLsa.clearAdjacencies();
  BOOST_CHECK_EQUAL(adjacencyLsa.getAdjacencies().size(), 0);

  Adjacency adjacency2;
  adjacency2.setName("test2");
  adjacency2.setUri("test2");
  adjacency2.setCost(128);
  adjacencyLsa.addAdjacency(adjacency2);
  BOOST_CHECK_EQUAL(adjacencyLsa.getAdjacencies().size(), 1);

  adjacencies = adjacencyLsa.getAdjacencies();
  it = adjacencies.begin();
  BOOST_CHECK_EQUAL(it->getName(), "test2");
  BOOST_CHECK_EQUAL(it->getUri(), "test2");
  BOOST_CHECK_EQUAL(it->getCost(), 128);
}

BOOST_AUTO_TEST_CASE(AdjacencyLsaOutputStream)
{
  AdjacencyLsa adjacencyLsa;

  LsaInfo lsaInfo;
  lsaInfo.setOriginRouter("test");
  lsaInfo.setSequenceNumber(128);
  lsaInfo.setExpirationPeriod(ndn::time::milliseconds(10000));
  adjacencyLsa.setLsaInfo(lsaInfo);

  Adjacency adjacency1;
  adjacency1.setName("test1");
  adjacency1.setUri("test1");
  adjacency1.setCost(128);
  adjacencyLsa.addAdjacency(adjacency1);

  Adjacency adjacency2;
  adjacency2.setName("test2");
  adjacency2.setUri("test2");
  adjacency2.setCost(128);
  adjacencyLsa.addAdjacency(adjacency2);

  std::ostringstream os;
  os << adjacencyLsa;

  BOOST_CHECK_EQUAL(os.str(), "AdjacencyLsa("
                                "LsaInfo("
                                  "OriginRouter: /test, "
                                  "SequenceNumber: 128, "
                                  "ExpirationPeriod: 10000 milliseconds), "
                                "Adjacency(Name: /test1, Uri: test1, Cost: 128), "
                                "Adjacency(Name: /test2, Uri: test2, Cost: 128))");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
