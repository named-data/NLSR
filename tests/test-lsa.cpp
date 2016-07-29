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

#include "test-common.hpp"

#include "adjacent.hpp"
#include "lsa.hpp"
#include "name-prefix-list.hpp"

#include <ndn-cxx/util/time.hpp>

#include <sstream>

namespace nlsr {
namespace test {

BOOST_AUTO_TEST_SUITE(TestLsa)

BOOST_AUTO_TEST_CASE(NameLsaBasic)
{
  NamePrefixList npl1;

  std::string s1 = "name1";
  std::string s2 = "name2";

  npl1.insert(s1);
  npl1.insert(s2);
  ndn::time::system_clock::TimePoint testTimePoint =  ndn::time::system_clock::now();

  //3rd arg is seqNo. which will be a random number I just put in 12.
  NameLsa nlsa1("router1", 12, testTimePoint, npl1);
  NameLsa nlsa2("router2", 12, testTimePoint, npl1);

  BOOST_CHECK_EQUAL(nlsa1.getLsType(), NameLsa::TYPE_STRING);

  BOOST_CHECK(nlsa1.getExpirationTimePoint() == nlsa2.getExpirationTimePoint());

  BOOST_CHECK(nlsa1.getKey() != nlsa2.getKey());
}

BOOST_AUTO_TEST_CASE(AdjacentLsaConstructorAndGetters)
{
  ndn::Name routerName("/ndn/site/router");
  ndn::Name adjacencyName("/ndn/site/adjacency");
  ndn::time::system_clock::TimePoint testTimePoint =  ndn::time::system_clock::now();
  uint32_t seqNo = 12;

  // An AdjLsa initialized with ACTIVE adjacencies should copy the adjacencies
  AdjacencyList activeAdjacencies;
  Adjacent activeAdjacency(adjacencyName);
  activeAdjacency.setStatus(Adjacent::STATUS_ACTIVE);
  activeAdjacencies.insert(activeAdjacency);

  AdjLsa alsa1(routerName, seqNo, testTimePoint,
               activeAdjacencies.getSize(), activeAdjacencies);
  BOOST_CHECK_EQUAL(alsa1.getAdl().getSize(), 1);
  BOOST_CHECK_EQUAL(alsa1.getLsType(), AdjLsa::TYPE_STRING);
  BOOST_CHECK_EQUAL(alsa1.getLsSeqNo(), seqNo);
  BOOST_CHECK_EQUAL(alsa1.getExpirationTimePoint(), testTimePoint);
  BOOST_CHECK_EQUAL(alsa1.getNoLink(), 1);
  BOOST_CHECK(alsa1.getAdl().isNeighbor(activeAdjacency.getName()));

  // An AdjLsa initialized with INACTIVE adjacencies should not copy the adjacencies
  AdjacencyList inactiveAdjacencies;
  Adjacent inactiveAdjacency(adjacencyName);
  inactiveAdjacency.setStatus(Adjacent::STATUS_INACTIVE);
  inactiveAdjacencies.insert(inactiveAdjacency);

  AdjLsa alsa2(routerName, seqNo, testTimePoint,
               inactiveAdjacencies.getSize(), inactiveAdjacencies);
  BOOST_CHECK_EQUAL(alsa2.getAdl().getSize(), 0);

  // Thus, the two LSAs should not have equal content
  BOOST_CHECK_EQUAL(alsa1.isEqualContent(alsa2), false);

  // Create a duplicate of alsa1 which should have equal content
  AdjLsa alsa3(routerName, seqNo, testTimePoint,
               activeAdjacencies.getSize(), activeAdjacencies);
  BOOST_CHECK(alsa1.isEqualContent(alsa3));
}

BOOST_AUTO_TEST_CASE(CoordinateLsaConstructorAndGetters)
{
  ndn::time::system_clock::TimePoint testTimePoint =  ndn::time::system_clock::now();
  CoordinateLsa clsa1("router1", 12, testTimePoint, 2.5, 30.0);
  CoordinateLsa clsa2("router1", 12, testTimePoint, 2.5, 30.0);

  BOOST_CHECK_CLOSE(clsa1.getCorRadius(), 2.5, 0.0001);
  BOOST_CHECK_CLOSE(clsa1.getCorTheta(), 30.0, 0.0001);

  BOOST_CHECK(clsa1.isEqualContent(clsa2));

  BOOST_CHECK_EQUAL(clsa1.getData(), clsa2.getData());
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

  ndn::time::system_clock::TimePoint testTimePoint = ndn::time::system_clock::now();

  std::ostringstream ss;
  ss << testTimePoint;

  const std::string TEST_TIME_POINT_STRING = ss.str();

  AdjLsa lsa("router1", 12, testTimePoint, adjList.getSize(), adjList);

  std::string EXPECTED_OUTPUT =
    "Adj Lsa:\n"
    "  Origination Router: /router1\n"
    "  Ls Type: adjacency\n"
    "  Ls Seq No: 12\n"
    "  Ls Lifetime: " + TEST_TIME_POINT_STRING + "\n"
    "  Adjacents: \n"
    "    Adjacent 1:\n"
    "      Adjacent Name: /adjacent1\n"
    "      Connecting FaceUri: \n"
    "      Link Cost: 10\n"
    "    Adjacent 2:\n"
    "      Adjacent Name: /adjacent2\n"
    "      Connecting FaceUri: \n"
    "      Link Cost: 10\n"
    "adj_lsa_end";

  std::ostringstream os;
  os << lsa;

  BOOST_CHECK_EQUAL(os.str(), EXPECTED_OUTPUT);
}

BOOST_AUTO_TEST_CASE(TestInitializeFromContent)
{
  //Adj LSA
  Adjacent adj1("adjacent1");
  Adjacent adj2("adjacent2");

  adj1.setStatus(Adjacent::STATUS_ACTIVE);
  adj2.setStatus(Adjacent::STATUS_ACTIVE);

  //If we don't do this the test will fail
  //Adjacent has default cost of 10 but no default
  //connecting face URI, so initializeFromContent fails
  adj1.setConnectingFaceUri("10.0.0.1");
  adj2.setConnectingFaceUri("10.0.0.2");

  AdjacencyList adjList;
  adjList.insert(adj1);
  adjList.insert(adj2);

  ndn::time::system_clock::TimePoint testTimePoint = ndn::time::system_clock::now();

  AdjLsa adjlsa1("router1", 1, testTimePoint, adjList.getSize(), adjList);
  AdjLsa adjlsa2;

  BOOST_CHECK(adjlsa2.initializeFromContent(adjlsa1.getData()));

  BOOST_CHECK(adjlsa1.isEqualContent(adjlsa2));

  //Name LSA
  NamePrefixList npl1;

  std::string s1 = "name1";
  std::string s2 = "name2";

  npl1.insert(s1);
  npl1.insert(s2);

  NameLsa nlsa1("router1", 1, testTimePoint, npl1);
  NameLsa nlsa2;

  BOOST_CHECK(nlsa2.initializeFromContent(nlsa1.getData()));

  BOOST_CHECK_EQUAL(nlsa1.getData(), nlsa2.getData());

  //Coordinate LSA
  CoordinateLsa clsa1("router1", 12, testTimePoint, 2.5, 30.0);
  CoordinateLsa clsa2;

  BOOST_CHECK(clsa2.initializeFromContent(clsa1.getData()));

  BOOST_CHECK_EQUAL(clsa1.getData(), clsa2.getData());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
