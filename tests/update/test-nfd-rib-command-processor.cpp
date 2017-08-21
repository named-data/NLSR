/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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

#include "update/nfd-rib-command-processor.hpp"

#include "../test-common.hpp"
#include "../control-commands.hpp"
#include "conf-parameter.hpp"
#include "adjacency-list.hpp"
#include "nlsr.hpp"

#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>
#include <ndn-cxx/security/key-chain.hpp>

namespace nlsr {
namespace update {
namespace test {

class NfdRibCommandProcessorFixture : public nlsr::test::UnitTestTimeFixture
{
public:
  NfdRibCommandProcessorFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , nlsr(m_ioService, m_scheduler, face, m_keyChain)
    , namePrefixes(nlsr.getNamePrefixList())
    , processor(nlsr.getNfdRibCommandProcessor())
  {
    // Set the network so the LSA prefix is constructed
    nlsr.getConfParameter().setNetwork("/ndn");
    nlsr.getConfParameter().setRouterName(ndn::Name("/This/router"));

    addIdentity(ndn::Name("/ndn/This/router"));

    // Initialize NLSR so a sync socket is created
    nlsr.initialize();

    // Saving clock::now before any advanceClocks so that it will
    // be the same value as what ChronoSync uses in setting the sessionName
    sessionTime.appendNumber(ndn::time::toUnixTimestamp(ndn::time::system_clock::now()).count());

    this->advanceClocks(ndn::time::milliseconds(10), 10);
    face.sentInterests.clear();

    nameLsaSeqNoBeforeInterest = nlsr.getLsdb().getSequencingManager().getNameLsaSeq();
  }

  std::shared_ptr<ndn::Interest>
  makeInterest(const ndn::Name& name, uint32_t nonce)
  {
    auto interest = std::make_shared<ndn::Interest>(name);
    if (nonce != 0) {
      interest->setNonce(nonce);
    }
    return interest;
  }

  void sendInterestForPublishedData() {
    // Need to send an interest now since ChronoSync
    // no longer does face->put(*data) in publishData.
    // Instead it does it in onInterest
    ndn::Name lsaInterestName("/localhop/ndn/NLSR/LSA/This/router");
    lsaInterestName.append(std::to_string(Lsa::Type::NAME));

    // The part after LSA is Chronosync getSession
    lsaInterestName.append(sessionTime);
    lsaInterestName.appendNumber(nlsr.getLsdb().getSequencingManager().getNameLsaSeq());
    shared_ptr<ndn::Interest> lsaInterest = make_shared<ndn::Interest>(lsaInterestName);

    face.receive(*lsaInterest);
    this->advanceClocks(ndn::time::milliseconds(10), 10);
  }

  bool
  wasRoutingUpdatePublished()
  {
    sendInterestForPublishedData();

    const ndn::Name& lsaPrefix = nlsr.getConfParameter().getLsaPrefix();

    const auto& it = std::find_if(face.sentData.begin(), face.sentData.end(),
      [&] (const ndn::Data& data) {
        return lsaPrefix.isPrefixOf(data.getName());
      });

    return (it != face.sentData.end());
  }

public:
  ndn::util::DummyClientFace face;

  Nlsr nlsr;
  NamePrefixList& namePrefixes;
  NfdRibCommandProcessor& processor;
  ndn::Name sessionTime;
  uint64_t nameLsaSeqNoBeforeInterest;
};

typedef boost::mpl::vector<NfdRibRegisterCommand, NfdRibUnregisterCommand> Commands;

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

  shared_ptr<ndn::Interest> command = makeInterest(name.append(parameters.setName(prefixName)
    .wireEncode()), 0);

  face.receive(*command);
  this->advanceClocks(ndn::time::milliseconds(10), 10);

  BOOST_CHECK_EQUAL(namePrefixes.getNames().size(), 1);
  std::list<ndn::Name> names = namePrefixes.getNames();
  auto itr = std::find(names.begin(), names.end(), prefixName);
  if (itr == namePrefixes.getNames().end()) {
    BOOST_FAIL("Prefix was not inserted!");
  }
  BOOST_CHECK_EQUAL((*itr), prefixName);
  BOOST_CHECK(wasRoutingUpdatePublished());
  BOOST_CHECK(nameLsaSeqNoBeforeInterest < nlsr.getLsdb().getSequencingManager().getNameLsaSeq());
}

BOOST_AUTO_TEST_CASE(onReceiveInterestUnregisterCommand)
{
  ndn::Name name("/localhost/nlsr/rib/unregister");
  ndn::Name prefixName("/test/prefixA");
  ndn::nfd::ControlParameters parameters;

  namePrefixes.insert(prefixName);

  shared_ptr<ndn::Interest> command = makeInterest(name.append(parameters.setName(prefixName)
    .wireEncode()), 0);

  face.receive(ndn::Interest(name));
  this->advanceClocks(ndn::time::milliseconds(10), 10);

  BOOST_CHECK_EQUAL(namePrefixes.getNames().size(), 0);
  BOOST_CHECK(wasRoutingUpdatePublished());
  BOOST_CHECK(nameLsaSeqNoBeforeInterest < nlsr.getLsdb().getSequencingManager().getNameLsaSeq());
}

BOOST_AUTO_TEST_CASE(onReceiveInterestInvalidPrefix)
{
  ndn::Name name("/localhost/invalid/rib/register");
  ndn::Name prefixName("/test/prefixA");
  ndn::nfd::ControlParameters parameters;

  shared_ptr<ndn::Interest> command = makeInterest(name.append(parameters.setName(prefixName)
    .wireEncode()), 0);

  face.receive(ndn::Interest(name));
  this->advanceClocks(ndn::time::milliseconds(10), 10);

  BOOST_CHECK_EQUAL(namePrefixes.getNames().size(), 0);

  // Cannot use routingUpdatePublish test now since in
  // initialize nlsr calls buildOwnNameLsa which publishes the routing update
  BOOST_CHECK(nameLsaSeqNoBeforeInterest == nlsr.getLsdb().getSequencingManager().getNameLsaSeq());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace update
} // namespace nlsr
