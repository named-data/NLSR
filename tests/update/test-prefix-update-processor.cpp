/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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
#include <ndn-cxx/management/nfd-control-parameters.hpp>
#include <ndn-cxx/management/nfd-control-response.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>

#include <boost/filesystem.hpp>

using namespace ndn;

namespace nlsr {
namespace update {
namespace test {

class PrefixUpdateFixture : public nlsr::test::BaseFixture
{
public:
  PrefixUpdateFixture()
    : face(make_shared<ndn::util::DummyClientFace>(g_ioService))
    , siteIdentity(ndn::Name("/ndn/edu/test-site").appendVersion())
    , opIdentity(ndn::Name(siteIdentity).append(ndn::Name("%C1.Operator")).appendVersion())
    , nlsr(g_ioService, g_scheduler, *face)
    , keyPrefix(("/ndn/broadcast"))
    , updateProcessor(nlsr.getPrefixUpdateProcessor())
    , SITE_CERT_PATH(boost::filesystem::current_path() / std::string("site.cert"))
  {
    createSiteCert();
    BOOST_REQUIRE(siteCert != nullptr);

    createOperatorCert();
    BOOST_REQUIRE(opCert != nullptr);

    const std::string CONFIG =
      "rule\n"
      "{\n"
      "  id \"NLSR ControlCommand Rule\"\n"
      "  for interest\n"
      "  filter\n"
      "  {\n"
      "    type name\n"
      "    regex ^<localhost><nlsr><prefix-update>[<advertise><withdraw>]<>$\n"
      "  }\n"
      "  checker\n"
      "  {\n"
      "    type customized\n"
      "    sig-type rsa-sha256\n"
      "    key-locator\n"
      "    {\n"
      "      type name\n"
      "      regex ^([^<KEY><%C1.Operator>]*)<%C1.Operator>[^<KEY>]*<KEY><ksk-.*><ID-CERT>$\n"
      "    }\n"
      "  }\n"
      "}\n"
      "rule\n"
      "{\n"
      "  id \"NLSR Hierarchy Rule\"\n"
      "  for data\n"
      "  filter\n"
      "  {\n"
      "    type name\n"
      "    regex ^[^<KEY>]*<KEY><ksk-.*><ID-CERT><>$\n"
      "  }\n"
      "  checker\n"
      "  {\n"
      "    type hierarchical\n"
      "    sig-type rsa-sha256\n"
      "  }\n"
      "}\n"
      "trust-anchor\n"
      "{\n"
      " type file\n"
      " file-name \"site.cert\"\n"
      "}\n";

    const boost::filesystem::path CONFIG_PATH =
      (boost::filesystem::current_path() / std::string("unit-test.conf"));

    updateProcessor.getValidator().load(CONFIG, CONFIG_PATH.native());

    // Insert certs after the validator is loaded since ValidatorConfig::load() clears
    // the certificate cache
    nlsr.addCertificateToCache(opCert);

    // Set the network so the LSA prefix is constructed
    nlsr.getConfParameter().setNetwork("/ndn");

    // Initialize NLSR so a sync socket is created
    nlsr.initialize();

    // Listen on localhost prefix
    updateProcessor.startListening();

    face->processEvents(ndn::time::milliseconds(1));
    face->sentInterests.clear();
  }

  void
  createSiteCert()
  {
    // Site cert
    keyChain.createIdentity(siteIdentity);
    siteCertName = keyChain.getDefaultCertificateNameForIdentity(siteIdentity);
    siteCert = keyChain.getCertificate(siteCertName);

    ndn::io::save(*siteCert, SITE_CERT_PATH.string());
  }

  void
  createOperatorCert()
  {
    // Operator cert
    ndn::Name keyName = keyChain.generateRsaKeyPairAsDefault(opIdentity, true);

    opCert = ndn::make_shared<ndn::IdentityCertificate>();
    ndn::shared_ptr<ndn::PublicKey> pubKey = keyChain.getPublicKey(keyName);
    opCertName = keyName.getPrefix(-1);
    opCertName.append("KEY").append(keyName.get(-1)).append("ID-CERT").appendVersion();
    opCert->setName(opCertName);
    opCert->setNotBefore(time::system_clock::now() - time::days(1));
    opCert->setNotAfter(time::system_clock::now() + time::days(1));
    opCert->setPublicKeyInfo(*pubKey);
    opCert->addSubjectDescription(CertificateSubjectDescription(ndn::oid::ATTRIBUTE_NAME,
                                                                keyName.toUri()));
    opCert->encode();

    keyChain.signByIdentity(*opCert, siteIdentity);

    keyChain.addCertificateAsIdentityDefault(*opCert);
  }

  bool
  wasRoutingUpdatePublished()
  {
    const ndn::Name& lsaPrefix = nlsr.getConfParameter().getLsaPrefix();

    const auto& it = std::find_if(face->sentData.begin(), face->sentData.end(),
      [lsaPrefix] (const ndn::Data& data) {
        return lsaPrefix.isPrefixOf(data.getName());
      });

    return (it != face->sentData.end());
  }

  void
  checkResponseCode(const Name& commandPrefix, uint64_t expectedCode)
  {
    std::vector<Data>::iterator it = std::find_if(face->sentData.begin(),
                                                  face->sentData.end(),
                                                  [commandPrefix] (const Data& data) {
                                                    return commandPrefix.isPrefixOf(data.getName());
                                                  });
    BOOST_REQUIRE(it != face->sentData.end());

    ndn::nfd::ControlResponse response(it->getContent().blockFromValue());
    BOOST_CHECK_EQUAL(response.getCode(), expectedCode);
  }

  ~PrefixUpdateFixture()
  {
    keyChain.deleteIdentity(siteIdentity);
    keyChain.deleteIdentity(opIdentity);

    boost::filesystem::remove(SITE_CERT_PATH);
  }

public:
  shared_ptr<ndn::util::DummyClientFace> face;
  ndn::KeyChain keyChain;

  ndn::Name siteIdentity;
  ndn::Name siteCertName;
  shared_ptr<IdentityCertificate> siteCert;

  ndn::Name opIdentity;
  ndn::Name opCertName;
  shared_ptr<IdentityCertificate> opCert;

  Nlsr nlsr;
  ndn::Name keyPrefix;
  PrefixUpdateProcessor& updateProcessor;

  const boost::filesystem::path SITE_CERT_PATH;
};

BOOST_FIXTURE_TEST_SUITE(TestPrefixUpdateProcessor, PrefixUpdateFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  updateProcessor.enable();

  // Advertise
  ndn::nfd::ControlParameters parameters;
  parameters.setName("/prefix/to/advertise/");

  ndn::Name advertiseCommand("/localhost/nlsr/prefix-update/advertise");
  advertiseCommand.append(parameters.wireEncode());

  shared_ptr<Interest> advertiseInterest = make_shared<Interest>(advertiseCommand);
  keyChain.signByIdentity(*advertiseInterest, opIdentity);

  face->receive(*advertiseInterest);
  face->processEvents(ndn::time::milliseconds(1));

  NamePrefixList& namePrefixList = nlsr.getNamePrefixList();

  BOOST_REQUIRE_EQUAL(namePrefixList.getSize(), 1);
  BOOST_CHECK_EQUAL(namePrefixList.getNameList().front(), parameters.getName());

  BOOST_CHECK(wasRoutingUpdatePublished());
  face->sentData.clear();

  // Withdraw
  ndn::Name withdrawCommand("/localhost/nlsr/prefix-update/withdraw");
  withdrawCommand.append(parameters.wireEncode());

  shared_ptr<Interest> withdrawInterest = make_shared<Interest>(withdrawCommand);
  keyChain.signByIdentity(*withdrawInterest, opIdentity);

  face->receive(*withdrawInterest);
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_CHECK_EQUAL(namePrefixList.getSize(), 0);

  BOOST_CHECK(wasRoutingUpdatePublished());
}

BOOST_AUTO_TEST_CASE(DisabledAndEnabled)
{
  ndn::nfd::ControlParameters parameters;
  parameters.setName("/prefix/to/advertise/");

  ndn::Name advertiseCommand("/localhost/nlsr/prefix-update/advertise");
  advertiseCommand.append(parameters.wireEncode());

  shared_ptr<Interest> advertiseInterest = make_shared<Interest>(advertiseCommand);
  keyChain.signByIdentity(*advertiseInterest, opIdentity);

  // Command should be rejected
  face->receive(*advertiseInterest);
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_REQUIRE(!face->sentData.empty());

  const ndn::MetaInfo& metaInfo = face->sentData.front().getMetaInfo();
  BOOST_CHECK_EQUAL(metaInfo.getType(), ndn::tlv::ContentType_Nack);

  face->sentData.clear();

  // Enable PrefixUpdateProcessor so commands will be processed
  updateProcessor.enable();

  // Command should be accepted
  face->receive(*advertiseInterest);
  face->processEvents(ndn::time::milliseconds(1));

  checkResponseCode(advertiseCommand, 200);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace update
} // namespace nlsr
