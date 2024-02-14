/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
 *                           Regents of the University of California
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
 */

#include "route/name-map.hpp"

#include "tests/boost-test.hpp"

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestMap)

BOOST_AUTO_TEST_CASE(Basic)
{
  NameMap map1;

  ndn::Name name1("/r1");
  ndn::Name name2("/r2");
  ndn::Name name3("/r3");

  map1.addEntry(name1);
  map1.addEntry(name2);
  BOOST_CHECK_EQUAL(map1.size(), 2);

  std::optional<int32_t> mn1 = map1.getMappingNoByRouterName(name1);
  std::optional<int32_t> mn2 = map1.getMappingNoByRouterName(name2);
  BOOST_REQUIRE(mn1.has_value());
  BOOST_REQUIRE(mn2.has_value());
  BOOST_CHECK_NE(*mn1, *mn2);
  BOOST_CHECK_EQUAL(map1.getMappingNoByRouterName(name3).has_value(), false);

  BOOST_CHECK_EQUAL(map1.getRouterNameByMappingNo(*mn1).value_or(ndn::Name()), name1);
  BOOST_CHECK_EQUAL(map1.getRouterNameByMappingNo(*mn2).value_or(ndn::Name()), name2);

  int32_t mn3 = 3333;
  BOOST_CHECK_NE(mn3, *mn1);
  BOOST_CHECK_NE(mn3, *mn2);
  BOOST_CHECK_EQUAL(map1.getRouterNameByMappingNo(mn3).has_value(), false);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
