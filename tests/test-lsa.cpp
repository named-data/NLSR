/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "lsa.hpp"
#include "name-prefix-list.hpp"
#include "adjacent.hpp"
#include <boost/test/unit_test.hpp>

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

//lsType is 1 for NameLsa, 3rd arg is seqNo. which will be a random number I just put in 12.
//1800 is default lsa refresh time.
  NameLsa nlsa1("router1", 1, 12, 1800, npl1);
  NameLsa nlsa2("router2", 1, 12, 1500, npl1);

  BOOST_CHECK_EQUAL(nlsa1.getLsType(), (uint8_t)1);

  BOOST_CHECK(nlsa1.getLifeTime() != nlsa2.getLifeTime());

  BOOST_CHECK(nlsa1.getKey() != nlsa2.getKey());
}

BOOST_AUTO_TEST_CASE(AdjacentLsaConstructorAndGetters)
{
  Adjacent adj1("adjacent");
  Adjacent adj2("adjacent2");

  AdjacencyList adjList;
  adjList.insert(adj1);

//For AdjLsa, lsType is 2.
//1 is the number of adjacent in adjacent list.
  AdjLsa alsa1("router1", 2, 12, 1800, 1, adjList);
  AdjLsa alsa2("router1", 2, 12, 1800, 1, adjList);

  BOOST_CHECK_EQUAL(alsa1.getLsType(), (uint8_t)2);
  BOOST_CHECK_EQUAL(alsa1.getLsSeqNo(), (uint32_t)12);
  BOOST_CHECK_EQUAL(alsa1.getLifeTime(), (uint32_t)1800);
  BOOST_CHECK_EQUAL(alsa1.getNoLink(), (uint32_t)1);

  BOOST_CHECK(alsa1.isEqualContent(alsa2));

  alsa1.addAdjacent(adj2);

  const std::string ADJACENT_1 = "adjacent2";
  BOOST_CHECK(alsa1.getAdl().isNeighbor(ADJACENT_1));
}

BOOST_AUTO_TEST_CASE(CoordinateLsaConstructorAndGetters)
{
//For CoordinateLsa, lsType is 3.
  CoordinateLsa clsa1("router1", 3, 12, 1800, 2.5, 30.0);
  CoordinateLsa clsa2("router1", 3, 12, 1800, 2.5, 30.0);

  BOOST_CHECK_CLOSE(clsa1.getCorRadius(), 2.5, 0.0001);
  BOOST_CHECK_CLOSE(clsa1.getCorTheta(), 30.0, 0.0001);

  BOOST_CHECK(clsa1.isEqualContent(clsa2));

  BOOST_CHECK_EQUAL(clsa1.getData(), clsa2.getData());
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
