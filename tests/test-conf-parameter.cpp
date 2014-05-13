/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

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

  BOOST_CHECK_EQUAL(cp1.getInterestRetryNumber(), (uint32_t)2);

  BOOST_CHECK_EQUAL(cp1.getInterestResendTime(), 1000);

  BOOST_CHECK_EQUAL(cp1.getLsaRefreshTime(), 1500);

  BOOST_CHECK_EQUAL(cp1.getRouterDeadInterval(), 10);

  BOOST_CHECK_EQUAL(cp1.getMaxFacesPerPrefix(), 50);

  BOOST_CHECK_EQUAL(cp1.getHyperbolicState(), 1);

  BOOST_CHECK_CLOSE(cp1.getCorTheta(), 102.5, 0.0001);

  BOOST_CHECK_EQUAL(cp1.getInfoInterestInterval(), 3);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
