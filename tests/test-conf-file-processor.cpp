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

#include "test-common.hpp"
#include "dummy-face.hpp"

#include <fstream>
#include "conf-file-processor.hpp"
#include "nlsr.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {
namespace test {

using ndn::DummyFace;
using ndn::shared_ptr;

const std::string SECTION_GENERAL =
  "general\n"
  "{\n"
  "  network /ndn/\n"
  "  site /memphis.edu/\n"
  "  router /cs/pollux/\n"
  "  lsa-refresh-time 1800\n"
  "  lsa-interest-lifetime 3\n"
  "  log-level  INFO\n"
  "  log-dir /tmp\n"
  "  seq-dir /tmp\n"
  "}\n\n";

const std::string SECTION_NEIGHBORS =
  "neighbors\n"
  "{\n"
  "  hello-retries 3\n"
  "  hello-timeout 1\n"
  "  hello-interval  60\n\n"
  "  adj-lsa-build-interval 3\n"
  "  first-hello-interval  6\n"
  "  neighbor\n"
  "  {\n"
  "    name /ndn/memphis.edu/cs/castor\n"
  "    face-uri  udp4://localhost\n"
  "    link-cost 20\n"
  "  }\n\n"
  "  neighbor\n"
  "  {\n"
  "    name /ndn/memphis.edu/cs/mira\n"
  "    face-uri  udp4://localhost\n"
  "    link-cost 30\n"
  "  }\n"
  "}\n\n";

const std::string SECTION_HYPERBOLIC_ON =
  "hyperbolic\n"
  "{\n"
  "  state on\n"
  "  radius   123.456\n"
  "  angle    1.45\n"
  "}\n\n";

const std::string SECTION_HYPERBOLIC_OFF =
  "hyperbolic\n"
  "{\n"
  "  state off\n"
  "  radius   123.456\n"
  "  angle    1.45\n"
  "}\n\n";

const std::string SECTION_FIB =
  "fib\n"
  "{\n"
  "   max-faces-per-prefix 3\n"
  "   routing-calc-interval 9\n"
  "}\n\n";

const std::string SECTION_ADVERTISING =
  "advertising\n"
  "{\n"
  "  prefix /ndn/edu/memphis/cs/netlab\n"
  "  prefix /ndn/edu/memphis/sports/basketball\n"
  "}\n";

const std::string CONFIG_LINK_STATE = SECTION_GENERAL + SECTION_NEIGHBORS +
                                      SECTION_HYPERBOLIC_OFF + SECTION_FIB + SECTION_ADVERTISING;

const std::string CONFIG_HYPERBOLIC = SECTION_GENERAL + SECTION_NEIGHBORS +
                                      SECTION_HYPERBOLIC_ON + SECTION_FIB + SECTION_ADVERTISING;

class ConfFileProcessorFixture : public BaseFixture
{
public:
  ConfFileProcessorFixture()
    : face(ndn::makeDummyFace())
    , nlsr(g_ioService, g_scheduler, ndn::ref(*face))
    , CONFIG_FILE("unit-test-nlsr.conf")
  {
  }

  ~ConfFileProcessorFixture()
  {
    remove("unit-test-nlsr.conf");
  }

  bool processConfigurationString(std::string confString)
  {
    std::ofstream config;
    config.open("unit-test-nlsr.conf");
    config << confString;
    config.close();

    ConfFileProcessor processor(nlsr, CONFIG_FILE);
    return processor.processConfFile();
  }

public:
  shared_ptr<ndn::DummyFace> face;
  Nlsr nlsr;

private:
  const std::string CONFIG_FILE;
};

BOOST_FIXTURE_TEST_SUITE(TestConfFileProcessor, ConfFileProcessorFixture)

BOOST_AUTO_TEST_CASE(LinkState)
{
  processConfigurationString(CONFIG_LINK_STATE);

  ConfParameter& conf = nlsr.getConfParameter();
  conf.buildRouterPrefix();

  // General
  BOOST_CHECK_EQUAL(conf.getNetwork(), "/ndn/");
  BOOST_CHECK_EQUAL(conf.getSiteName(), "/memphis.edu/");
  BOOST_CHECK_EQUAL(conf.getRouterName(), "/cs/pollux/");
  BOOST_CHECK_EQUAL(conf.getRouterPrefix(), "/ndn/memphis.edu/cs/pollux/");
  BOOST_CHECK_EQUAL(conf.getChronosyncPrefix(), "/ndn/NLSR/sync");
  BOOST_CHECK_EQUAL(conf.getLsaPrefix(), "/ndn/NLSR/LSA");
  BOOST_CHECK_EQUAL(conf.getLsaRefreshTime(), 1800);
  BOOST_CHECK_EQUAL(conf.getLsaInterestLifetime(), ndn::time::seconds(3));
  BOOST_CHECK_EQUAL(conf.getRouterDeadInterval(), 3600);
  BOOST_CHECK_EQUAL(conf.getLogLevel(), "INFO");
  BOOST_CHECK_EQUAL(conf.getLogDir(), "/tmp");
  BOOST_CHECK_EQUAL(conf.getSeqFileDir(), "/tmp");

  // Neighbors
  BOOST_CHECK_EQUAL(conf.getInterestRetryNumber(), 3);
  BOOST_CHECK_EQUAL(conf.getInterestResendTime(), 1);
  BOOST_CHECK_EQUAL(conf.getInfoInterestInterval(), 60);

  BOOST_CHECK_EQUAL(conf.getAdjLsaBuildInterval(), 3);
  BOOST_CHECK_EQUAL(conf.getFirstHelloInterval(), 6);

  BOOST_CHECK(nlsr.getAdjacencyList().isNeighbor("/ndn/memphis.edu/cs/mira"));
  BOOST_CHECK(nlsr.getAdjacencyList().isNeighbor("/ndn/memphis.edu/cs/castor"));
  BOOST_CHECK(!nlsr.getAdjacencyList().isNeighbor("/ndn/memphis.edu/cs/fail"));

  Adjacent mira = nlsr.getAdjacencyList().getAdjacent("/ndn/memphis.edu/cs/mira");
  BOOST_CHECK_EQUAL(mira.getName(), "/ndn/memphis.edu/cs/mira");
  BOOST_CHECK_EQUAL(mira.getLinkCost(), 30);

  Adjacent castor = nlsr.getAdjacencyList().getAdjacent("/ndn/memphis.edu/cs/castor");
  BOOST_CHECK_EQUAL(castor.getName(), "/ndn/memphis.edu/cs/castor");
  BOOST_CHECK_EQUAL(castor.getLinkCost(), 20);

  // Hyperbolic
  BOOST_CHECK_EQUAL(conf.getHyperbolicState(), 0);

  // FIB
  BOOST_CHECK_EQUAL(conf.getMaxFacesPerPrefix(), 3);
  BOOST_CHECK_EQUAL(conf.getRoutingCalcInterval(), 9);

  // Advertising
  BOOST_CHECK_EQUAL(nlsr.getNamePrefixList().getSize(), 2);
}

BOOST_AUTO_TEST_CASE(Hyperbolic)
{
  processConfigurationString(CONFIG_HYPERBOLIC);

  ConfParameter& conf = nlsr.getConfParameter();
  BOOST_CHECK_EQUAL(conf.getHyperbolicState(), 1);
  BOOST_CHECK_EQUAL(conf.getCorR(), 123.456);
  BOOST_CHECK_EQUAL(conf.getCorTheta(), 1.45);
}

BOOST_AUTO_TEST_CASE(DefaultValues)
{
  // Missing adj-lsa-build-interval
  const std::string SECTION_NEIGHBORS_DEFAULT_VALUES =
  "neighbors\n"
  "{\n"
  "  hello-retries 3\n"
  "  hello-timeout 1\n"
  "  hello-interval  60\n\n"
  "  first-hello-interval  6\n"
  "  neighbor\n"
  "  {\n"
  "    name /ndn/memphis.edu/cs/castor\n"
  "    face-uri  udp4://localhost\n"
  "    link-cost 20\n"
  "  }\n\n"
  "  neighbor\n"
  "  {\n"
  "    name /ndn/memphis.edu/cs/mira\n"
  "    face-uri  udp4://localhost\n"
  "    link-cost 30\n"
  "  }\n"
  "}\n\n";

  processConfigurationString(SECTION_NEIGHBORS_DEFAULT_VALUES);

  ConfParameter& conf = nlsr.getConfParameter();

  BOOST_CHECK_EQUAL(conf.getAdjLsaBuildInterval(),
                    static_cast<uint32_t>(ADJ_LSA_BUILD_INTERVAL_DEFAULT));

  BOOST_CHECK_EQUAL(conf.getFirstHelloInterval(), 6);
}

BOOST_AUTO_TEST_CASE(OutOfRangeValue)
{
  const std::string SECTION_FIB_OUT_OF_RANGE =
  "fib\n"
  "{\n"
  "   max-faces-per-prefix 3\n"
  "   routing-calc-interval 999\n" // Larger than max value
  "}\n\n";

  // Processing should fail due to out of range value
  BOOST_CHECK_EQUAL(processConfigurationString(SECTION_FIB_OUT_OF_RANGE), false);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
