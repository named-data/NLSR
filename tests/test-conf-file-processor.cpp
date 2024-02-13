/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
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
 **/

#include "conf-file-processor.hpp"

#include "tests/boost-test.hpp"
#include "tests/io-key-chain-fixture.hpp"

#include <fstream>
#include <boost/algorithm/string.hpp>

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr::tests {

const std::string SECTION_GENERAL =
  "general\n"
  "{\n"
  "  network /ndn\n"
  "  site /memphis.edu\n"
  "  router /cs/pollux\n"
  "  lsa-refresh-time 1800\n"
  "  lsa-interest-lifetime 3\n"
  "  router-dead-interval 86400\n"
  "  sync-protocol psync\n"
  "  sync-interest-lifetime 10000\n"
  "  state-dir /tmp\n"
  "}\n\n";

const std::string SECTION_GENERAL_SVS =
  "general\n"
  "{\n"
  "  network /ndn\n"
  "  site /memphis.edu\n"
  "  router /cs/pollux\n"
  "  sync-protocol svs\n"
  "  state-dir /tmp\n"
  "}\n\n";

const std::string SECTION_NEIGHBORS =
  "neighbors\n"
  "{\n"
  "  hello-retries 3\n"
  "  hello-timeout 1\n"
  "  hello-interval  60\n\n"
  "  adj-lsa-build-interval 10\n"
  "  neighbor\n"
  "  {\n"
  "    name /ndn/memphis.edu/cs/castor\n"
  "    face-uri  udp://10.0.0.1\n"
  "    link-cost 20\n"
  "  }\n\n"
  "  neighbor\n"
  "  {\n"
  "    name /ndn/memphis.edu/cs/mira\n"
  "    face-uri  udp://10.0.0.2\n"
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

const std::string SECTION_HYPERBOLIC_ANGLES_ON =
    "hyperbolic\n"
    "{\n"
    "  state on\n"
    "  radius   123.456\n"
    "  angle    1.45,2.25\n"
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

// NEED TO TEST SECURITY SECTION SUCH AS LOADING CERTIFICATE

const std::string CONFIG_LINK_STATE = SECTION_GENERAL + SECTION_NEIGHBORS +
                                      SECTION_HYPERBOLIC_OFF + SECTION_FIB + SECTION_ADVERTISING;

const std::string CONFIG_HYPERBOLIC = SECTION_GENERAL + SECTION_NEIGHBORS +
                                      SECTION_HYPERBOLIC_ON + SECTION_FIB + SECTION_ADVERTISING;

const std::string CONFIG_HYPERBOLIC_ANGLES = SECTION_GENERAL + SECTION_NEIGHBORS +
                                             SECTION_HYPERBOLIC_ANGLES_ON + SECTION_FIB +
                                             SECTION_ADVERTISING;

const std::string CONFIG_SVS = SECTION_GENERAL_SVS;

class ConfFileProcessorFixture : public IoKeyChainFixture
{
public:
  ConfFileProcessorFixture()
    : face(m_io, m_keyChain)
    , conf(face, m_keyChain, "unit-test-nlsr.conf")
  {
  }

  ~ConfFileProcessorFixture() override
  {
    remove("unit-test-nlsr.conf");
  }

  bool
  processConfigurationString(std::string_view confString)
  {
    std::ofstream config;
    config.open("unit-test-nlsr.conf");
    config << confString;
    config.close();

    ConfFileProcessor processor(conf);
    return processor.processConfFile();
  }

  void
  commentOut(const std::string& key, std::string& config)
  {
    boost::replace_all(config, key, ";" + key);
  }

public:
  ndn::DummyClientFace face;
  ConfParameter conf;
};

BOOST_FIXTURE_TEST_SUITE(TestConfFileProcessor, ConfFileProcessorFixture)

BOOST_AUTO_TEST_CASE(LinkState)
{
  BOOST_REQUIRE(processConfigurationString(CONFIG_LINK_STATE));
  conf.buildRouterAndSyncUserPrefix();

  // General
  BOOST_CHECK_EQUAL(conf.getNetwork(), "/ndn");
  BOOST_CHECK_EQUAL(conf.getSiteName(), "/memphis.edu");
  BOOST_CHECK_EQUAL(conf.getRouterName(), "/cs/pollux");
  BOOST_CHECK_EQUAL(conf.getRouterPrefix(), "/ndn/memphis.edu/cs/pollux");
  BOOST_CHECK_EQUAL(conf.getSyncPrefix(), ndn::Name("/localhop/ndn/nlsr/sync").appendVersion(ConfParameter::SYNC_VERSION));
  BOOST_CHECK_EQUAL(conf.getLsaPrefix(), "/localhop/ndn/nlsr/LSA");
  BOOST_CHECK_EQUAL(conf.getLsaRefreshTime(), 1800);
  BOOST_CHECK(conf.getSyncProtocol() == SyncProtocol::PSYNC);
  BOOST_CHECK_EQUAL(conf.getLsaInterestLifetime(), ndn::time::seconds(3));
  BOOST_CHECK_EQUAL(conf.getRouterDeadInterval(), 86400);
  BOOST_CHECK_EQUAL(conf.getSyncInterestLifetime(), ndn::time::milliseconds(10000));
  BOOST_CHECK_EQUAL(conf.getStateFileDir(), "/tmp");

  // Neighbors
  BOOST_CHECK_EQUAL(conf.getInterestRetryNumber(), 3);
  BOOST_CHECK_EQUAL(conf.getInterestResendTime(), 1);
  BOOST_CHECK_EQUAL(conf.getInfoInterestInterval(), 60);

  BOOST_CHECK_EQUAL(conf.getAdjLsaBuildInterval(), 10);

  BOOST_CHECK(conf.getAdjacencyList().isNeighbor("/ndn/memphis.edu/cs/mira"));
  BOOST_CHECK(conf.getAdjacencyList().isNeighbor("/ndn/memphis.edu/cs/castor"));
  BOOST_CHECK(!conf.getAdjacencyList().isNeighbor("/ndn/memphis.edu/cs/fail"));

  Adjacent mira = conf.getAdjacencyList().getAdjacent("/ndn/memphis.edu/cs/mira");
  BOOST_CHECK_EQUAL(mira.getName(), "/ndn/memphis.edu/cs/mira");
  BOOST_CHECK_EQUAL(mira.getLinkCost(), 30);
  BOOST_CHECK_EQUAL(mira.getFaceUri().toString(), "udp4://10.0.0.2:6363");

  Adjacent castor = conf.getAdjacencyList().getAdjacent("/ndn/memphis.edu/cs/castor");
  BOOST_CHECK_EQUAL(castor.getName(), "/ndn/memphis.edu/cs/castor");
  BOOST_CHECK_EQUAL(castor.getLinkCost(), 20);
  BOOST_CHECK_EQUAL(castor.getFaceUri().toString(), "udp4://10.0.0.1:6363");

  // Hyperbolic
  BOOST_CHECK_EQUAL(conf.getHyperbolicState(), HYPERBOLIC_STATE_OFF);

  // FIB
  BOOST_CHECK_EQUAL(conf.getMaxFacesPerPrefix(), 3);
  BOOST_CHECK_EQUAL(conf.getRoutingCalcInterval(), 9);

  // Advertising
  BOOST_CHECK_EQUAL(conf.getNamePrefixList().size(), 2);
}

BOOST_AUTO_TEST_CASE(SvsPrefix)
{
#ifdef HAVE_SVS
  BOOST_REQUIRE(processConfigurationString(CONFIG_SVS));
  conf.buildRouterAndSyncUserPrefix();

  // SVS does not use localhop
  BOOST_CHECK_EQUAL(conf.getNetwork(), "/ndn");
  BOOST_CHECK_EQUAL(conf.getSyncPrefix(), ndn::Name("/ndn/nlsr/sync").appendVersion(ConfParameter::SYNC_VERSION));
#else
  BOOST_CHECK_EQUAL(processConfigurationString(CONFIG_SVS), false);
#endif
}

BOOST_AUTO_TEST_CASE(MalformedUri)
{
  const std::string MALFORMED_URI =
    "neighbors\n"
    "{\n"
    "  hello-retries 3\n"
    "  hello-timeout 1\n"
    "  hello-interval  60\n\n"
    "  adj-lsa-build-interval 10\n"
    "  neighbor\n"
    "  {\n"
    "    name /ndn/memphis.edu/cs/castor\n"
    "    face-uri  udp4:malformed-uri\n"
    "    link-cost 20\n"
    "  }\n"
    "}\n\n";

  BOOST_CHECK_EQUAL(processConfigurationString(MALFORMED_URI), false);
}

BOOST_AUTO_TEST_CASE(Hyperbolic)
{
  BOOST_REQUIRE(processConfigurationString(CONFIG_HYPERBOLIC));

  BOOST_CHECK_EQUAL(conf.getHyperbolicState(), HYPERBOLIC_STATE_ON);
  BOOST_CHECK_EQUAL(conf.getCorR(), 123.456);
  std::vector<double> angles{1.45};
  BOOST_CHECK(conf.getCorTheta() == angles);
}

BOOST_AUTO_TEST_CASE(Hyperbolic2)
{
  BOOST_REQUIRE(processConfigurationString(CONFIG_HYPERBOLIC_ANGLES));

  BOOST_CHECK_EQUAL(conf.getHyperbolicState(), HYPERBOLIC_STATE_ON);
  BOOST_CHECK_EQUAL(conf.getCorR(), 123.456);
  std::vector<double> angles{1.45, 2.25};
  BOOST_CHECK(conf.getCorTheta() == angles);
}

BOOST_AUTO_TEST_CASE(DefaultValuesGeneral)
{
  std::string config = SECTION_GENERAL;

  commentOut("lsa-refresh-time", config);
  commentOut("lsa-interest-lifetime", config);
  commentOut("router-dead-interval", config);

  BOOST_REQUIRE(processConfigurationString(config));

  BOOST_CHECK_EQUAL(conf.getLsaRefreshTime(), static_cast<uint32_t>(LSA_REFRESH_TIME_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getLsaInterestLifetime(),
                    static_cast<ndn::time::seconds>(LSA_INTEREST_LIFETIME_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getRouterDeadInterval(), (2 * conf.getLsaRefreshTime()));

  BOOST_CHECK_NE(conf.m_confFileName, conf.getConfFileNameDynamic());
  conf.m_confFileName = "/tmp/nlsr.conf";
  BOOST_CHECK_EQUAL(conf.m_confFileName, conf.getConfFileNameDynamic());
  BOOST_CHECK_EQUAL(processConfigurationString(config), false);
}

BOOST_AUTO_TEST_CASE(DefaultValuesNeighbors)
{
  std::string config = SECTION_NEIGHBORS;

  commentOut("hello-retries", config);
  commentOut("hello-timeout", config);
  commentOut("hello-interval", config);
  commentOut("first-hello-interval", config);
  commentOut("adj-lsa-build-interval", config);

  BOOST_REQUIRE(processConfigurationString(config));

  BOOST_CHECK_EQUAL(conf.getInterestRetryNumber(), static_cast<uint32_t>(HELLO_RETRIES_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getInterestResendTime(), static_cast<uint32_t>(HELLO_TIMEOUT_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getInfoInterestInterval(), static_cast<uint32_t>(HELLO_INTERVAL_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getAdjLsaBuildInterval(),
                    static_cast<uint32_t>(ADJ_LSA_BUILD_INTERVAL_DEFAULT));
}

BOOST_AUTO_TEST_CASE(CanonizeNeighbors)
{
  std::string config{R"INFO(neighbors
    {
      neighbor
      {
        name /ndn/edu/arizona/%C1.Router/hobo
        face-uri  udp4://hobo.cs.arizona.edu
        link-cost 20
      }
      neighbor
      {
        name /ndn/edu/umich/%C1.Router/ndn0
        face-uri  udp4://michigan.testbed.named-data.net
        link-cost 30
      }
    })INFO"};

  BOOST_REQUIRE(processConfigurationString(config));

  BOOST_CHECK_EQUAL(conf.m_adjl.getAdjacent("/ndn/edu/arizona/%C1.Router/hobo").getFaceUri().toString(),
                    "udp4://128.196.203.36:6363");
  BOOST_CHECK_EQUAL(conf.m_adjl.getAdjacent("/ndn/edu/umich/%C1.Router/ndn0").getFaceUri().toString(),
                    "udp4://198.111.224.197:6363");
}

BOOST_AUTO_TEST_CASE(DefaultValuesFib)
{
  std::string config = SECTION_FIB;

  commentOut("max-faces-per-prefix", config);
  commentOut("routing-calc-interval", config);

  BOOST_REQUIRE(processConfigurationString(config));

  BOOST_CHECK_EQUAL(conf.getMaxFacesPerPrefix(),
                    static_cast<uint32_t>(MAX_FACES_PER_PREFIX_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getRoutingCalcInterval(),
                    static_cast<uint32_t>(ROUTING_CALC_INTERVAL_DEFAULT));
}

BOOST_AUTO_TEST_CASE(DefaultValuesHyperbolic)
{
  std::string config = SECTION_HYPERBOLIC_ON;

  commentOut("state", config);

  BOOST_REQUIRE(processConfigurationString(config));

  BOOST_CHECK_EQUAL(conf.getHyperbolicState(), HYPERBOLIC_STATE_DEFAULT);
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

BOOST_AUTO_TEST_CASE(NegativeValue)
{
  const std::string SECTION_GENERAL_NEGATIVE_VALUE =
  "general\n"
  "{\n"
  "  network /ndn/\n"
  "  site /memphis.edu/\n"
  "  router /cs/pollux/\n"
  "  lsa-refresh-time -1800\n"
  "  lsa-interest-lifetime -3\n"
  "  router-dead-interval -86400\n"
  "}\n\n";

  // Processing should fail due to negative value
  BOOST_CHECK_EQUAL(processConfigurationString(SECTION_GENERAL_NEGATIVE_VALUE), false);
}

BOOST_AUTO_TEST_CASE(LoadCertToPublish)
{
  auto identity = m_keyChain.createIdentity("/TestNLSR/identity");
  saveIdentityCert(identity, "cert-to-publish.cert");

  const std::string SECTION_SECURITY = R"CONF(
      security
      {
        validator
        {
          trust-anchor
          {
            type any
          }
        }
        prefix-update-validator
        {
          trust-anchor
          {
            type any
          }
        }
        cert-to-publish "cert-to-publish.cert"
      }
    )CONF";

  BOOST_REQUIRE(processConfigurationString(SECTION_SECURITY));
}

BOOST_AUTO_TEST_CASE(PrefixUpdateValidatorOptional) // Bug #2814
{
  const std::string SECTION_SECURITY = R"CONF(
      security
      {
        validator
        {
          trust-anchor
          {
            type any
          }
        }
      }
    )CONF";

  BOOST_REQUIRE(processConfigurationString(SECTION_SECURITY));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
