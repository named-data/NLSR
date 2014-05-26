/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "lsdb.hpp"
#include "nlsr.hpp"
#include "lsa.hpp"
#include "name-prefix-list.hpp"
#include <boost/test/unit_test.hpp>
#include <ndn-cxx/util/time.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestLsdb)

BOOST_AUTO_TEST_CASE(LsdbRemoveAndExists)
{
  Nlsr nlsr1;
  ndn::time::system_clock::TimePoint testTimePoint =  ndn::time::system_clock::now();
  NamePrefixList npl1;

  std::string s1 = "name1";
  std::string s2 = "name2";
  std::string router1 = "router1/1";

  npl1.insert(s1);
  npl1.insert(s2);

//For NameLsa lsType is name.
//12 is seqNo, randomly generated.
//1800 is the default life time.
  NameLsa nlsa1("router1", std::string("name"), 12, testTimePoint, npl1);

  Lsdb lsdb1(nlsr1);

  lsdb1.installNameLsa(nlsa1);

  BOOST_CHECK(lsdb1.doesLsaExist("router1/1", std::string("name")));

  lsdb1.removeNameLsa(router1);

  BOOST_CHECK_EQUAL(lsdb1.doesLsaExist("router1/1", std::string("name")), false);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
