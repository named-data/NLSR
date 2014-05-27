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
#include <ndn-cxx/util/time.hpp>
#include "route/fib-entry.hpp"
#include "route/nexthop-list.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestFibEntry)

BOOST_AUTO_TEST_CASE(FibEntryConstructorAndGetters)
{
  ndn::time::system_clock::TimePoint testTimePoint =  ndn::time::system_clock::now();
  FibEntry fe1("next1");
  fe1.setExpirationTimePoint(testTimePoint);

  BOOST_CHECK_EQUAL(fe1.getName(), "next1");
  BOOST_CHECK_EQUAL(fe1.getExpirationTimePoint(), testTimePoint);
  BOOST_CHECK_EQUAL(fe1.getSeqNo(), 0);           //Default Seq No.
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
