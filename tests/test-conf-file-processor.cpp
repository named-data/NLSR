/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
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

#include "test-common.hpp"
#include "logger.hpp"

#include <fstream>
#include "conf-file-processor.hpp"
#include "nlsr.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

using ndn::shared_ptr;

const std::string SECTION_GENERAL =
  "general\n"
  "{\n"
  "  network /ndn/\n"
  "  site /memphis.edu/\n"
  "  router /cs/pollux/\n"
  "  lsa-refresh-time 1800\n"
  "  lsa-interest-lifetime 3\n"
  "  router-dead-interval 86400\n"
  "  log-level  INFO\n"
  "  log-dir /tmp\n"
  "  seq-dir /tmp\n"
  "}\n\n";

const std::string LOG4CXX_PLACEHOLDER = "$LOG4CXX$";

const std::string SECTION_GENERAL_WITH_LOG4CXX =
  "general\n"
  "{\n"
  "  network /ndn/\n"
  "  site /memphis.edu/\n"
  "  router /cs/pollux/\n"
  "  lsa-refresh-time 1800\n"
  "  lsa-interest-lifetime 3\n"
  "  router-dead-interval 86400\n"
  "  log-level  INFO\n"
  "  log-dir /tmp\n"
  "  seq-dir /tmp\n"
  "  log4cxx-conf " + LOG4CXX_PLACEHOLDER + "\n"
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

const std::string CONFIG_LOG4CXX = SECTION_GENERAL_WITH_LOG4CXX;

const std::string CONFIG_HYPERBOLIC = SECTION_GENERAL + SECTION_NEIGHBORS +
                                      SECTION_HYPERBOLIC_ON + SECTION_FIB + SECTION_ADVERTISING;

class ConfFileProcessorFixture : public BaseFixture
{
public:
  ConfFileProcessorFixture()
    : face(make_shared<ndn::util::DummyClientFace>())
    , nlsr(g_ioService, g_scheduler, ndn::ref(*face))
    , CONFIG_FILE("unit-test-nlsr.conf")
    , m_logConfigFileName(boost::filesystem::unique_path().native())
    , m_logFileName(boost::filesystem::unique_path().native())
  {
  }

  ~ConfFileProcessorFixture()
  {
    remove("unit-test-nlsr.conf");
    remove("/tmp/unit-test-log4cxx.xml");

    boost::filesystem::remove(boost::filesystem::path(getLogConfigFileName()));
    boost::filesystem::remove(boost::filesystem::path(getLogFileName()));
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

  void
  verifyOutputLog4cxx(const std::string expected[], size_t nExpected)
  {
    std::ifstream is(getLogFileName().c_str());
    std::string buffer((std::istreambuf_iterator<char>(is)),
                       (std::istreambuf_iterator<char>()));

    std::vector<std::string> components;
    boost::split(components, buffer, boost::is_any_of(" ,\n"));

    // expected + number of timestamps (one per log statement) + trailing newline of last statement
    BOOST_REQUIRE_EQUAL(components.size(), nExpected);

    for (size_t i = 0; i < nExpected; ++i) {
      if (expected[i] == "")
        continue;

      BOOST_CHECK_EQUAL(components[i], expected[i]);
    }
  }

  const std::string&
  getLogConfigFileName()
  {
    return m_logConfigFileName;
  }

  const std::string&
  getLogFileName()
  {
    return m_logFileName;
  }

  void
  commentOut(const std::string& key, std::string& config)
  {
    boost::replace_all(config, key, ";" + key);
  }

public:
  shared_ptr<ndn::util::DummyClientFace> face;
  Nlsr nlsr;

private:
  const std::string CONFIG_FILE;
  std::string m_logConfigFileName;
  std::string m_logFileName;
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
  BOOST_CHECK_EQUAL(conf.getRouterDeadInterval(), 86400);
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

BOOST_AUTO_TEST_CASE(Log4cxxFileExists)
{
  std::string configPath = boost::filesystem::unique_path().native();

  std::ofstream log4cxxConfFile;
  log4cxxConfFile.open(configPath);
  log4cxxConfFile.close();

  std::string config = CONFIG_LOG4CXX;
  boost::replace_all(config, LOG4CXX_PLACEHOLDER, configPath);

  BOOST_CHECK_EQUAL(processConfigurationString(config), true);

  ConfParameter& conf = nlsr.getConfParameter();
  BOOST_CHECK_EQUAL(conf.getLog4CxxConfPath(), configPath);
  BOOST_CHECK_EQUAL(conf.isLog4CxxConfAvailable(), true);

  boost::filesystem::remove(boost::filesystem::path(configPath));
}

BOOST_AUTO_TEST_CASE(Log4cxxFileDoesNotExist)
{
  std::string configPath = boost::filesystem::unique_path().native();

  std::string config = CONFIG_LOG4CXX;
  boost::replace_all(config, LOG4CXX_PLACEHOLDER, configPath);

  BOOST_CHECK_EQUAL(processConfigurationString(config), false);
}

BOOST_AUTO_TEST_CASE(Log4cxxNoValue)
{
  std::string config = CONFIG_LOG4CXX;
  boost::replace_all(config, LOG4CXX_PLACEHOLDER, "");

  BOOST_CHECK_EQUAL(processConfigurationString(config), false);
}

BOOST_AUTO_TEST_CASE(Log4cxxTestCase)
{
  {
    std::ofstream of(getLogConfigFileName().c_str());
    of << "log4j.rootLogger=TRACE, FILE\n"
       << "log4j.appender.FILE=org.apache.log4j.FileAppender\n"
       << "log4j.appender.FILE.layout=org.apache.log4j.PatternLayout\n"
       << "log4j.appender.FILE.File=" << getLogFileName() << "\n"
       << "log4j.appender.FILE.ImmediateFlush=true\n"
       << "log4j.appender.FILE.layout.ConversionPattern=%d{HH:mm:ss} %p %c{1} - %m%n\n";
  }

  INIT_LOG4CXX(getLogConfigFileName());

  INIT_LOGGER("DefaultConfig");

  _LOG_TRACE("trace-message-JHGFDSR^1");
  _LOG_DEBUG("debug-message-IGg2474fdksd-fo-" << 15 << 16 << 17);
  _LOG_INFO("info-message-Jjxjshj13");
  _LOG_WARN("warning-message-XXXhdhd11" << 1 <<"x");
  _LOG_ERROR("error-message-!#$&^%$#@");
  _LOG_FATAL("fatal-message-JJSjaamcng");

  const std::string EXPECTED[] =
    {
      "", "TRACE", "DefaultConfig", "-", "trace-message-JHGFDSR^1",
      "", "DEBUG", "DefaultConfig", "-", "debug-message-IGg2474fdksd-fo-151617",
      "", "INFO",  "DefaultConfig", "-", "info-message-Jjxjshj13",
      "", "WARN",  "DefaultConfig", "-", "warning-message-XXXhdhd111x",
      "", "ERROR", "DefaultConfig", "-", "error-message-!#$&^%$#@",
      "", "FATAL", "DefaultConfig", "-", "fatal-message-JJSjaamcng",
      "",
    };

  verifyOutputLog4cxx(EXPECTED, sizeof(EXPECTED) / sizeof(std::string));
}

BOOST_AUTO_TEST_CASE(MalformedUri)
{
  const std::string MALFORMED_URI =
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
    "    face-uri  udp4:malformed-uri\n"
    "    link-cost 20\n"
    "  }\n"
    "}\n\n";

  BOOST_CHECK_EQUAL(processConfigurationString(MALFORMED_URI), false);
}

BOOST_AUTO_TEST_CASE(Hyperbolic)
{
  processConfigurationString(CONFIG_HYPERBOLIC);

  ConfParameter& conf = nlsr.getConfParameter();
  BOOST_CHECK_EQUAL(conf.getHyperbolicState(), 1);
  BOOST_CHECK_EQUAL(conf.getCorR(), 123.456);
  BOOST_CHECK_EQUAL(conf.getCorTheta(), 1.45);
}

BOOST_AUTO_TEST_CASE(DefaultValuesGeneral)
{
  std::string config = SECTION_GENERAL;

  commentOut("lsa-refresh-time", config);
  commentOut("lsa-interest-lifetime", config);
  commentOut("router-dead-interval", config);
  commentOut("log-level", config);

  BOOST_CHECK_EQUAL(processConfigurationString(config), true);

  ConfParameter& conf = nlsr.getConfParameter();

  BOOST_CHECK_EQUAL(conf.getLsaRefreshTime(), static_cast<uint32_t>(LSA_REFRESH_TIME_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getLsaInterestLifetime(),
                    static_cast<ndn::time::seconds>(LSA_INTEREST_LIFETIME_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getRouterDeadInterval(), (2*conf.getLsaRefreshTime()));
  BOOST_CHECK_EQUAL(conf.getLogLevel(), "INFO");
}

BOOST_AUTO_TEST_CASE(DefaultValuesNeighbors)
{
  std::string config = SECTION_NEIGHBORS;

  commentOut("hello-retries", config);
  commentOut("hello-timeout", config);
  commentOut("hello-interval", config);
  commentOut("first-hello-interval", config);
  commentOut("adj-lsa-build-interval", config);

  BOOST_CHECK_EQUAL(processConfigurationString(config), true);

  ConfParameter& conf = nlsr.getConfParameter();

  BOOST_CHECK_EQUAL(conf.getInterestRetryNumber(), static_cast<uint32_t>(HELLO_RETRIES_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getInterestResendTime(), static_cast<uint32_t>(HELLO_TIMEOUT_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getInfoInterestInterval(), static_cast<uint32_t>(HELLO_INTERVAL_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getFirstHelloInterval(),
                    static_cast<uint32_t>(FIRST_HELLO_INTERVAL_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getAdjLsaBuildInterval(),
                    static_cast<uint32_t>(ADJ_LSA_BUILD_INTERVAL_DEFAULT));
}

BOOST_AUTO_TEST_CASE(DefaultValuesFib)
{
  std::string config = SECTION_FIB;

  commentOut("max-faces-per-prefix", config);
  commentOut("routing-calc-interval", config);

  BOOST_CHECK_EQUAL(processConfigurationString(config), true);

  ConfParameter& conf = nlsr.getConfParameter();

  BOOST_CHECK_EQUAL(conf.getMaxFacesPerPrefix(),
                    static_cast<uint32_t>(MAX_FACES_PER_PREFIX_DEFAULT));
  BOOST_CHECK_EQUAL(conf.getRoutingCalcInterval(),
                    static_cast<uint32_t>(ROUTING_CALC_INTERVAL_DEFAULT));
}

BOOST_AUTO_TEST_CASE(DefaultValuesHyperbolic)
{
  std::string config = SECTION_HYPERBOLIC_ON;

  commentOut("state", config);

  BOOST_CHECK_EQUAL(processConfigurationString(config), true);

  ConfParameter& conf = nlsr.getConfParameter();

  BOOST_CHECK_EQUAL(conf.getHyperbolicState(), static_cast<int32_t>(HYPERBOLIC_STATE_DEFAULT));
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
  ndn::Name identity("/TestNLSR/identity");
  identity.appendVersion();

  ndn::KeyChain keyChain;
  keyChain.createIdentity(identity);
  ndn::Name certName = keyChain.getDefaultCertificateNameForIdentity(identity);
  shared_ptr<ndn::IdentityCertificate> certificate = keyChain.getCertificate(certName);

  const boost::filesystem::path CERT_PATH =
      (boost::filesystem::current_path() / std::string("cert-to-publish.cert"));
  ndn::io::save(*certificate, CERT_PATH.string());

  const std::string SECTION_SECURITY =
  "security\n"
  "{\n"
  "  validator\n"
  "  {\n"
  "    trust-anchor\n"
  "    {\n"
  "      type any\n"
  "    }\n"
  "  }\n"
  "  prefix-update-validator\n"
  "  {\n"
  "    trust-anchor\n"
  "    {\n"
  "      type any\n"
  "    }\n"
  "  }\n"
  "  cert-to-publish \"cert-to-publish.cert\"\n"
  "}\n\n";

  BOOST_CHECK(processConfigurationString(SECTION_SECURITY));

  // Certificate should now be in the CertificateStore
  const security::CertificateStore& certStore = nlsr.getCertificateStore();
  const ndn::Name certKey = certificate->getName().getPrefix(-1);

  BOOST_CHECK(certStore.find(certKey) != nullptr);

  // Cleanup
  keyChain.deleteIdentity(identity);
  boost::filesystem::remove(CERT_PATH);
}

BOOST_AUTO_TEST_CASE(PrefixUpdateValidatorOptional) // Bug #2814
{
  const std::string SECTION_SECURITY =
  "security\n"
  "{\n"
  "  validator\n"
  "  {\n"
  "    trust-anchor\n"
  "    {\n"
  "      type any\n"
  "    }\n"
  "  }\n"
  "}\n\n";

  BOOST_CHECK(processConfigurationString(SECTION_SECURITY));
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
