/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
 *
 * This file is part of NLSR (Named-data Link State Routing).
 * See AUTHORS.md for complete list of NLSR authors and contributors.
 *
 * NLSR is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Ashlesh Gawande <agawande@memphis.edu>
 *
 **/
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
  INIT_LOGGERS("/tmp", "DEBUG");

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
  NameLsa nlsa1(ndn::Name("/router1/1"), std::string("name"), 12, testTimePoint, npl1);

  Lsdb lsdb1(nlsr1);

  lsdb1.installNameLsa(nlsa1);
  lsdb1.writeNameLsdbLog();

  BOOST_CHECK(lsdb1.doesLsaExist(ndn::Name("/router1/1/name"), std::string("name")));

  lsdb1.removeNameLsa(router1);

  BOOST_CHECK_EQUAL(lsdb1.doesLsaExist(ndn::Name("/router1/1"), std::string("name")), false);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
