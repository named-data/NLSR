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
#include "conf-parameter.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

using namespace std;

BOOST_AUTO_TEST_SUITE(TestConfParameter)

BOOST_AUTO_TEST_CASE(ConfParameterSettersAndGetters)
{
  ConfParameter cp1;

  const string NAME = "router1";
  const string SITE = "memphis";
  const string NETWORK = "ATT";

  cp1.setRouterName(NAME);

  cp1.setSiteName(SITE);

  cp1.setNetwork(NETWORK);

  cp1.setInterestRetryNumber(2);

  cp1.setInterestResendTime(1000);

  cp1.setLsaRefreshTime(1500);

  cp1.setLsaInterestLifetime(ndn::time::seconds(1));

  cp1.setRouterDeadInterval(10);

  cp1.setMaxFacesPerPrefix(50);

  cp1.setHyperbolicState(1);

  cp1.setCorR(2.5);

  cp1.setCorTheta(102.5);

  cp1.setInfoInterestInterval(3);

  BOOST_CHECK_EQUAL(cp1.getRouterName(), "router1");

  BOOST_CHECK_EQUAL(cp1.getSiteName(), "memphis");

  BOOST_CHECK_EQUAL(cp1.getNetwork(), "ATT");

  cp1.buildRouterPrefix();

  BOOST_CHECK_EQUAL(cp1.getRouterPrefix(), "/ATT/memphis/router1");

  BOOST_CHECK_EQUAL(cp1.getInterestRetryNumber(), 2);

  BOOST_CHECK_EQUAL(cp1.getInterestResendTime(), 1000);

  BOOST_CHECK_EQUAL(cp1.getLsaRefreshTime(), 1500);

  BOOST_CHECK_EQUAL(cp1.getLsaInterestLifetime(), ndn::time::seconds(1));

  BOOST_CHECK_EQUAL(cp1.getRouterDeadInterval(), 10);

  BOOST_CHECK_EQUAL(cp1.getMaxFacesPerPrefix(), 50);

  BOOST_CHECK_EQUAL(cp1.getHyperbolicState(), 1);

  BOOST_CHECK_CLOSE(cp1.getCorTheta(), 102.5, 0.0001);

  BOOST_CHECK_EQUAL(cp1.getInfoInterestInterval(), 3);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
