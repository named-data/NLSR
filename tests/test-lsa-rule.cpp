/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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

#include "nlsr.hpp"
#include "security/certificate-store.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <filesystem>

namespace nlsr::tests {

using namespace ndn;

class LsaRuleFixture : public IoKeyChainFixture
{
public:
  LsaRuleFixture()
    : face(m_io, m_keyChain, {true, true})
    , rootIdName("/ndn")
    , siteIdentityName("/ndn/edu/test-site")
    , opIdentityName("/ndn/edu/test-site/%C1.Operator/op1")
    , routerIdName("/ndn/edu/test-site/%C1.Router/router1")
    , confParam(face, m_keyChain)
    , confProcessor(confParam, SyncProtocol::PSYNC, HYPERBOLIC_STATE_OFF,
                    "/ndn/", "/edu/test-site", "/%C1.Router/router1")
    , lsdb(face, m_keyChain, confParam)
    , ROOT_CERT_PATH(std::filesystem::current_path() / "root.cert")
  {
    rootId = m_keyChain.createIdentity(rootIdName);
    siteIdentity = addSubCertificate(siteIdentityName, rootId);
    opIdentity = addSubCertificate(opIdentityName, siteIdentity);
    routerId = addSubCertificate(routerIdName, opIdentity);

    // Create certificate and load it to the validator
    // previously this was done by in nlsr ctor
    confParam.initializeKey();

    saveIdentityCert(rootId, ROOT_CERT_PATH.string());

    for (const auto& id : {rootId, siteIdentity, opIdentity, routerId}) {
      confParam.loadCertToValidator(id.getDefaultKey().getDefaultCertificate());
    }

    // Loading the security section's validator part into the validator
    // See conf file processor for more details
    boost::property_tree::ptree pt;
    boost::property_tree::read_info("nlsr.conf", pt);

    // Loads section and file name
    for (const auto& tn : pt) {
      if (tn.first == "security") {
        auto it = tn.second.begin();
        confParam.getValidator().load(it->second, std::string("nlsr.conf"));
        break;
      }
    }

    this->advanceClocks(10_ms);
    face.sentInterests.clear();
   }

public:
  ndn::DummyClientFace face;

  ndn::Name rootIdName, siteIdentityName, opIdentityName, routerIdName;
  ndn::security::pib::Identity rootId, siteIdentity, opIdentity, routerId;
  ConfParameter confParam;
  DummyConfFileProcessor confProcessor;
  Lsdb lsdb;

  const std::filesystem::path ROOT_CERT_PATH;
};

BOOST_FIXTURE_TEST_SUITE(TestLsaDataValidation, LsaRuleFixture)

BOOST_AUTO_TEST_CASE(ValidateCorrectLSA)
{
  ndn::Name lsaDataName = confParam.getLsaPrefix();
  lsaDataName.append(confParam.getSiteName());
  lsaDataName.append(confParam.getRouterName());

  // Append LSA type
  lsaDataName.append(boost::lexical_cast<std::string>(Lsa::Type::NAME));

  // This would be the sequence number of its own NameLsa
  lsaDataName.appendNumber(lsdb.m_sequencingManager.getNameLsaSeq());

  // Append version, segmentNo
  lsaDataName.appendNumber(1).appendNumber(1);

  ndn::Data data(lsaDataName);
  data.setFreshnessPeriod(10_s);

  // Sign data with NLSR's key
  m_keyChain.sign(data, confParam.getSigningInfo());

  // Make NLSR validate data signed by its own key
  confParam.getValidator().validate(data,
                                    [] (const Data&) { BOOST_CHECK(true); },
                                    [] (const Data&, const ndn::security::ValidationError& e) {
                                      BOOST_ERROR(e);
                                    });
}

BOOST_AUTO_TEST_CASE(DoNotValidateIncorrectLSA)
{
  // getSubName removes the /localhop compnonent from /localhop/ndn/NLSR/LSA
  ndn::Name lsaDataName = confParam.getLsaPrefix().getSubName(1);
  lsaDataName.append(confParam.getSiteName());
  lsaDataName.append(confParam.getRouterName());

  // Append LSA type
  lsaDataName.append(boost::lexical_cast<std::string>(Lsa::Type::NAME));

  // This would be the sequence number of its own NameLsa
  lsaDataName.appendNumber(lsdb.m_sequencingManager.getNameLsaSeq());

  // Append version, segmentNo
  lsaDataName.appendNumber(1).appendNumber(1);

  ndn::Data data(lsaDataName);
  data.setFreshnessPeriod(ndn::time::seconds(10));

  // Make NLSR validate data signed by its own key
  confParam.getValidator().validate(data,
                                    [] (const Data&) { BOOST_CHECK(false); },
                                    [] (const Data&, const ndn::security::ValidationError&) {
                                      BOOST_CHECK(true);
                                    });
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
