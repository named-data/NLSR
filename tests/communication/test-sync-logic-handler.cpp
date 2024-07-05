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

#include "communication/sync-logic-handler.hpp"
#include "nlsr.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

namespace nlsr::tests {

class SyncLogicFixture : public IoKeyChainFixture
{
public:
  SyncLogicFixture()
  {
    m_keyChain.createIdentity(opts.routerPrefix);
  }

  SyncLogicHandler&
  getSync()
  {
    if (m_sync == nullptr) {
      m_sync = std::make_unique<SyncLogicHandler>(face, m_keyChain, testIsLsaNew, opts);
    }
    return *m_sync;
  }

  void
  receiveUpdate(const ndn::Name& prefix, uint64_t seqNo)
  {
    this->advanceClocks(1_ms, 10);
    face.sentInterests.clear();

#ifdef HAVE_PSYNC
    std::vector<psync::MissingDataInfo> updates;
    updates.push_back({prefix, 0, seqNo, 0});
    getSync().m_syncLogic.onPSyncUpdate(updates);
#endif

    this->advanceClocks(1_ms, 10);
  }

public:
  ndn::DummyClientFace face{m_io, m_keyChain};
  SyncLogicHandler::IsLsaNew testIsLsaNew = [] (auto&&...) { return true; };
  SyncLogicOptions opts{
    SyncProtocol::PSYNC,
    ndn::Name("/ndn/nlsr/sync").appendVersion(ConfParameter::SYNC_VERSION),
    "/localhop/ndn/nlsr/LSA/site/%C1.Router/this-router",
    time::milliseconds(SYNC_INTEREST_LIFETIME_DEFAULT),
    "/ndn/site/%C1.Router/this-router",
    HYPERBOLIC_STATE_OFF
  };

  ndn::Name otherRouter = "/localhop/ndn/nlsr/LSA/site/%C1.Router/other-router";
  const std::vector<Lsa::Type> lsaTypes{Lsa::Type::NAME,
                                        Lsa::Type::ADJACENCY,
                                        Lsa::Type::COORDINATE};

private:
  std::unique_ptr<SyncLogicHandler> m_sync;
};

BOOST_FIXTURE_TEST_SUITE(TestSyncLogicHandler, SyncLogicFixture)

/* Tests that when SyncLogicHandler receives an LSA of either Name or
   Adjacency type that appears to be newer, it will emit to its signal
   with those LSA details.
 */
BOOST_AUTO_TEST_CASE(UpdateForOtherLS)
{
  size_t nCallbacks = 0;
  uint64_t syncSeqNo = 1;

  for (auto lsaType : {Lsa::Type::NAME, Lsa::Type::ADJACENCY}) {
    auto updateName = makeLsaUserPrefix(otherRouter, lsaType);

    ndn::signal::ScopedConnection connection = getSync().onNewLsa.connect(
      [&] (const auto& routerName, uint64_t sequenceNumber, const auto& originRouter, uint64_t incomingFaceId) {
        BOOST_CHECK_EQUAL(updateName, routerName);
        BOOST_CHECK_EQUAL(sequenceNumber, syncSeqNo);
        ++nCallbacks;
      });

    this->receiveUpdate(updateName, syncSeqNo);
  }

  BOOST_CHECK_EQUAL(nCallbacks, 2);
}

/* Tests that when SyncLogicHandler in HR mode receives an LSA of
   either Coordinate or Name type that appears to be newer, it will
   emit to its signal with those LSA details.
 */
BOOST_AUTO_TEST_CASE(UpdateForOtherHR)
{
  opts.hyperbolicState = HYPERBOLIC_STATE_ON;

  size_t nCallbacks = 0;
  uint64_t syncSeqNo = 1;

  for (auto lsaType : {Lsa::Type::NAME, Lsa::Type::COORDINATE}) {
    auto updateName = makeLsaUserPrefix(otherRouter, lsaType);

    ndn::signal::ScopedConnection connection = getSync().onNewLsa.connect(
      [&] (const auto& routerName, uint64_t sequenceNumber, const auto& originRouter, uint64_t incomingFaceId) {
        BOOST_CHECK_EQUAL(updateName, routerName);
        BOOST_CHECK_EQUAL(sequenceNumber, syncSeqNo);
        ++nCallbacks;
      });

    this->receiveUpdate(updateName, syncSeqNo);
  }

  BOOST_CHECK_EQUAL(nCallbacks, 2);
}

/* Tests that when SyncLogicHandler in HR-dry mode receives an LSA of
   any type that appears to be newer, it will emit to its signal with
   those LSA details.
 */
BOOST_AUTO_TEST_CASE(UpdateForOtherHRDry)
{
  opts.hyperbolicState = HYPERBOLIC_STATE_DRY_RUN;

  size_t nCallbacks = 0;
  uint64_t syncSeqNo = 1;

  for (auto lsaType : this->lsaTypes) {
    auto updateName = makeLsaUserPrefix(otherRouter, lsaType);

    ndn::signal::ScopedConnection connection = getSync().onNewLsa.connect(
      [&] (const auto& routerName, uint64_t sequenceNumber, const auto& originRouter, uint64_t incomingFaceId) {
        BOOST_CHECK_EQUAL(updateName, routerName);
        BOOST_CHECK_EQUAL(sequenceNumber, syncSeqNo);
        ++nCallbacks;
      });

    this->receiveUpdate(updateName, syncSeqNo);
  }

  BOOST_CHECK_EQUAL(nCallbacks, 3);
}

/* Tests that when SyncLogicHandler receives an update for an LSA with
   details matching this router's details, it will *not* emit to its
   signal those LSA details.
 */
BOOST_AUTO_TEST_CASE(NoUpdateForSelf)
{
  const uint64_t sequenceNumber = 1;

  for (auto lsaType : this->lsaTypes) {
    auto updateName = makeLsaUserPrefix(opts.routerPrefix, lsaType);

    ndn::signal::ScopedConnection connection = getSync().onNewLsa.connect(
      [&] (const auto& routerName, uint64_t sequenceNumber, const auto& originRouter, uint64_t incomingFaceId) {
        BOOST_FAIL("Updates for self should not be emitted!");
      });

    this->receiveUpdate(updateName, sequenceNumber);
  }

  // avoid "test case [...] did not check any assertions" message from Boost.Test
  BOOST_CHECK(true);
}

/* Tests that when SyncLogicHandler receives an update for an LSA with
   details that do not match the expected format, it will *not* emit
   to its signal those LSA details.
 */
BOOST_AUTO_TEST_CASE(MalformedUpdate)
{
  const uint64_t sequenceNumber = 1;

  for (auto lsaType : this->lsaTypes) {
    auto updateName = makeLsaUserPrefix("/site/%C1.Router/this-router", lsaType);

    ndn::signal::ScopedConnection connection = getSync().onNewLsa.connect(
      [&] (const auto& routerName, uint64_t sequenceNumber, const auto& originRouter, uint64_t incomingFaceId) {
        BOOST_FAIL("Malformed updates should not be emitted!");
      });

    this->receiveUpdate(updateName, sequenceNumber);
  }

  // avoid "test case [...] did not check any assertions" message from Boost.Test
  BOOST_CHECK(true);
}

/* Tests that when SyncLogicHandler receives an update for an LSA with
   details that do not appear to be new, it will *not* emit to its
   signal those LSA details.
 */
BOOST_AUTO_TEST_CASE(LsaNotNew)
{
  testIsLsaNew = [] (const ndn::Name& routerName, const Lsa::Type& lsaType,
                     const uint64_t& sequenceNumber, uint64_t incomingFaceId) {
    return false;
  };

  const uint64_t sequenceNumber = 1;
  ndn::signal::ScopedConnection connection = getSync().onNewLsa.connect(
    [&] (const auto& routerName, uint64_t sequenceNumber, const auto& originRouter, uint64_t incomingFaceId) {
      BOOST_FAIL("An update for an LSA with non-new sequence number should not emit!");
    });

  auto updateName = makeLsaUserPrefix(otherRouter, Lsa::Type::NAME);
  this->receiveUpdate(updateName, sequenceNumber);

  // avoid "test case [...] did not check any assertions" message from Boost.Test
  BOOST_CHECK(true);
}

/* Tests that SyncLogicHandler successfully concatenates configured
   variables together to form the necessary prefixes to advertise
   through sync.
 */
BOOST_AUTO_TEST_CASE(UpdatePrefix)
{
  BOOST_CHECK_EQUAL(getSync().m_nameLsaUserPrefix,
                    ndn::Name(opts.userPrefix).append(boost::lexical_cast<std::string>(Lsa::Type::NAME)));
  BOOST_CHECK_EQUAL(getSync().m_adjLsaUserPrefix,
                    ndn::Name(opts.userPrefix).append(boost::lexical_cast<std::string>(Lsa::Type::ADJACENCY)));
  BOOST_CHECK_EQUAL(getSync().m_coorLsaUserPrefix,
                    ndn::Name(opts.userPrefix).append(boost::lexical_cast<std::string>(Lsa::Type::COORDINATE)));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
