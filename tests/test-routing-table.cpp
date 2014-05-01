/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "route/routing-table.hpp"
#include "route/routing-table-entry.hpp"
#include "route/nexthop.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestRoutingTable)

BOOST_AUTO_TEST_CASE(RoutingTableAddNextHop)
{
  RoutingTable rt1;

  NextHop nh1;

  const std::string DEST_ROUTER = "destRouter";

  rt1.addNextHop("destRouter", nh1);

  BOOST_CHECK_EQUAL(rt1.findRoutingTableEntry(DEST_ROUTER)->getDestination(),
                    "destRouter");
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
