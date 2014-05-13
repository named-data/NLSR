/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */
#include <fstream>
#include "conf-file-processor.hpp"
#include "nlsr.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestConfFileProcessor)

BOOST_AUTO_TEST_CASE(ConfFileProcessorSample)
{
  Nlsr nlsr1;

  const std::string CONFIG =
     "general\n"
    "{\n"
    "  network /ndn/\n"      
    "  site /memphis.edu/\n"
    "  router /cs/pollux/\n"
    "  lsa-refresh-time 1800\n"
    "  log-level  INFO\n"
    "}\n\n"
    "neighbors\n"
    "{\n"
    "  hello-retries 3\n"
    "  hello-time-out 1\n"
    "  hello-interval  60\n\n"
    "  neighbor\n"
    "  {\n"
    "    name /ndn/memphis.edu/cs/castor\n"
    "    face-id  15\n"
    "    link-cost 20\n"
    "  }\n\n"
    "  neighbor\n"
    "  {\n"
    "    name /ndn/memphis.edu/cs/mira\n"
    "    face-id  17\n"
    "    link-cost 30\n"
    "  }\n"
    "}\n\n"
    "hyperbolic\n"
    "{\n"
    "state off\n"
    "radius   123.456\n"
    "angle    1.45\n"
    "}\n\n"
    "fib\n"
    "{\n"
    "   max-faces-per-prefix 3\n"
    "}\n\n"
    "advertising\n"
    "{\n"
    "prefix /ndn/edu/memphis/cs/netlab\n"
    "prefix /ndn/edu/memphis/sports/basketball\n"
    "}\n";

  std::ofstream config;
  config.open("unit-test-nlsr.conf");
  config << CONFIG;
  config.close();

  const std::string CONFIG_FILE = "unit-test-nlsr.conf";

  ConfFileProcessor cfp1(nlsr1, CONFIG_FILE);

  cfp1.processConfFile();

  BOOST_CHECK(nlsr1.getAdjacencyList().isNeighbor("/ndn/memphis.edu/cs/mira"));
  BOOST_CHECK_EQUAL(
    nlsr1.getAdjacencyList().getAdjacent("/ndn/memphis.edu/cs/mira").getName(),
    "/ndn/memphis.edu/cs/mira");
  BOOST_CHECK_EQUAL(
    nlsr1.getAdjacencyList().getAdjacent("/ndn/memphis.edu/cs/mira").getLinkCost(),
    30);

  BOOST_CHECK_EQUAL(nlsr1.getNamePrefixList().getSize(), 2);

  remove("unit-test-nlsr.conf");
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
