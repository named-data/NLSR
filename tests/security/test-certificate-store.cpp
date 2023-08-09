/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023,  The University of Memphis,
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

#include "security/certificate-store.hpp"
#include "nlsr.hpp"
#include "lsdb.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/info_parser.hpp>

namespace nlsr {
namespace test {

using std::shared_ptr;

class CertificateStoreFixture : public IoKeyChainFixture
{
public:
  CertificateStoreFixture()
    : face(m_io, m_keyChain, {true, true})
    , conf(face, m_keyChain, "unit-test-nlsr.conf")
    , confProcessor(conf, SyncProtocol::PSYNC, HYPERBOLIC_STATE_OFF,
                    "/ndn/", "/site", "/%C1.Router/router1")
    , rootIdName(conf.getNetwork())
    , siteIdentityName(ndn::Name(conf.getNetwork()).append(conf.getSiteName()))
    , opIdentityName(ndn::Name(conf.getNetwork())
                     .append(ndn::Name(conf.getSiteName()))
                     .append(ndn::Name("%C1.Operator")))
    , routerIdName(conf.getRouterPrefix())
    , nlsr(face, m_keyChain, conf)
    , lsdb(nlsr.getLsdb())
    , certStore(face, conf, lsdb)
    , ROOT_CERT_PATH(boost::filesystem::current_path() / "root.cert")
  {
    rootId = m_keyChain.createIdentity(rootIdName);
    siteIdentity = addSubCertificate(siteIdentityName, rootId);
    opIdentity = addSubCertificate(opIdentityName, siteIdentity);
    routerId = addSubCertificate(routerIdName, opIdentity);

    auto instanceCert = conf.initializeKey();
    BOOST_REQUIRE(!!instanceCert);
    certStore.insert(*instanceCert);
    instanceCertName = instanceCert->getName();

    // Create certificate and load it to the validator
    // previously this was done by in nlsr ctor
    for (const auto& id : {rootId, siteIdentity, opIdentity, routerId}) {
      auto cert = id.getDefaultKey().getDefaultCertificate();
      conf.loadCertToValidator(cert);
      certStore.insert(cert);
    }

    boost::property_tree::ptree pt;
    boost::property_tree::read_info("nlsr.conf", pt);

    // Load security section and file name
    for (const auto& tn : pt) {
      if (tn.first == "security") {
        auto it = tn.second.begin();
        conf.getValidator().load(it->second, std::string("nlsr.conf"));
        break;
      }
    }

    advanceClocks(20_ms);
  }

public:
  void
  checkForInterest(ndn::Name& interstName)
  {
    std::vector<ndn::Interest>& interests = face.sentInterests;
    BOOST_REQUIRE(interests.size() > 0);

    bool didFindInterest = false;
    for (const auto& interest : interests) {
      didFindInterest = didFindInterest || interest.getName() == interstName;
    }
    BOOST_CHECK(didFindInterest);
  }

  ndn::DummyClientFace face;

  ConfParameter conf;
  DummyConfFileProcessor confProcessor;

  ndn::Name rootIdName, siteIdentityName, opIdentityName, routerIdName;
  ndn::security::pib::Identity rootId, siteIdentity, opIdentity, routerId;
  ndn::Name instanceCertName;

  Nlsr nlsr;
  Lsdb& lsdb;
  ndn::security::Certificate certificate;
  ndn::Name certificateKey;
  security::CertificateStore certStore;
  const boost::filesystem::path ROOT_CERT_PATH;
};

BOOST_FIXTURE_TEST_SUITE(TestCertificateStore, CertificateStoreFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  ndn::Name identityName("/TestNLSR/identity");
  identityName.appendVersion();

  auto identity = m_keyChain.createIdentity(identityName);
  auto certificate = identity.getDefaultKey().getDefaultCertificate();

  ndn::Name certKey = certificate.getKeyName();

  BOOST_CHECK(certStore.find(certKey) == nullptr);
  BOOST_CHECK(certStore.find(certificate.getName()) == nullptr);

  // Certificate should be retrievable from the CertificateStore
  certStore.insert(certificate);
  conf.loadCertToValidator(certificate);

  BOOST_CHECK(certStore.find(certKey) != nullptr);
  BOOST_CHECK(certStore.find(certificate.getName()) != nullptr);

  lsdb.expressInterest(certKey, 0, 0);

  advanceClocks(10_ms);
  checkForInterest(certKey);
}

BOOST_AUTO_TEST_CASE(RetrieveCert)
{
  ndn::DummyClientFace consumer(m_io);
  consumer.linkTo(face);

  auto checkRetrieve = [&] (const ndn::Name& interestName, bool canBePrefix, const ndn::Name& dataName) {
    ndn::Interest interest(interestName);
    interest.setCanBePrefix(canBePrefix);
    BOOST_TEST_CONTEXT(interest) {
      bool hasData = false;
      consumer.expressInterest(interest,
        [&] (const auto&, const auto& data) {
          BOOST_CHECK(!hasData);
          hasData = true;
          BOOST_CHECK_EQUAL(data.getName(), dataName);
        },
        [&] (const auto&, const auto&) { BOOST_ERROR("unexpected Nack"); },
        [&] (const auto&) { BOOST_ERROR("unexpected timeout"); }
      );
      advanceClocks(10_ms, 2);
      BOOST_CHECK(hasData);
    }
  };

  for (const auto& id : {siteIdentity, opIdentity, routerId}) {
    auto key = id.getDefaultKey();
    auto cert = key.getDefaultCertificate();
    checkRetrieve(key.getName(), true, cert.getName());
    checkRetrieve(cert.getName(), false, cert.getName());
  }

  checkRetrieve(ndn::security::extractKeyNameFromCertName(instanceCertName), true, instanceCertName);
  checkRetrieve(instanceCertName, false, instanceCertName);
}

BOOST_AUTO_TEST_CASE(TestKeyPrefixRegistration)
{
  // check if nlsrKeyPrefix is registered
  ndn::Name nlsrKeyPrefix = conf.getRouterPrefix();
  nlsrKeyPrefix.append("nlsr");
  nlsrKeyPrefix.append(ndn::security::Certificate::KEY_COMPONENT);
  checkPrefixRegistered(face, nlsrKeyPrefix);

  // check if routerPrefix is registered
  ndn::Name routerKeyPrefix = conf.getRouterPrefix();
  routerKeyPrefix.append(ndn::security::Certificate::KEY_COMPONENT);
  checkPrefixRegistered(face, routerKeyPrefix);

  // check if operatorKeyPrefix is registered
  ndn::Name operatorKeyPrefix = conf.getNetwork();
  operatorKeyPrefix.append(conf.getSiteName());
  operatorKeyPrefix.append(std::string("%C1.Operator"));
  checkPrefixRegistered(face, operatorKeyPrefix);
}

BOOST_AUTO_TEST_CASE(SegmentValidatedSignal)
{
  ndn::Name lsaInterestName("/localhop");
  lsaInterestName.append(conf.getLsaPrefix().getSubName(1));
  lsaInterestName.append(conf.getSiteName());
  lsaInterestName.append(conf.getRouterName());
  lsaInterestName.append(boost::lexical_cast<std::string>(Lsa::Type::NAME));
  lsaInterestName.appendNumber(nlsr.m_lsdb.m_sequencingManager.getNameLsaSeq() + 1);

  lsdb.expressInterest(lsaInterestName, 0, 0);
  advanceClocks(10_ms);

  checkForInterest(lsaInterestName);

  ndn::Name lsaDataName(lsaInterestName);
  lsaDataName.appendVersion();
  lsaDataName.appendSegment(0);

  ndn::Data data(lsaDataName);
  data.setFreshnessPeriod(ndn::time::seconds(10));
  NameLsa nameLsa;
  data.setContent(nameLsa.wireEncode());
  data.setFinalBlock(lsaDataName[-1]);

  // Sign data with this NLSR's key (in real it would be different NLSR)
  m_keyChain.sign(data, conf.m_signingInfo);
  face.put(data);

  this->advanceClocks(ndn::time::milliseconds(1));

  // Make NLSR validate data signed by its own key
  conf.getValidator().validate(data,
                                 [] (const ndn::Data&) { BOOST_CHECK(true); },
                                 [] (const ndn::Data&, const ndn::security::ValidationError& e) {
                                   BOOST_ERROR(e);
                                 });

  lsdb.emitSegmentValidatedSignal(data);
  auto certName = data.getSignatureInfo().getKeyLocator().getName();
  auto keyName = ndn::security::extractKeyNameFromCertName(certName);
  BOOST_CHECK(certStore.find(keyName) != nullptr);

  // testing a callback after segment validation signal from lsdb
  ndn::signal::ScopedConnection connection = lsdb.afterSegmentValidatedSignal.connect(
  [&] (const ndn::Data& lsaSegment) {
    BOOST_CHECK_EQUAL(lsaSegment.getName(), data.getName());
  });
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
