/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "route/map.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestMap)

BOOST_AUTO_TEST_CASE(MapAddElementAndSize)
{
  Map map1;

  std::string router1 = "r1";
  std::string router2 = "r2";

  map1.addElement(router1);
  map1.addElement(router2);

  BOOST_CHECK_EQUAL(map1.getMapSize(), 2);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
