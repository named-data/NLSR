/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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

#include "lsa/adj-lsa.hpp"

#include "tests/boost-test.hpp"

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestAdjLsa)

const uint8_t ADJ_LSA1[] = {
  0x83, 0x58, 0x80, 0x2D, 0x07, 0x13, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x04, 0x73, 0x69,
  0x74, 0x65, 0x08, 0x06, 0x72, 0x6F, 0x75, 0x74, 0x65, 0x72, 0x82, 0x01, 0x0C, 0x8B, 0x13,
  0x32, 0x30, 0x32, 0x30, 0x2D, 0x30, 0x33, 0x2D, 0x32, 0x36, 0x20, 0x30, 0x34, 0x3A, 0x31,
  0x33, 0x3A, 0x33, 0x34, 0x84, 0x27, 0x07, 0x16, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x04,
  0x73, 0x69, 0x74, 0x65, 0x08, 0x09, 0x61, 0x64, 0x6A, 0x61, 0x63, 0x65, 0x6E, 0x63, 0x79,
  0x8D, 0x03, 0x3A, 0x2F, 0x2F, 0x8C, 0x08, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t ADJ_LSA_EXTRA_NEIGHBOR[] = {
  0x83, 0x80, 0x80, 0x2D, 0x07, 0x13, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x04, 0x73, 0x69,
  0x74, 0x65, 0x08, 0x06, 0x72, 0x6F, 0x75, 0x74, 0x65, 0x72, 0x82, 0x01, 0x0C, 0x8B, 0x13,
  0x32, 0x30, 0x32, 0x30, 0x2D, 0x30, 0x33, 0x2D, 0x32, 0x36, 0x20, 0x30, 0x34, 0x3A, 0x31,
  0x33, 0x3A, 0x33, 0x34, 0x84, 0x27, 0x07, 0x16, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x04,
  0x73, 0x69, 0x74, 0x65, 0x08, 0x09, 0x61, 0x64, 0x6A, 0x61, 0x63, 0x65, 0x6E, 0x63, 0x79,
  0x8D, 0x03, 0x3A, 0x2F, 0x2F, 0x8C, 0x08, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x84, 0x26, 0x07, 0x15, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x03, 0x65, 0x64, 0x75, 0x08,
  0x09, 0x61, 0x64, 0x6A, 0x61, 0x63, 0x65, 0x6E, 0x63, 0x79, 0x8D, 0x03, 0x3A, 0x2F, 0x2F,
  0x8C, 0x08, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t ADJ_LSA_DIFF_SEQ[] = {
  0x83, 0x80, 0x80, 0x2D, 0x07, 0x13, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x04, 0x73, 0x69,
  0x74, 0x65, 0x08, 0x06, 0x72, 0x6F, 0x75, 0x74, 0x65, 0x72, 0x82, 0x01, 0x0E, 0x8B, 0x13,
  0x32, 0x30, 0x32, 0x30, 0x2D, 0x30, 0x33, 0x2D, 0x32, 0x36, 0x20, 0x30, 0x34, 0x3A, 0x31,
  0x33, 0x3A, 0x33, 0x34, 0x84, 0x27, 0x07, 0x16, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x04,
  0x73, 0x69, 0x74, 0x65, 0x08, 0x09, 0x61, 0x64, 0x6A, 0x61, 0x63, 0x65, 0x6E, 0x63, 0x79,
  0x8D, 0x03, 0x3A, 0x2F, 0x2F, 0x8C, 0x08, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x84, 0x26, 0x07, 0x15, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x03, 0x65, 0x64, 0x75, 0x08,
  0x09, 0x61, 0x64, 0x6A, 0x61, 0x63, 0x65, 0x6E, 0x63, 0x79, 0x8D, 0x03, 0x3A, 0x2F, 0x2F,
  0x8C, 0x08, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const uint8_t ADJ_LSA_DIFF_TS[] = {
  0x83, 0x80, 0x80, 0x2D, 0x07, 0x13, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x04, 0x73, 0x69,
  0x74, 0x65, 0x08, 0x06, 0x72, 0x6F, 0x75, 0x74, 0x65, 0x72, 0x82, 0x01, 0x0E, 0x8B, 0x13,
  0x32, 0x30, 0x32, 0x30, 0x2D, 0x30, 0x33, 0x2D, 0x32, 0x36, 0x20, 0x30, 0x34, 0x3A, 0x31,
  0x33, 0x3A, 0x34, 0x34, 0x84, 0x27, 0x07, 0x16, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x04,
  0x73, 0x69, 0x74, 0x65, 0x08, 0x09, 0x61, 0x64, 0x6A, 0x61, 0x63, 0x65, 0x6E, 0x63, 0x79,
  0x8D, 0x03, 0x3A, 0x2F, 0x2F, 0x8C, 0x08, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x84, 0x26, 0x07, 0x15, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x03, 0x65, 0x64, 0x75, 0x08,
  0x09, 0x61, 0x64, 0x6A, 0x61, 0x63, 0x65, 0x6E, 0x63, 0x79, 0x8D, 0x03, 0x3A, 0x2F, 0x2F,
  0x8C, 0x08, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

BOOST_AUTO_TEST_CASE(Basic)
{
  ndn::Name routerName("/ndn/site/router");
  ndn::Name adjacencyName("/ndn/site/adjacency");
  auto testTimePoint = ndn::time::fromUnixTimestamp(ndn::time::milliseconds(1585196014943));
  uint32_t seqNo = 12;

  // An AdjLsa initialized with ACTIVE adjacencies should copy the adjacencies
  AdjacencyList activeAdjacencies;
  Adjacent activeAdjacency(adjacencyName);
  activeAdjacency.setStatus(Adjacent::STATUS_ACTIVE);
  activeAdjacencies.insert(activeAdjacency);

  AdjLsa alsa1(routerName, seqNo, testTimePoint, activeAdjacencies);
  BOOST_CHECK_EQUAL(alsa1.getAdl().size(), 1);
  BOOST_CHECK_EQUAL(alsa1.getType(), Lsa::Type::ADJACENCY);
  BOOST_CHECK_EQUAL(alsa1.getSeqNo(), seqNo);
  BOOST_CHECK_EQUAL(alsa1.getExpirationTimePoint(), testTimePoint);
  BOOST_CHECK(alsa1.getAdl().isNeighbor(activeAdjacency.getName()));

  // An AdjLsa initialized with INACTIVE adjacencies should not copy the adjacencies
  AdjacencyList inactiveAdjacencies;
  Adjacent inactiveAdjacency(adjacencyName);
  inactiveAdjacency.setStatus(Adjacent::STATUS_INACTIVE);
  inactiveAdjacencies.insert(inactiveAdjacency);

  AdjLsa alsa2(routerName, seqNo, testTimePoint, inactiveAdjacencies);
  BOOST_CHECK_EQUAL(alsa2.getAdl().size(), 0);

  // Thus, the two LSAs should not have equal content
  BOOST_CHECK_NE(alsa1, alsa2);

  // Create a duplicate of alsa1 which should have equal content
  AdjLsa alsa3(routerName, seqNo, testTimePoint, activeAdjacencies);
  BOOST_CHECK_EQUAL(alsa1, alsa3);

  auto wire = alsa1.wireEncode();
  BOOST_TEST(wire == ADJ_LSA1, boost::test_tools::per_element());

  Adjacent activeAdjacency2("/ndn/edu/adjacency");
  activeAdjacency2.setStatus(Adjacent::STATUS_ACTIVE);
  alsa1.addAdjacent(activeAdjacency2);
  wire = alsa1.wireEncode();
  BOOST_TEST(wire == ADJ_LSA_EXTRA_NEIGHBOR, boost::test_tools::per_element());

  alsa1.setSeqNo(14);
  wire = alsa1.wireEncode();
  BOOST_TEST(wire == ADJ_LSA_DIFF_SEQ, boost::test_tools::per_element());

  testTimePoint = ndn::time::fromUnixTimestamp(ndn::time::milliseconds(1585196024993));
  alsa1.setExpirationTimePoint(testTimePoint);
  wire = alsa1.wireEncode();
  BOOST_TEST(wire == ADJ_LSA_DIFF_TS, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(IncrementAdjacentNumber)
{
  Adjacent adj1("adjacent1");
  Adjacent adj2("adjacent2");

  adj1.setStatus(Adjacent::STATUS_ACTIVE);
  adj2.setStatus(Adjacent::STATUS_ACTIVE);

  AdjacencyList adjList;
  adjList.insert(adj1);
  adjList.insert(adj2);

  auto testTimePoint = ndn::time::system_clock::now() + ndn::time::seconds(3600);

  AdjLsa lsa("router1", 12, testTimePoint, adjList);

  std::ostringstream os;
  os << lsa;

  std::string EXPECTED_OUTPUT =
    "    ADJACENCY LSA:\n"
    "      Origin Router      : /router1\n"
    "      Sequence Number    : 12\n"
    "      Expires in         : 3599999 milliseconds\n"
    "      Adjacent(s):\n"
    "        Adjacent 0: (name=/adjacent1, uri=://, cost=10)\n"
    "        Adjacent 1: (name=/adjacent2, uri=://, cost=10)\n";

  BOOST_CHECK_EQUAL(os.str(), EXPECTED_OUTPUT);
}

BOOST_AUTO_TEST_CASE(InitializeFromContent)
{
  Adjacent adj1("adjacent1");
  Adjacent adj2("adjacent2");

  adj1.setStatus(Adjacent::STATUS_ACTIVE);
  adj2.setStatus(Adjacent::STATUS_ACTIVE);

  //If we don't do this the test will fail
  //Adjacent has default cost of 10 but no default
  //connecting face URI, so initializeFromContent fails
  adj1.setFaceUri(ndn::FaceUri("udp://10.0.0.1"));
  adj2.setFaceUri(ndn::FaceUri("udp://10.0.0.2"));

  AdjacencyList adjList;
  adjList.insert(adj1);
  adjList.insert(adj2);

  auto testTimePoint = ndn::time::system_clock::now();

  AdjLsa adjlsa1("router1", 1, testTimePoint, adjList);
  AdjLsa adjlsa2(adjlsa1.wireEncode());
  BOOST_CHECK_EQUAL(adjlsa1, adjlsa2);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests