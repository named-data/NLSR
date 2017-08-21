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

#include "update/prefix-update-processor.hpp"

#include "../control-commands.hpp"
#include "../test-common.hpp"
#include "nlsr.hpp"

#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/pib/identity.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/security/command-interest-signer.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>
#include <ndn-cxx/lp/tags.hpp>

#include <boost/filesystem.hpp>

using namespace ndn;

namespace nlsr {
namespace update {
namespace test {

class PrefixUpdateFixture : public nlsr::test::UnitTestTimeFixture
{
public:
  PrefixUpdateFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , siteIdentityName(ndn::Name("/edu/test-site"))
    , opIdentityName(ndn::Name("/edu/test-site").append(ndn::Name("%C1.Operator")))
    , nlsr(m_ioService, m_scheduler, face, m_keyChain)
    , namePrefixList(nlsr.getNamePrefixList())
    , updatePrefixUpdateProcessor(nlsr.getPrefixUpdateProcessor())
    , SITE_CERT_PATH(boost::filesystem::current_path() / std::string("site.cert"))
  {
    // Site cert
    siteIdentity = addIdentity(siteIdentityName);
    saveCertificate(siteIdentity, SITE_CERT_PATH.string());

    // Operator cert
    opIdentity = addSubCertificate(opIdentityName, siteIdentity);

    const std::string CONFIG = R"CONF(
        rule
        {
          id "NLSR ControlCommand Rule"
          for interest
          filter
          {
            type name
            regex ^(<localhost><nlsr>)<prefix-update>[<advertise><withdraw>]<><><>$
          }
          checker
          {
            type customized
            sig-type rsa-sha256
            key-locator
            {
              type name
              regex ^<>*<KEY><>$
            }
          }
        }
        rule
        {
          id "NLSR Hierarchy Rule"
          for data
          filter
          {
            type name
            regex ^[^<KEY>]*<KEY><><><>$
          }
          checker
          {
            type hierarchical
            sig-type rsa-sha256
          }
        }
        trust-anchor
        {
         type file
         file-name "site.cert"
        }
    )CONF";

    const boost::filesystem::path CONFIG_PATH =
      (boost::filesystem::current_path() / std::string("unit-test.conf"));

    updatePrefixUpdateProcessor.getValidator().load(CONFIG, CONFIG_PATH.native());

    nlsr.loadCertToPublish(opIdentity.getDefaultKey().getDefaultCertificate());

    // Set the network so the LSA prefix is constructed
    nlsr.getConfParameter().setNetwork("/ndn");
    nlsr.getConfParameter().setSiteName("/edu/test-site");
    nlsr.getConfParameter().setRouterName("/%C1.Router/this-router");
    nlsr.getConfParameter().buildRouterPrefix();

    addIdentity(ndn::Name("/ndn/edu/test-site/%C1.Router/this-router"));

    // Initialize NLSR so a sync socket is created
    nlsr.initialize();

    // Saving clock::now before any advanceClocks so that it will
    // be the same value as what ChronoSync uses in setting the sessionName
    sessionTime.appendNumber(ndn::time::toUnixTimestamp(ndn::time::system_clock::now()).count());

    this->advanceClocks(ndn::time::milliseconds(10));

    face.sentInterests.clear();
  }

  void sendInterestForPublishedData()
  {
    // Need to send an interest now since ChronoSync
    // no longer does face->put(*data) in publishData.
    // Instead it does it in onInterest
    ndn::Name lsaInterestName = nlsr.getConfParameter().getLsaPrefix();
    lsaInterestName.append(nlsr.getConfParameter().getSiteName());
    lsaInterestName.append(nlsr.getConfParameter().getRouterName());
    lsaInterestName.append(std::to_string(Lsa::Type::NAME));

    // The part after LSA is Chronosync getSession
    lsaInterestName.append(sessionTime);
    lsaInterestName.appendNumber(nlsr.getLsdb().getSequencingManager().getNameLsaSeq());

    std::shared_ptr<Interest> lsaInterest = std::make_shared<Interest>(lsaInterestName);

    face.receive(*lsaInterest);
    this->advanceClocks(ndn::time::milliseconds(100));
  }

  bool
  wasRoutingUpdatePublished()
  {
    sendInterestForPublishedData();

    const ndn::Name& lsaPrefix = nlsr.getConfParameter().getLsaPrefix();

    const auto& it = std::find_if(face.sentData.begin(), face.sentData.end(),
      [lsaPrefix] (const ndn::Data& data) {
        return lsaPrefix.isPrefixOf(data.getName());
      }
    );

    return (it != face.sentData.end());
  }

  void
  checkResponseCode(const Name& commandPrefix, uint64_t expectedCode)
  {
    std::vector<Data>::iterator it = std::find_if(face.sentData.begin(),
                                                  face.sentData.end(),
                                                  [commandPrefix] (const Data& data) {
                                                    return commandPrefix.isPrefixOf(data.getName());
                                                  });
    BOOST_REQUIRE(it != face.sentData.end());

    ndn::nfd::ControlResponse response(it->getContent().blockFromValue());
    BOOST_CHECK_EQUAL(response.getCode(), expectedCode);
  }

public:
  ndn::util::DummyClientFace face;

  ndn::Name siteIdentityName;
  ndn::security::pib::Identity siteIdentity;

  ndn::Name opIdentityName;
  ndn::security::pib::Identity opIdentity;

  Nlsr nlsr;
  NamePrefixList& namePrefixList;
  PrefixUpdateProcessor& updatePrefixUpdateProcessor;

  const boost::filesystem::path SITE_CERT_PATH;
  ndn::Name sessionTime;
};

BOOST_FIXTURE_TEST_SUITE(TestPrefixUpdateProcessor, PrefixUpdateFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  uint64_t nameLsaSeqNoBeforeInterest = nlsr.getLsdb().getSequencingManager().getNameLsaSeq();

  ndn::nfd::ControlParameters parameters;
  parameters.setName("/prefix/to/advertise/");

  // Control Command format: /<prefix>/<management-module>/<command-verb>/<control-parameters>
  // /<timestamp>/<random-value>/<signed-interests-components>

  // Advertise
  ndn::Name advertiseCommand("/localhost/nlsr/prefix-update/advertise");

  // append /<control-parameters>
  advertiseCommand.append(parameters.wireEncode());

  ndn::security::CommandInterestSigner cis(m_keyChain);

  // CommandInterestSigner::makeCommandInterest() will append the last
  // three components: (<timestamp>/<random-value>/<signed-interests-components>)
  ndn::Interest advertiseInterest =
    cis.makeCommandInterest(advertiseCommand,
                            ndn::security::signingByIdentity(opIdentity));

  face.receive(advertiseInterest);

  this->advanceClocks(ndn::time::milliseconds(10));

  NamePrefixList& namePrefixList = nlsr.getNamePrefixList();

  BOOST_REQUIRE_EQUAL(namePrefixList.size(), 1);
  BOOST_CHECK_EQUAL(namePrefixList.getNames().front(), parameters.getName());

  BOOST_CHECK(wasRoutingUpdatePublished());
  BOOST_CHECK(nameLsaSeqNoBeforeInterest < nlsr.getLsdb().getSequencingManager().getNameLsaSeq());

  face.sentData.clear();
  nameLsaSeqNoBeforeInterest = nlsr.getLsdb().getSequencingManager().getNameLsaSeq();

  //Withdraw
  ndn::Name withdrawCommand("/localhost/nlsr/prefix-update/withdraw");
  withdrawCommand.append(parameters.wireEncode());

  ndn::Interest withdrawInterest
    = cis.makeCommandInterest(withdrawCommand,
                              ndn::security::signingByIdentity(opIdentity));

  face.receive(withdrawInterest);
  this->advanceClocks(ndn::time::milliseconds(10));

  BOOST_CHECK_EQUAL(namePrefixList.size(), 0);

  BOOST_CHECK(wasRoutingUpdatePublished());
  BOOST_CHECK(nameLsaSeqNoBeforeInterest < nlsr.getLsdb().getSequencingManager().getNameLsaSeq());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace update
} // namespace nlsr
