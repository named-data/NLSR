/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "name-prefix-list.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestNpl)

BOOST_AUTO_TEST_CASE(NplSizeAndRemove)
{
  NamePrefixList npl1;

  std::string a = "testname";
  std::string b = "name";

  npl1.insert(a);
  npl1.insert(b);

  BOOST_CHECK_EQUAL(npl1.getSize(), 2);

  npl1.remove(b);

  BOOST_CHECK_EQUAL(npl1.getSize(), 1);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
