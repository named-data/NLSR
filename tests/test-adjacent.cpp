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

#include "adjacent.hpp"
#include "tests/boost-test.hpp"

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestAdjacent)

BOOST_AUTO_TEST_CASE(OperatorEquals)
{
  const ndn::Name ADJ_NAME_1 = "name1";
  const ndn::FaceUri ADJ_URI_1 = ndn::FaceUri("udp4://10.0.0.1:8000");
  const double ADJ_LINK_COST_1 = 1;
  Adjacent adjacent1(ADJ_NAME_1);
  Adjacent adjacent2(ADJ_NAME_1);
  adjacent1.setFaceUri(ADJ_URI_1);
  adjacent2.setFaceUri(ADJ_URI_1);
  adjacent1.setLinkCost(ADJ_LINK_COST_1);
  adjacent2.setLinkCost(ADJ_LINK_COST_1);

  BOOST_CHECK(adjacent1 == adjacent2);
}

BOOST_AUTO_TEST_CASE(OperatorLessThan)
{
  const ndn::Name ADJ_NAME_1 = "name1";
  const double ADJ_LINK_COST_1 = 1;
  const ndn::Name ADJ_NAME_2 = "name2";
  const double ADJ_LINK_COST_2 = 2;
  Adjacent adjacent1(ADJ_NAME_1);
  Adjacent adjacent2(ADJ_NAME_1);
  adjacent1.setLinkCost(ADJ_LINK_COST_1);
  adjacent2.setLinkCost(ADJ_LINK_COST_2);

  BOOST_CHECK(adjacent1 < adjacent2);

  Adjacent adjacent3(ADJ_NAME_2);
  adjacent3.setLinkCost(ADJ_LINK_COST_1);

  BOOST_CHECK(adjacent1 < adjacent3);
}

BOOST_AUTO_TEST_CASE(Accessors)
{
  const ndn::Name ADJ_NAME_1 = "name1";
  Adjacent adjacent1(ADJ_NAME_1);

  // Link cost should always be rounded up to the nearest integer.
  // The library only acceps integral values for prefix registration.
  adjacent1.setLinkCost(10.1);

  BOOST_CHECK_EQUAL(adjacent1.getName(), "name1");
  BOOST_CHECK_EQUAL(adjacent1.getLinkCost(), 11);
}

BOOST_AUTO_TEST_CASE(CompareFaceUri)
{
  const ndn::Name ADJ_NAME_1 = "name1";
  const ndn::Name ADJ_NAME_2 = "name2";
  const ndn::FaceUri ADJ_URI_1 = ndn::FaceUri("udp4://10.0.0.1:8000");
  Adjacent adjacent1(ADJ_NAME_1);
  Adjacent adjacent2(ADJ_NAME_2);
  adjacent1.setFaceUri(ADJ_URI_1);
  adjacent2.setFaceUri(ADJ_URI_1);

  BOOST_CHECK(adjacent1.compareFaceUri(adjacent2.getFaceUri()));
}

BOOST_AUTO_TEST_CASE(CompareFaceId)
{
  const ndn::Name ADJ_NAME_1 = "name1";
  const ndn::Name ADJ_NAME_2 = "name2";
  const uint64_t ADJ_FACEID_1 = 1;
  Adjacent adjacent1(ADJ_NAME_1);
  Adjacent adjacent2(ADJ_NAME_2);
  adjacent1.setFaceId(ADJ_FACEID_1);
  adjacent2.setFaceId(ADJ_FACEID_1);

  BOOST_CHECK(adjacent1.compareFaceId(adjacent2.getFaceId()));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
