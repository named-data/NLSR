/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "adjacency-list.hpp"
#include "adjacent.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

using namespace std;

BOOST_AUTO_TEST_SUITE(TestAdjacenctList)

BOOST_AUTO_TEST_CASE(AdjacenctListBasic)
{
  const string ADJ_NAME_1 = "testname";
  const string ADJ_NAME_2 = "testname2";

//adjacent needed to test adjacency list.
  Adjacent adjacent1(ADJ_NAME_1);
  Adjacent adjacent2(ADJ_NAME_2);

  adjacent1.setLinkCost(4);
  adjacent2.setLinkCost(5);

  AdjacencyList adjacentList1;
  AdjacencyList adjacentList2;

  adjacentList1.insert(adjacent1);
  adjacentList2.insert(adjacent2);

  BOOST_CHECK_EQUAL(adjacentList1.getSize(), (uint32_t)1);
  BOOST_CHECK_EQUAL(adjacentList1 == adjacentList2, false);

  BOOST_CHECK(adjacentList1.isNeighbor("testname"));
  BOOST_CHECK_EQUAL(adjacentList1.isNeighbor("adjacent"), false);

  string n1 = "testname";
  BOOST_CHECK_EQUAL(adjacentList1.getStatusOfNeighbor(n1), (uint32_t)0);

  adjacentList1.setStatusOfNeighbor(n1, 1);
  BOOST_CHECK_EQUAL(adjacentList1.getStatusOfNeighbor(n1), (uint32_t)1);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace tests
} //namespace nlsr
