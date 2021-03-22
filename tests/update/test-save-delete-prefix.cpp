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

#include "update/prefix-update-processor.hpp"
#include "nlsr.hpp"

#include "tests/control-commands.hpp"
#include "tests/test-common.hpp"
#include "conf-parameter.hpp"

#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/security/command-interest-signer.hpp>
#include <ndn-cxx/security/pib/identity.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>

#include <boost/filesystem.hpp>

namespace nlsr {
namespace test {

namespace pt = boost::property_tree;
using namespace pt;

class PrefixSaveDeleteFixture : public nlsr::test::UnitTestTimeFixture
{
public:
  PrefixSaveDeleteFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , siteIdentityName(ndn::Name("/edu/test-site"))
    , opIdentityName(ndn::Name("/edu/test-site").append(ndn::Name("%C1.Operator")))
    , testConfFile("/tmp/nlsr.conf.test")
    , conf(face, m_keyChain, testConfFile)
    , confProcessor(conf)
    , nlsr(face, m_keyChain, conf)
    , SITE_CERT_PATH(boost::filesystem::current_path() / std::string("site.cert"))
    , counter(0)
  {
    std::ifstream source("nlsr.conf", std::ios::binary);
    std::ofstream destination(testConfFile, std::ios::binary);
    destination << source.rdbuf();
    source.close();
    destination.close();

    conf.setConfFileNameDynamic(testConfFile);
    siteIdentity = addIdentity(siteIdentityName);
    saveCertificate(siteIdentity, SITE_CERT_PATH.string());

    // Operator cert
    opIdentity = addSubCertificate(opIdentityName, siteIdentity);
    // Loading the security section's validator part into the validator
    // See conf file processor for more details
    std::ifstream inputFile;
    inputFile.open(testConfFile);
    BOOST_REQUIRE(inputFile.is_open());
    pt::ptree pt;
    boost::property_tree::read_info(inputFile, pt);
    // Loads section and file name
    for (const auto& section : pt) {
      if (section.first == "security") {
        for (const auto& it : section.second) {
          if (it.first == "prefix-update-validator") {
            conf.getPrefixUpdateValidator().load(it.second, std::string("nlsr.conf"));
          }
        }
      }
    }

    inputFile.close();

    // Site cert
    siteIdentity = addIdentity(siteIdentityName);
    saveCertificate(siteIdentity, SITE_CERT_PATH.string());

    // Operator cert
    opIdentity = addSubCertificate(opIdentityName, siteIdentity);

    // Create certificate and load it to the validator
    conf.initializeKey();
    conf.loadCertToValidator(siteIdentity.getDefaultKey().getDefaultCertificate());
    conf.loadCertToValidator(opIdentity.getDefaultKey().getDefaultCertificate());

    // Set the network so the LSA prefix is constructed
    addIdentity(conf.getRouterPrefix());

    this->advanceClocks(ndn::time::milliseconds(10));
    face.sentInterests.clear();
  }

  uint32_t
  getResponseCode()
  {
    ndn::nfd::ControlResponse response;
    for (const auto& data : face.sentData) {
      response.wireDecode(data.getContent().blockFromValue());
    }
    return response.getCode();
  }

  bool
  checkPrefix(const std::string prefixName)
  {
    counter = 0;
    pt::ptree m_savePrefix;
    pt::info_parser::read_info(testConfFile, m_savePrefix);
    // counter helps to check if multiple prefix of same name exists on conf file
    for (const auto& section : m_savePrefix.get_child("advertising")) {
      std:: string b = section.second.get_value<std::string>();
      if (b == prefixName) {
        counter++;
      }
    }
    if (counter > 0) {
      return true;
    }
    return false;
  }

  ndn::Interest
  advertiseWithdraw(std::string prefixName, std::string type, bool P_FLAG)
  {
    ndn::nfd::ControlParameters parameters;
    parameters.setName(prefixName);
    if (P_FLAG)
    {
      parameters.setFlags(update::PREFIX_FLAG);
    }
    ndn::Name advertiseCommand("/localhost/nlsr/prefix-update/advertise");
    ndn::Name withdrawCommand("/localhost/nlsr/prefix-update/withdraw");
    ndn::security::CommandInterestSigner cis(m_keyChain);
    // type true for advertise, else withdraw
    if (type == "advertise") {
      advertiseCommand.append(parameters.wireEncode());
      return cis.makeCommandInterest(advertiseCommand, ndn::security::signingByIdentity(opIdentity));
    }
    else {
      withdrawCommand.append(parameters.wireEncode());
      return cis.makeCommandInterest(withdrawCommand, ndn::security::signingByIdentity(opIdentity));
    }
  }

public:
  ndn::util::DummyClientFace face;
  ndn::Name siteIdentityName, routerIdName;
  ndn::security::pib::Identity siteIdentity, routerId;

  ndn::Name opIdentityName;
  ndn::security::pib::Identity opIdentity;

  std::string testConfFile;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  Nlsr nlsr;
  const boost::filesystem::path SITE_CERT_PATH;
  ndn::Name sessionTime;
  int counter;
};

BOOST_FIXTURE_TEST_SUITE(TestAdvertiseWithdrawPrefix, PrefixSaveDeleteFixture)

BOOST_AUTO_TEST_CASE(Basic)
{

  face.receive(advertiseWithdraw("/prefix/to/save", "advertise", false));
  this->advanceClocks(ndn::time::milliseconds(10));
  BOOST_CHECK_EQUAL(checkPrefix("/prefix/to/save"), false);
  BOOST_CHECK_EQUAL(getResponseCode(), 200);
  face.sentData.clear();

  // trying to re-advertise
  face.receive(advertiseWithdraw("/prefix/to/save", "advertise", false));
  this->advanceClocks(ndn::time::milliseconds(10));
  BOOST_CHECK_EQUAL(getResponseCode(), 204);
  face.sentData.clear();

  // only withdraw
  face.receive(advertiseWithdraw("/prefix/to/save", "withdraw", false));
  this->advanceClocks(ndn::time::milliseconds(10));
  BOOST_CHECK_EQUAL(checkPrefix("/prefix/to/save"), false);
  BOOST_CHECK_EQUAL(getResponseCode(), 200);
  face.sentData.clear();

  // trying to re-advertise
  face.receive(advertiseWithdraw("/prefix/to/save", "withdraw", false));
  this->advanceClocks(ndn::time::milliseconds(10));
  BOOST_CHECK_EQUAL(getResponseCode(), 204);
  face.sentData.clear();
}

BOOST_AUTO_TEST_CASE(PrefixStillSavedAfterJustWithdrawn)
{
  // advertise and save
  face.receive(advertiseWithdraw("/prefix/to/save", "advertise", true));
  this->advanceClocks(ndn::time::milliseconds(10));
  face.sentData.clear();
  BOOST_CHECK_EQUAL(checkPrefix("/prefix/to/save"), true);

  // trying to advertise same name prefix
  face.receive(advertiseWithdraw("/prefix/to/save", "advertise", true));
  this->advanceClocks(ndn::time::milliseconds(10));
  BOOST_REQUIRE(counter == 1);
  BOOST_CHECK_EQUAL(getResponseCode(), 406);
  face.sentData.clear();

  // only withdraw
  face.receive(advertiseWithdraw("/prefix/to/save", "withdraw", false));
  this->advanceClocks(ndn::time::milliseconds(10));
  // after withdrawing only prefix should still be there
  BOOST_CHECK_EQUAL(checkPrefix("/prefix/to/save"), true);
  BOOST_CHECK_EQUAL(getResponseCode(), 200);

  // delete
  face.receive(advertiseWithdraw("/prefix/to/save", "withdraw", true));
  this->advanceClocks(ndn::time::milliseconds(10));
  // after withdrawn delete prefix should be deleted from the file
  BOOST_CHECK_EQUAL(getResponseCode(), 205);
  BOOST_CHECK_EQUAL(checkPrefix("/prefix/to/save"), false);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
