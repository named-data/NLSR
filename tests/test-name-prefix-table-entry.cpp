/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "route/name-prefix-table-entry.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestNpte)

BOOST_AUTO_TEST_CASE(NpteConstructorAndNamePrefix)
{
  NamePrefixTableEntry npte1("/ndn/memphis.edu/cs");

  BOOST_CHECK_EQUAL(npte1.getNamePrefix(), "/ndn/memphis.edu/cs");
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
