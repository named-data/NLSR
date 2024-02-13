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

#include "route/name-prefix-table-entry.hpp"

#include "tests/boost-test.hpp"

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestNpte)

BOOST_AUTO_TEST_CASE(NpteConstructorAndNamePrefix)
{
  NamePrefixTableEntry npte1("/ndn/memphis.edu/cs");

  BOOST_CHECK_EQUAL(npte1.getNamePrefix(), "/ndn/memphis.edu/cs");
}

BOOST_AUTO_TEST_CASE(AddRoutingTableEntry)
{
  NamePrefixTableEntry npte1("/ndn/memphis/rtr1");
  RoutingTablePoolEntry rtpe1("/ndn/memphis/rtr2", 0);
  auto rtpePtr = std::make_shared<RoutingTablePoolEntry>(rtpe1);

  BOOST_CHECK_EQUAL(npte1.m_rteList.size(), 0);
  npte1.addRoutingTableEntry(rtpePtr);
  BOOST_CHECK_EQUAL(npte1.m_rteList.size(), 1);

  auto itr = std::find(npte1.m_rteList.begin(), npte1.m_rteList.end(), rtpePtr);
  BOOST_CHECK_EQUAL(rtpePtr, *itr);
}

BOOST_AUTO_TEST_CASE(RemoveRoutingTableEntry)
{
  NamePrefixTableEntry npte1("/ndn/memphis/rtr1");
  RoutingTablePoolEntry rtpe1("/ndn/memphis/rtr2", 0);
  auto rtpePtr = std::make_shared<RoutingTablePoolEntry>(rtpe1);

  npte1.addRoutingTableEntry(rtpePtr);
  npte1.removeRoutingTableEntry(rtpePtr);

  int count = 0;
  for (auto&& rte : npte1.m_rteList) {
    if (*rte == rtpe1) {
      count++;
    }
  }

  BOOST_CHECK_EQUAL(count, 0);
}

BOOST_AUTO_TEST_CASE(EqualsOperatorTwoObj)
{
  NamePrefixTableEntry npte1("/ndn/memphis/rtr1");
  NamePrefixTableEntry npte2("/ndn/memphis/rtr1");

  BOOST_CHECK_EQUAL(npte1, npte2);
}

BOOST_AUTO_TEST_CASE(EqualsOperatorOneObjOneName)
{
  NamePrefixTableEntry npte1("/ndn/memphis/rtr1");

  BOOST_CHECK_EQUAL(npte1, "/ndn/memphis/rtr1");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
