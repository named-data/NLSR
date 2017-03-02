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

#include "test-common.hpp"
#include "nlsr.hpp"

#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/security/signing-info.hpp>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

using namespace ndn;

namespace nlsr {
namespace test {

class LsaRuleFixture : public nlsr::test::BaseFixture
{
public:
  LsaRuleFixture()
    : face(std::make_shared<ndn::util::DummyClientFace>(g_ioService))
    , rootId(ndn::Name("ndn"))
    , siteIdentity(ndn::Name("/ndn/edu/test-site"))
    , opIdentity(ndn::Name(siteIdentity).append(ndn::Name("%C1.Operator/op1")))
    , routerId(ndn::Name("/ndn/edu/test-site/%C1.Router/router1"))
    , nlsr(g_ioService, g_scheduler, *face, g_keyChain)
    , ROOT_CERT_PATH(boost::filesystem::current_path() / std::string("root.cert"))
  {
    try {
      keyChain.deleteIdentity(rootId);
      keyChain.deleteIdentity(siteIdentity);
      keyChain.deleteIdentity(opIdentity);
      keyChain.deleteIdentity(routerId);
    }
    catch (const std::exception& e) {
    }

    createCert(rootId, rootCertName, rootCert, rootId);
    BOOST_REQUIRE(rootCert != nullptr);

    createCert(siteIdentity, siteCertName, siteCert, rootId);
    BOOST_REQUIRE(siteCert != nullptr);

    createCert(opIdentity, opCertName, opCert, siteIdentity);
    BOOST_REQUIRE(opCert != nullptr);

    createCert(routerId, routerCertName, routerCert, opIdentity);
    BOOST_REQUIRE(routerCert != nullptr);

    // Loading the security section's validator part into the validator
    // See conf file processor for more details
    std::ifstream inputFile;
    inputFile.open(std::string("nlsr.conf"));

    BOOST_REQUIRE(inputFile.is_open());

    boost::property_tree::ptree pt;

    boost::property_tree::read_info(inputFile, pt);

    //Loads section and file name
    for (auto tn = pt.begin(); tn != pt.end(); ++tn) {
      if (tn->first == "security") {
        auto it = tn->second.begin();
        nlsr.loadValidator(it->second, std::string("nlsr.conf"));
        break;
      }
    }
    inputFile.close();

    // Set the network so the LSA prefix is constructed
    // Set all so that buildRouterPrefix is set
    nlsr.getConfParameter().setNetwork("/ndn");
    nlsr.getConfParameter().setSiteName("/edu/test-site");
    nlsr.getConfParameter().setRouterName("/%C1.Router/router1");

    // Initialize NLSR to initialize the keyChain
    nlsr.initialize();
  }

  void
  createCert(ndn::Name& identity, ndn::Name& certName, std::shared_ptr<IdentityCertificate>& cert, const ndn::Name& signer)
  {
    ndn::Name keyName = keyChain.generateRsaKeyPairAsDefault(identity, true);

    cert = std::make_shared<ndn::IdentityCertificate>();
    std::shared_ptr<ndn::PublicKey> pubKey = keyChain.getPublicKey(keyName);
    certName = keyName.getPrefix(-1);
    certName.append("KEY").append(keyName.get(-1)).append("ID-CERT").appendVersion();
    cert->setName(certName);
    cert->setNotBefore(time::system_clock::now() - time::days(1));
    cert->setNotAfter(time::system_clock::now() + time::days(1));
    cert->setPublicKeyInfo(*pubKey);
    cert->addSubjectDescription(CertificateSubjectDescription(ndn::oid::ATTRIBUTE_NAME,
                                                                keyName.toUri()));
    cert->encode();

    // root is self signed and root.cert is saved
    if (signer == identity) {
      keyChain.selfSign(*cert);

      keyChain.addCertificateAsIdentityDefault(*cert);

      nlsr.loadCertToPublish(cert);

      ndn::io::save(*cert, ROOT_CERT_PATH.string());
    }
    else {
      ndn::security::SigningInfo signingInfo;
      signingInfo.setSigningIdentity(signer);
      keyChain.sign(*cert, signingInfo);

      keyChain.addCertificateAsIdentityDefault(*cert);

      nlsr.loadCertToPublish(cert);
    }
  }

  ~LsaRuleFixture()
  {
    keyChain.deleteIdentity(rootId);
    keyChain.deleteIdentity(siteIdentity);
    keyChain.deleteIdentity(opIdentity);
    keyChain.deleteIdentity(routerId);

    boost::filesystem::remove(ROOT_CERT_PATH);
  }

public:
  std::shared_ptr<ndn::util::DummyClientFace> face;
  ndn::KeyChain keyChain;

  ndn::Name rootId, siteIdentity, opIdentity, routerId;
  ndn::Name rootCertName, siteCertName, opCertName, routerCertName;
  std::shared_ptr<IdentityCertificate> rootCert, siteCert, opCert, routerCert;

  Nlsr nlsr;

  const boost::filesystem::path ROOT_CERT_PATH;
};

BOOST_FIXTURE_TEST_SUITE(TestLsaDataValidation, LsaRuleFixture)

BOOST_AUTO_TEST_CASE(ValidateCorrectLSA)
{
  ndn::Name lsaInterestName = nlsr.getConfParameter().getLsaPrefix();
  lsaInterestName.append(nlsr.getConfParameter().getSiteName());
  lsaInterestName.append(nlsr.getConfParameter().getRouterName());

  // Append LSA type
  lsaInterestName.append(ndn::Name("name"));

  // This would be the sequence number of its own NameLsa
  lsaInterestName.appendNumber(nlsr.getSequencingManager().getNameLsaSeq());

  // Append version, segmentNo
  lsaInterestName.appendNumber(1).appendNumber(1);

  std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>();
  data->setName(lsaInterestName);
  data->setFreshnessPeriod(ndn::time::seconds(10));

  // Sign data with NLSR's key
  nlsr.getKeyChain().sign(*data, ndn::security::signingByCertificate(nlsr.getDefaultCertName()));

  // Make NLSR validate data signed by its own key
  nlsr.getValidator().validate(*data,
                               [] (const std::shared_ptr<const Data>&) { BOOST_CHECK(true); },
                               [] (const std::shared_ptr<const Data>&, const std::string&) {
                                 BOOST_CHECK(false);
                               });
}

BOOST_AUTO_TEST_CASE(DoNotValidateIncorrectLSA)
{
  // getSubName removes the /localhop compnonent from /localhop/ndn/NLSR/LSA
  ndn::Name lsaInterestName = nlsr.getConfParameter().getLsaPrefix().getSubName(1);
  lsaInterestName.append(nlsr.getConfParameter().getSiteName());
  lsaInterestName.append(nlsr.getConfParameter().getRouterName());

  // Append LSA type
  lsaInterestName.append(ndn::Name("name"));

  // This would be the sequence number of its own NameLsa
  lsaInterestName.appendNumber(nlsr.getSequencingManager().getNameLsaSeq());

  // Append version, segmentNo
  lsaInterestName.appendNumber(1).appendNumber(1);

  std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>();
  data->setName(lsaInterestName);
  data->setFreshnessPeriod(ndn::time::seconds(10));
  nlsr.getKeyChain().sign(*data, ndn::security::signingByCertificate(nlsr.getDefaultCertName()));

  // Make NLSR validate data signed by its own key
  nlsr.getValidator().validate(*data,
                               [] (const std::shared_ptr<const Data>&) { BOOST_CHECK(false); },
                               [] (const std::shared_ptr<const Data>&, const std::string&) {
                                 BOOST_CHECK(true);
                               });
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
