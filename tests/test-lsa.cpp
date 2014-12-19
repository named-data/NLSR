/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 * \author Ashlesh Gawande <agawande@memphis.edu>
 *
 **/
#include "lsa.hpp"
#include "name-prefix-list.hpp"
#include "adjacent.hpp"
#include <boost/test/unit_test.hpp>
#include <ndn-cxx/util/time.hpp>

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
//lsType is 1 for NameLsa, 3rd arg is seqNo. which will be a random number I just put in 12.
  NameLsa nlsa1("router1", NameLsa::TYPE_STRING, 12, testTimePoint, npl1);
  NameLsa nlsa2("router2", NameLsa::TYPE_STRING, 12, testTimePoint, npl1);

  BOOST_CHECK_EQUAL(nlsa1.getLsType(), NameLsa::TYPE_STRING);

  BOOST_CHECK(nlsa1.getExpirationTimePoint() == nlsa2.getExpirationTimePoint());

  BOOST_CHECK(nlsa1.getKey() != nlsa2.getKey());
}

BOOST_AUTO_TEST_CASE(AdjacentLsaConstructorAndGetters)
{
  Adjacent adj1("adjacent");
  Adjacent adj2("adjacent2");

  AdjacencyList adjList;
  adjList.insert(adj1);
  ndn::time::system_clock::TimePoint testTimePoint =  ndn::time::system_clock::now();
//For AdjLsa, lsType is 2.
//1 is the number of adjacent in adjacent list.
  AdjLsa alsa1("router1", AdjLsa::TYPE_STRING, 12, testTimePoint, 1, adjList);
  AdjLsa alsa2("router1", AdjLsa::TYPE_STRING, 12, testTimePoint, 1, adjList);

  BOOST_CHECK_EQUAL(alsa1.getLsType(), AdjLsa::TYPE_STRING);
  BOOST_CHECK_EQUAL(alsa1.getLsSeqNo(), (uint32_t)12);
  BOOST_CHECK_EQUAL(alsa1.getExpirationTimePoint(), testTimePoint);
  BOOST_CHECK_EQUAL(alsa1.getNoLink(), (uint32_t)1);

  BOOST_CHECK(alsa1.isEqualContent(alsa2));

  alsa1.addAdjacent(adj2);

  const std::string ADJACENT_1 = "adjacent2";
  BOOST_CHECK(alsa1.getAdl().isNeighbor(ADJACENT_1));
}

BOOST_AUTO_TEST_CASE(CoordinateLsaConstructorAndGetters)
{
  ndn::time::system_clock::TimePoint testTimePoint =  ndn::time::system_clock::now();
//For CoordinateLsa, lsType is 3.
  CoordinateLsa clsa1("router1", CoordinateLsa::TYPE_STRING, 12, testTimePoint, 2.5, 30.0);
  CoordinateLsa clsa2("router1", CoordinateLsa::TYPE_STRING, 12, testTimePoint, 2.5, 30.0);

  BOOST_CHECK_CLOSE(clsa1.getCorRadius(), 2.5, 0.0001);
  BOOST_CHECK_CLOSE(clsa1.getCorTheta(), 30.0, 0.0001);

  BOOST_CHECK(clsa1.isEqualContent(clsa2));

  BOOST_CHECK_EQUAL(clsa1.getData(), clsa2.getData());
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
