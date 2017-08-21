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

#include "communication/sync-logic-handler.hpp"
#include "test-common.hpp"
#include "common.hpp"
#include "nlsr.hpp"
#include "lsa.hpp"
#include "logger.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

using std::shared_ptr;

class SyncLogicFixture : public UnitTestTimeFixture
{
public:
  SyncLogicFixture()
    : face(m_ioService, m_keyChain)
    , nlsr(m_ioService, m_scheduler, face, m_keyChain)
    , testIsLsaNew([] (const ndn::Name& name, const Lsa::Type& lsaType,
                       const uint64_t sequenceNumber) {
                     return true;
                   })
    , CONFIG_NETWORK("/ndn")
    , CONFIG_SITE("/site")
    , CONFIG_ROUTER_NAME("/%C1.Router/this-router")
    , OTHER_ROUTER_NAME("/%C1.Router/other-router/")
  {
    nlsr.getConfParameter().setNetwork(CONFIG_NETWORK);
    nlsr.getConfParameter().setSiteName(CONFIG_SITE);
    nlsr.getConfParameter().setRouterName(CONFIG_ROUTER_NAME);
    nlsr.getConfParameter().buildRouterPrefix();

    conf.setNetwork(CONFIG_NETWORK);
    conf.setSiteName(CONFIG_SITE);
    conf.setRouterName(CONFIG_ROUTER_NAME);
    conf.buildRouterPrefix();

    addIdentity(conf.getRouterPrefix());

    INIT_LOGGERS("/tmp", "TRACE");
  }

  void
  receiveUpdate(std::string prefix, uint64_t seqNo, SyncLogicHandler& p_sync)
  {
    chronosync::MissingDataInfo info = {ndn::Name(prefix).appendNumber(1), 0, seqNo};

    std::vector<chronosync::MissingDataInfo> updates;
    updates.push_back(info);

    this->advanceClocks(ndn::time::milliseconds(1), 10);
    face.sentInterests.clear();

    p_sync.onChronoSyncUpdate(updates);

    this->advanceClocks(ndn::time::milliseconds(1), 10);
  }

public:
  ndn::util::DummyClientFace face;
  Nlsr nlsr;
  ConfParameter conf;
  SyncLogicHandler::IsLsaNew testIsLsaNew;

  const std::string CONFIG_NETWORK;
  const std::string CONFIG_SITE;
  const std::string CONFIG_ROUTER_NAME;
  const std::string OTHER_ROUTER_NAME;
  const std::vector<Lsa::Type> lsaTypes = {Lsa::Type::NAME, Lsa::Type::ADJACENCY,
                                             Lsa::Type::COORDINATE};
};

BOOST_FIXTURE_TEST_SUITE(TestSyncLogicHandler, SyncLogicFixture)

/* Tests that when SyncLogicHandler receives an LSA of either Name or
   Adjacency type that appears to be newer, it will emit to its signal
   with those LSA details.
 */
BOOST_AUTO_TEST_CASE(UpdateForOtherLS)
{
  SyncLogicHandler sync{face, testIsLsaNew, conf};
  sync.createSyncSocket(conf.getChronosyncPrefix());

  std::vector<Lsa::Type> lsaTypes = {Lsa::Type::NAME, Lsa::Type::ADJACENCY};

  uint64_t syncSeqNo = 1;

  for (const Lsa::Type& lsaType : lsaTypes) {
    std::string updateName = conf.getLsaPrefix().toUri() + CONFIG_SITE
      + OTHER_ROUTER_NAME + std::to_string(lsaType);

    // Actual testing done here -- signal function callback
    ndn::util::signal::ScopedConnection connection = sync.onNewLsa->connect(
      [&, this] (const ndn::Name& routerName, const uint64_t& sequenceNumber) {
        BOOST_CHECK_EQUAL(ndn::Name{updateName}, routerName);
        BOOST_CHECK_EQUAL(sequenceNumber, syncSeqNo);
      });

    receiveUpdate(updateName, syncSeqNo, sync);
  }
}

/* Tests that when SyncLogicHandler in HR mode receives an LSA of
   either Coordinate or Name type that appears to be newer, it will
   emit to its signal with those LSA details.
 */
BOOST_AUTO_TEST_CASE(UpdateForOtherHR)
{
  conf.setHyperbolicState(HYPERBOLIC_STATE_ON);

  SyncLogicHandler sync{face, testIsLsaNew, conf};
  sync.createSyncSocket(conf.getChronosyncPrefix());

  uint64_t syncSeqNo = 1;
  std::vector<Lsa::Type> lsaTypes = {Lsa::Type::NAME, Lsa::Type::COORDINATE};

  for (const Lsa::Type& lsaType : lsaTypes) {
    std::string updateName = conf.getLsaPrefix().toUri() + CONFIG_SITE
      + OTHER_ROUTER_NAME + std::to_string(lsaType);

    ndn::util::signal::ScopedConnection connection = sync.onNewLsa->connect(
      [& ,this] (const ndn::Name& routerName, const uint64_t& sequenceNumber) {
        BOOST_CHECK_EQUAL(ndn::Name{updateName}, routerName);
        BOOST_CHECK_EQUAL(sequenceNumber, syncSeqNo);
      });

    receiveUpdate(updateName, syncSeqNo, sync);
  }
}

/* Tests that when SyncLogicHandler in HR-dry mode receives an LSA of
   any type that appears to be newer, it will emit to its signal with
   those LSA details.
 */
BOOST_AUTO_TEST_CASE(UpdateForOtherHRDry)
{
  conf.setHyperbolicState(HYPERBOLIC_STATE_DRY_RUN);

  SyncLogicHandler sync{face, testIsLsaNew, conf};
  sync.createSyncSocket(conf.getChronosyncPrefix());

  for (const Lsa::Type& lsaType : lsaTypes) {
    uint64_t syncSeqNo = 1;

    std::string updateName = conf.getLsaPrefix().toUri() + CONFIG_SITE
      + OTHER_ROUTER_NAME + std::to_string(lsaType);

    ndn::util::signal::ScopedConnection connection = sync.onNewLsa->connect(
      [& ,this] (const ndn::Name& routerName, const uint64_t& sequenceNumber) {
        BOOST_CHECK_EQUAL(ndn::Name{updateName}, routerName);
        BOOST_CHECK_EQUAL(sequenceNumber, syncSeqNo);
      });

    receiveUpdate(updateName, syncSeqNo, sync);
  }
}

/* Tests that when SyncLogicHandler receives an update for an LSA with
   details matching this router's details, it will *not* emit to its
   signal those LSA details.
 */
BOOST_AUTO_TEST_CASE(NoUpdateForSelf)
{
  const uint64_t sequenceNumber = 1;

  SyncLogicHandler sync{face, testIsLsaNew, conf};
  sync.createSyncSocket(conf.getChronosyncPrefix());

  for (const Lsa::Type& lsaType : lsaTypes) {
    // To ensure that we get correctly-separated components, create
    // and modify a Name to hand off.
    ndn::Name updateName = ndn::Name{conf.getLsaPrefix()};
    updateName.append(CONFIG_SITE).append(CONFIG_ROUTER_NAME).append(std::to_string(lsaType));

    ndn::util::signal::ScopedConnection connection = sync.onNewLsa->connect(
      [& ,this] (const ndn::Name& routerName, const uint64_t& sequenceNumber) {
        BOOST_FAIL("Updates for self should not be emitted!");
      });

    receiveUpdate(updateName.toUri(), sequenceNumber, sync);
  }
}

/* Tests that when SyncLogicHandler receives an update for an LSA with
   details that do not match the expected format, it will *not* emit
   to its signal those LSA details.
 */
BOOST_AUTO_TEST_CASE(MalformedUpdate)
{
  const uint64_t sequenceNumber = 1;

  SyncLogicHandler sync{face, testIsLsaNew, conf};
  sync.createSyncSocket(conf.getChronosyncPrefix());

  for (const Lsa::Type& lsaType : lsaTypes) {
    ndn::Name updateName{CONFIG_SITE};
    updateName.append(CONFIG_ROUTER_NAME).append(std::to_string(lsaType));

    ndn::util::signal::ScopedConnection connection = sync.onNewLsa->connect(
      [& ,this] (const ndn::Name& routerName, const uint64_t& sequenceNumber) {
        BOOST_FAIL("Malformed updates should not be emitted!");
      });

    receiveUpdate(updateName.toUri(), sequenceNumber, sync);
  }
}

/* Tests that when SyncLogicHandler receives an update for an LSA with
   details that do not appear to be new, it will *not* emit to its
   signal those LSA details.
 */
BOOST_AUTO_TEST_CASE(LsaNotNew)
{
  auto testLsaAlwaysFalse = [] (const ndn::Name& routerName, const Lsa::Type& lsaType,
                           const uint64_t& sequenceNumber) {
    return false;
  };

  const uint64_t sequenceNumber = 1;
  SyncLogicHandler sync{face, testLsaAlwaysFalse, conf};
  sync.createSyncSocket(conf.getChronosyncPrefix());
    ndn::util::signal::ScopedConnection connection = sync.onNewLsa->connect(
      [& ,this] (const ndn::Name& routerName, const uint64_t& sequenceNumber) {
        BOOST_FAIL("An update for an LSA with non-new sequence number should not emit!");
      });

  std::string updateName = nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_SITE + "/%C1.Router/other-router/" +
                           std::to_string(Lsa::Type::NAME);

  receiveUpdate(updateName, sequenceNumber, sync);
}

/* Tests that SyncLogicHandler successfully concatenates configured
   variables together to form the necessary prefixes to advertise
   through ChronoSync.
 */
BOOST_AUTO_TEST_CASE(UpdatePrefix)
{

  SyncLogicHandler sync{face, testIsLsaNew, conf};

  ndn::Name expectedPrefix = nlsr.getConfParameter().getLsaPrefix();
  expectedPrefix.append(CONFIG_SITE);
  expectedPrefix.append(CONFIG_ROUTER_NAME);

  sync.buildUpdatePrefix();

  BOOST_CHECK_EQUAL(sync.m_nameLsaUserPrefix,
                    ndn::Name(expectedPrefix).append(std::to_string(Lsa::Type::NAME)));
  BOOST_CHECK_EQUAL(sync.m_adjLsaUserPrefix,
                    ndn::Name(expectedPrefix).append(std::to_string(Lsa::Type::ADJACENCY)));
  BOOST_CHECK_EQUAL(sync.m_coorLsaUserPrefix,
                    ndn::Name(expectedPrefix).append(std::to_string(Lsa::Type::COORDINATE)));
}

/* Tests that SyncLogicHandler's socket will be created when
   Nlsr::initialize is called, preventing use of sync before the
   socket is created.

   NB: This test is as much an Nlsr class test as a
   SyncLogicHandler class test, but it rides the line and ends up here.
 */
BOOST_AUTO_TEST_CASE(CreateSyncSocketOnInitialization) // Bug #2649
{
  nlsr.initialize();

  // Make sure an adjacency LSA has not been built yet
  ndn::Name key = ndn::Name(nlsr.getConfParameter().getRouterPrefix()).append(std::to_string(Lsa::Type::ADJACENCY));
  AdjLsa* lsa = nlsr.getLsdb().findAdjLsa(key);
  BOOST_REQUIRE(lsa == nullptr);

  // Publish a routing update before an Adjacency LSA is built
  BOOST_CHECK_NO_THROW(nlsr.getLsdb().getSyncLogicHandler()
                       .publishRoutingUpdate(Lsa::Type::ADJACENCY, 0));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
