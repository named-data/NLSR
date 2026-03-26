/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2026,  The University of Memphis,
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

#include "lsdb.hpp"
#include "lsa/lsa.hpp"
#include "name-prefix-list.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

#include <boost/mp11/list.hpp>

#include <fstream>
#include <string>

namespace nlsr::tests {

class DummyConfProcessor
{
public:
  DummyConfProcessor(ConfParameter& conf, SyncProtocol syncProtocol,
                     std::string& routerName, int lsaSeq)
  {
    conf.setNetwork("/ndn");
    conf.setSiteName("/site");
    conf.setRouterName(routerName);
    conf.buildRouterAndSyncUserPrefix();
    conf.setSyncProtocol(syncProtocol);
    conf.setHyperbolicState(HYPERBOLIC_STATE_OFF);
    conf.setSyncInterestLifetime(1000);

    std::string nlsrTestDir = "/tmp/nlsr-tests/";
    std::filesystem::create_directory(nlsrTestDir);

    std::string seqFileDir = nlsrTestDir + routerName;
    std::filesystem::create_directory(seqFileDir);
    conf.setStateFileDir(seqFileDir);
    std::string seqFile = seqFileDir + "/nlsrSeqNo.txt";
    std::ofstream seqofs(seqFile);
    seqofs << "NameLsaSeq " << lsaSeq;
    seqofs << "AdjLsaSeq " << lsaSeq;
    seqofs << "CorLsaSeq " << 0;
    seqofs.close();

    std::string nlsrFile = nlsrTestDir + "nlsr.conf";
    std::string validatorStr = "security { validator { trust-anchor { type any } } }";
    std::ofstream ofs(nlsrFile);
    ofs << validatorStr;
    ofs.close();

    conf.getValidator().load("trust-anchor { type any }", nlsrFile);
  }
};

template<SyncProtocol P>
class LsdbFixture : public IoKeyChainFixture
{
public:
  LsdbFixture()
  {
    routerName[0] = "routerA";
    routerName[1] = "routerB";
    this->advanceClocks(10_ms);
  }

  void
  startRouter(int index, int lsaSeq)
  {
    if (faces[index] == nullptr) {
      faces[index] = std::make_shared<ndn::DummyClientFace>(m_io, m_keyChain, ndn::DummyClientFace::Options{true, true});
    }

    if (conf[index] == nullptr) {
      conf[index] = std::make_shared<ConfParameter>(*faces[index], m_keyChain);
    }

    if (confProcessor[index] == nullptr) {
      confProcessor[index] = std::make_shared<DummyConfProcessor>(*conf[index], P, routerName[index], lsaSeq);
    }

    if (lsdb[index] == nullptr) {
      lsdb[index] = std::make_shared<Lsdb>(*faces[index], m_keyChain, *conf[index]);
    }
  }

  void
  killRouter(int index)
  {
    faces[index] = nullptr;
    conf[index] = nullptr;
    confProcessor[index] = nullptr;
    lsdb[index] = nullptr;
  }

public:
  std::string routerName[2];
  std::shared_ptr<ndn::DummyClientFace> faces[2];
  std::shared_ptr<ConfParameter> conf[2];
  std::shared_ptr<DummyConfProcessor> confProcessor[2];
  std::shared_ptr<Lsdb> lsdb[2];
};

using Tests = boost::mp11::mp_list<
  LsdbFixture<SyncProtocol::PSYNC>,
  LsdbFixture<SyncProtocol::CHRONOSYNC>,
  LsdbFixture<SyncProtocol::SVS>
>;

BOOST_AUTO_TEST_SUITE(TestSeqNoRecovery)

BOOST_FIXTURE_TEST_CASE_TEMPLATE(LsaSeqNumberRecoveryFromNetwork, T, Tests, T)
{
  this->startRouter(0, 200);
  this->startRouter(1, 300);
  this->faces[0]->linkTo(*this->faces[1]);

  this->advanceClocks(1_s, 2);
  // Seq in file + 10 + 1
  BOOST_CHECK_EQUAL(this->lsdb[0]->m_sequencingManager.getNameLsaSeq(), 211);
  BOOST_CHECK_EQUAL(this->lsdb[1]->m_sequencingManager.getNameLsaSeq(), 311);

  this->killRouter(1);
  this->advanceClocks(1_s);

  // Start RouterB with sequence number of 10 (simulating file removal)
  this->startRouter(1, 10);

  this->faces[0]->linkTo(*this->faces[1]);
  this->advanceClocks(1_s, 10);
  // RouterB would receive sync update of self (from RouterA and will publish new sequence)
  BOOST_CHECK_EQUAL(this->lsdb[1]->m_sequencingManager.getNameLsaSeq(), 312);

  this->advanceClocks(3600_s);
  BOOST_CHECK_EQUAL(this->lsdb[1]->m_sequencingManager.getNameLsaSeq(), 313);
}

BOOST_AUTO_TEST_SUITE_END() // TestSeqNoRecovery

} // namespace nlsr::tests
