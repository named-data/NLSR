/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "route/routing-table-entry.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestRoutingTableEntry)

BOOST_AUTO_TEST_CASE(RoutingTableEntryDestination)
{
  RoutingTableEntry rte1("router1");

  BOOST_CHECK_EQUAL(rte1.getDestination(), "router1");
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
