/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "route/nexthop-list.hpp"
#include "route/nexthop.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestNhl)

BOOST_AUTO_TEST_CASE(NhlAddNextHop)
{
  NextHop np1;

  NexthopList nhl1;

  nhl1.addNextHop(np1);
  BOOST_CHECK_EQUAL(nhl1.getSize(), 1);

  nhl1.removeNextHop(np1);
  BOOST_CHECK_EQUAL(nhl1.getSize(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
