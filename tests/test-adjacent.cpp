/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "adjacent.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

using namespace std;

BOOST_AUTO_TEST_SUITE(TestAdjacenct)

BOOST_AUTO_TEST_CASE(AdjacenctBasic)
{
  const string ADJ_NAME_1 = "testname";
  const string ADJ_NAME_2 = "testname";

  Adjacent adjacent1(ADJ_NAME_1);
  Adjacent adjacent2(ADJ_NAME_2);
  BOOST_CHECK(adjacent1 == adjacent2);

  adjacent1.setLinkCost(10.5);
  BOOST_CHECK_CLOSE(adjacent1.getLinkCost(), 10.5, 0.0001);

  BOOST_CHECK_EQUAL(adjacent1.getName(), "testname");

}

BOOST_AUTO_TEST_SUITE_END()

} //namespace tests
} //namespace nlsr
