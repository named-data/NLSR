/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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
 */

#include "update/nfd-rib-command-processor.hpp"
#include "adjacency-list.hpp"
#include "conf-parameter.hpp"
#include "nlsr.hpp"

#include "../test-common.hpp"
#include "../control-commands.hpp"

#include <boost/lexical_cast.hpp>

namespace nlsr {
namespace test {

class NfdRibCommandProcessorFixture : public nlsr::test::UnitTestTimeFixture
{
public:
  NfdRibCommandProcessorFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , conf(face, m_keyChain)
    , confProcessor(conf)
    , nlsr(face, m_keyChain, conf)
    , namePrefixes(conf.getNamePrefixList())
    , processor(nlsr.m_nfdRibCommandProcessor)
  {
    addIdentity(conf.getRouterPrefix());

    this->advanceClocks(ndn::time::milliseconds(10), 10);
    face.sentInterests.clear();

    nameLsaSeqNoBeforeInterest = nlsr.m_lsdb.m_sequencingManager.getNameLsaSeq();
  }

  void
  sendCommand(ndn::Name prefix, const ndn::nfd::ControlParameters& parameters)
  {
    ndn::Interest interest(prefix.append(parameters.wireEncode()));
    interest.setCanBePrefix(false);
    face.receive(interest);
    this->advanceClocks(ndn::time::milliseconds(10), 10);
  }

  void sendInterestForPublishedData()
  {
    ndn::Name lsaInterestName = conf.getLsaPrefix();
    lsaInterestName.append(conf.getSiteName());
    lsaInterestName.append(conf.getRouterName());
    lsaInterestName.append(boost::lexical_cast<std::string>(Lsa::Type::NAME));
    lsaInterestName.appendNumber(nlsr.m_lsdb.m_sequencingManager.getNameLsaSeq());

    face.receive(ndn::Interest(lsaInterestName).setCanBePrefix(true));
    this->advanceClocks(ndn::time::milliseconds(10), 10);
  }

  bool
  wasRoutingUpdatePublished()
  {
    sendInterestForPublishedData();

    const ndn::Name& lsaPrefix = conf.getLsaPrefix();

    const auto& it = std::find_if(face.sentData.begin(), face.sentData.end(),
      [&] (const ndn::Data& data) {
        return lsaPrefix.isPrefixOf(data.getName());
      });

    return (it != face.sentData.end());
  }

public:
  ndn::util::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;

  Nlsr nlsr;
  NamePrefixList& namePrefixes;
  update::NfdRibCommandProcessor& processor;
  uint64_t nameLsaSeqNoBeforeInterest;
};

typedef boost::mpl::vector<update::NfdRibRegisterCommand,
                           update::NfdRibUnregisterCommand> Commands;

BOOST_FIXTURE_TEST_SUITE(TestNfdRibCommandProcessor, NfdRibCommandProcessorFixture)

BOOST_AUTO_TEST_CASE_TEMPLATE(ValidateParametersSuccess, NfdRibCommand, Commands)
{
  ndn::nfd::ControlParameters parameters;
  parameters.setName("/test/prefixA");

  BOOST_CHECK(processor.validateParameters<NfdRibCommand>(parameters));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ValidateParametersFailure, NfdRibCommand, Commands)
{
  ndn::nfd::ControlParameters parameters;
  parameters.setName("/test/prefixA").setCost(10);

  bool wasValidated = true;
  try {
    processor.validateParameters<NfdRibCommand>(parameters);
  }
  catch (...) {
    wasValidated = false;
  }
  BOOST_CHECK(!wasValidated);
}

BOOST_AUTO_TEST_CASE(onReceiveInterestRegisterCommand)
{
  ndn::Name name("/localhost/nlsr/rib/register");
  ndn::Name prefixName("/test/prefixA");
  ndn::nfd::ControlParameters parameters;
  parameters.setName(prefixName);

  sendCommand(name, parameters);

  BOOST_CHECK_EQUAL(namePrefixes.getNames().size(), 1);
  std::list<ndn::Name> names = namePrefixes.getNames();
  auto itr = std::find(names.begin(), names.end(), prefixName);
  if (itr == namePrefixes.getNames().end()) {
    BOOST_FAIL("Prefix was not inserted!");
  }
  BOOST_CHECK_EQUAL((*itr), prefixName);
  BOOST_CHECK(wasRoutingUpdatePublished());
  BOOST_CHECK(nameLsaSeqNoBeforeInterest < nlsr.m_lsdb.m_sequencingManager.getNameLsaSeq());
}

BOOST_AUTO_TEST_CASE(onReceiveInterestUnregisterCommand)
{
  ndn::Name name("/localhost/nlsr/rib/unregister");
  ndn::Name prefixName("/test/prefixA");
  ndn::nfd::ControlParameters parameters;
  parameters.setName(prefixName);

  namePrefixes.insert(prefixName);

  sendCommand(name, parameters);

  BOOST_CHECK_EQUAL(namePrefixes.getNames().size(), 0);
  BOOST_CHECK(wasRoutingUpdatePublished());
  BOOST_CHECK(nameLsaSeqNoBeforeInterest < nlsr.m_lsdb.m_sequencingManager.getNameLsaSeq());
}

BOOST_AUTO_TEST_CASE(onReceiveInterestInvalidPrefix)
{
  ndn::Name name("/localhost/invalid/rib/register");
  ndn::Name prefixName("/test/prefixA");
  ndn::nfd::ControlParameters parameters;
  parameters.setName(prefixName);

  sendCommand(name, parameters);

  BOOST_CHECK_EQUAL(namePrefixes.getNames().size(), 0);

  // Cannot use routingUpdatePublish test now since in
  // initialize nlsr calls buildOwnNameLsa which publishes the routing update
  BOOST_CHECK(nameLsaSeqNoBeforeInterest == nlsr.m_lsdb.m_sequencingManager.getNameLsaSeq());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
