/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "route/nexthop.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestNexthop)

BOOST_AUTO_TEST_CASE(NexthopSetAndGet)
{
  NextHop np1;

  np1.setConnectingFace(1);

  np1.setRouteCost(10.5);

  BOOST_CHECK_EQUAL(np1.getConnectingFace(), 1);
  BOOST_CHECK_CLOSE(np1.getRouteCost(), 10.5, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
