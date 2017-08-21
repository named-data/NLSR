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

class LsaRuleFixture : public nlsr::test::UnitTestTimeFixture
{
public:
  LsaRuleFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , rootIdName("/ndn")
    , siteIdentityName("/ndn/edu/test-site")
    , opIdentityName("/ndn/edu/test-site/%C1.Operator/op1")
    , routerIdName("/ndn/edu/test-site/%C1.Router/router1")
    , nlsr(m_ioService, m_scheduler, face, m_keyChain)
    , ROOT_CERT_PATH(boost::filesystem::current_path() / std::string("root.cert"))
  {
    rootId = addIdentity(rootIdName);
    siteIdentity = addSubCertificate(siteIdentityName, rootId);
    opIdentity = addSubCertificate(opIdentityName, siteIdentity);
    routerId = addSubCertificate(routerIdName, opIdentity);

    saveCertificate(rootId, ROOT_CERT_PATH.string());

    auto load = [this] (const ndn::security::Identity& id) {
      nlsr.loadCertToPublish(id.getDefaultKey().getDefaultCertificate());
    };
    load(rootId);
    load(siteIdentity);
    load(opIdentity);
    load(routerId);

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

    this->advanceClocks(ndn::time::milliseconds(10));

    face.sentInterests.clear();
   }

public:
  ndn::util::DummyClientFace face;

  ndn::Name rootIdName, siteIdentityName, opIdentityName, routerIdName;
  ndn::security::pib::Identity rootId, siteIdentity, opIdentity, routerId;

  Nlsr nlsr;

  const boost::filesystem::path ROOT_CERT_PATH;

  //std::function<void(const ndn::Interest& interest)> processInterest;
};

BOOST_FIXTURE_TEST_SUITE(TestLsaDataValidation, LsaRuleFixture)

BOOST_AUTO_TEST_CASE(ValidateCorrectLSA)
{
  ndn::Name lsaDataName = nlsr.getConfParameter().getLsaPrefix();
  lsaDataName.append(nlsr.getConfParameter().getSiteName());
  lsaDataName.append(nlsr.getConfParameter().getRouterName());

  // Append LSA type
  lsaDataName.append(std::to_string(Lsa::Type::NAME));

  // This would be the sequence number of its own NameLsa
  lsaDataName.appendNumber(nlsr.getLsdb().getSequencingManager().getNameLsaSeq());

  // Append version, segmentNo
  lsaDataName.appendNumber(1).appendNumber(1);

  ndn::Data data(lsaDataName);
  data.setFreshnessPeriod(ndn::time::seconds(10));

  // Sign data with NLSR's key
  nlsr.getKeyChain().sign(data, nlsr.getSigningInfo());

  // Make NLSR validate data signed by its own key
  nlsr.getValidator().validate(data,
                               [] (const Data&) { BOOST_CHECK(true); },
                               [] (const Data&, const ndn::security::v2::ValidationError&) {
                                 BOOST_CHECK(false);
                               });
}

BOOST_AUTO_TEST_CASE(DoNotValidateIncorrectLSA)
{
  // getSubName removes the /localhop compnonent from /localhop/ndn/NLSR/LSA
  ndn::Name lsaDataName = nlsr.getConfParameter().getLsaPrefix().getSubName(1);
  lsaDataName.append(nlsr.getConfParameter().getSiteName());
  lsaDataName.append(nlsr.getConfParameter().getRouterName());

  // Append LSA type
  lsaDataName.append(std::to_string(Lsa::Type::NAME));

  // This would be the sequence number of its own NameLsa
  lsaDataName.appendNumber(nlsr.getLsdb().getSequencingManager().getNameLsaSeq());

  // Append version, segmentNo
  lsaDataName.appendNumber(1).appendNumber(1);

  ndn::Data data(lsaDataName);
  data.setFreshnessPeriod(ndn::time::seconds(10));

  // Make NLSR validate data signed by its own key
  nlsr.getValidator().validate(data,
                               [] (const Data&) { BOOST_CHECK(false); },
                               [] (const Data&, const ndn::security::v2::ValidationError&) {
                                 BOOST_CHECK(true);
                               });
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
