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

#include "conf-parameter.hpp"
#include "tests/boost-test.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestConfParameter)

BOOST_AUTO_TEST_CASE(ConfParameterSettersAndGetters)
{
  ndn::DummyClientFace face;
  ndn::KeyChain keyChain;
  ConfParameter cp1(face, keyChain);

  cp1.setRouterName("router1");
  cp1.setSiteName("memphis");
  cp1.setNetwork("ATT");
  cp1.setInterestRetryNumber(2);
  cp1.setInterestResendTime(1000);
  cp1.setLsaRefreshTime(1500);
  cp1.setLsaInterestLifetime(ndn::time::seconds(1));
  cp1.setSyncProtocol(SyncProtocol::PSYNC);
  cp1.setRouterDeadInterval(10);
  cp1.setMaxFacesPerPrefix(50);
  cp1.setHyperbolicState(HYPERBOLIC_STATE_ON);
  cp1.setCorR(2.5);
  std::vector<double> angles = {102.5};
  cp1.setCorTheta(angles);
  cp1.setInfoInterestInterval(3);

  BOOST_CHECK_EQUAL(cp1.getRouterName(), "router1");
  BOOST_CHECK_EQUAL(cp1.getSiteName(), "memphis");
  BOOST_CHECK_EQUAL(cp1.getNetwork(), "ATT");

  cp1.buildRouterAndSyncUserPrefix();

  BOOST_CHECK_EQUAL(cp1.getRouterPrefix(), "/ATT/memphis/router1");
  BOOST_CHECK_EQUAL(cp1.getSyncUserPrefix(), "/localhop/ATT/nlsr/LSA/memphis/router1");
  BOOST_CHECK_EQUAL(cp1.getInterestRetryNumber(), 2);
  BOOST_CHECK_EQUAL(cp1.getInterestResendTime(), 1000);
  BOOST_CHECK_EQUAL(cp1.getLsaRefreshTime(), 1500);
  BOOST_CHECK_EQUAL(cp1.getLsaInterestLifetime(), ndn::time::seconds(1));
  BOOST_CHECK(cp1.getSyncProtocol() == SyncProtocol::PSYNC);
  BOOST_CHECK_EQUAL(cp1.getRouterDeadInterval(), 10);
  BOOST_CHECK_EQUAL(cp1.getMaxFacesPerPrefix(), 50);
  BOOST_CHECK_EQUAL(cp1.getHyperbolicState(), HYPERBOLIC_STATE_ON);
  BOOST_CHECK(cp1.getCorTheta() == angles);
  BOOST_CHECK_EQUAL(cp1.getInfoInterestInterval(), 3);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
