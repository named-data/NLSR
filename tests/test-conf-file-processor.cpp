/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

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
    "network ndn\n"
    "site-name memphis.edu\n"
    "router-name cs/macbook\n\n"
    "ndnneighbor /ndn/memphis.edu/cs/maia 7\n"
    "link-cost /ndn/memphis.edu/cs/maia 30\n"
    "ndnneighbor /ndn/memphis.edu/cs/pollux 10\n"
    "link-cost /ndn/memphis.edu/cs/pollux 25\n\n"
    "ndnname /ndn/memphis.edu/cs/macbook/name1\n"
    "ndnname /ndn/memphis.edu/cs/macbook/name2\n\n\n"
    ;

  ofstream config;
  config.open("unit-test-nlsr.conf");
  config << CONFIG;
  config.close();

  const string CONFIG_FILE = "unit-test-nlsr.conf";

  ConfFileProcessor cfp1(nlsr1, CONFIG_FILE);

  cfp1.processConfFile();

  BOOST_CHECK(nlsr1.getAdjacencyList().isNeighbor("/ndn/memphis.edu/cs/maia"));
  BOOST_CHECK_EQUAL(
    nlsr1.getAdjacencyList().getAdjacent("/ndn/memphis.edu/cs/maia").getName(),
    "/ndn/memphis.edu/cs/maia");
  BOOST_CHECK_EQUAL(
    nlsr1.getAdjacencyList().getAdjacent("/ndn/memphis.edu/cs/maia").getLinkCost(),
    30);

  BOOST_CHECK_EQUAL(nlsr1.getNamePrefixList().getSize(), 2);

  remove("unit-test-nlsr.conf");
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
