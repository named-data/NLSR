/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "route/map.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestMapEntry)

BOOST_AUTO_TEST_CASE(MapEntryConstructorAndGetters)
{
  std::string rtr = "r0";

  MapEntry me1(rtr, 1);

  BOOST_CHECK_EQUAL(me1.getRouter(), "r0");

  BOOST_CHECK_EQUAL(me1.getMappingNumber(), 1);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
