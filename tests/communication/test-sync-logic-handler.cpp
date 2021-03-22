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

#include "communication/sync-logic-handler.hpp"
#include "tests/test-common.hpp"
#include "common.hpp"
#include "nlsr.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>
#include <boost/lexical_cast.hpp>

namespace nlsr {
namespace test {

using std::shared_ptr;

class SyncLogicFixture : public UnitTestTimeFixture
{
public:
  SyncLogicFixture()
    : face(m_ioService, m_keyChain)
    , conf(face, m_keyChain)
    , confProcessor(conf, SYNC_PROTOCOL_PSYNC)
    , testIsLsaNew([] (const ndn::Name& name, const Lsa::Type& lsaType,
                       const uint64_t sequenceNumber) {
                     return true;
                   })
    , sync(face, testIsLsaNew, conf)
    , updateNamePrefix(this->conf.getLsaPrefix().toUri() +
                       this->conf.getSiteName().toUri() +
                       "/%C1.Router/other-router/")
  {
    addIdentity(conf.getRouterPrefix());
  }

  void
  receiveUpdate(const std::string& prefix, uint64_t seqNo)
  {
    this->advanceClocks(ndn::time::milliseconds(1), 10);
    face.sentInterests.clear();

    std::vector<psync::MissingDataInfo> updates;
    updates.push_back({ndn::Name(prefix), 0, seqNo});
    sync.m_syncLogic.onPSyncUpdate(updates);

    this->advanceClocks(ndn::time::milliseconds(1), 10);
  }

public:
  ndn::util::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  SyncLogicHandler::IsLsaNew testIsLsaNew;
  SyncLogicHandler sync;

  const std::string updateNamePrefix;
  const std::vector<Lsa::Type> lsaTypes = {Lsa::Type::NAME, Lsa::Type::ADJACENCY,
                                             Lsa::Type::COORDINATE};
};

BOOST_AUTO_TEST_SUITE(TestSyncLogicHandler)

/* Tests that when SyncLogicHandler receives an LSA of either Name or
   Adjacency type that appears to be newer, it will emit to its signal
   with those LSA details.
 */
BOOST_FIXTURE_TEST_CASE(UpdateForOtherLS, SyncLogicFixture)
{
  std::vector<Lsa::Type> lsaTypes = {Lsa::Type::NAME, Lsa::Type::ADJACENCY};

  uint64_t syncSeqNo = 1;

  for (const Lsa::Type& lsaType : lsaTypes) {
    std::string updateName = this->updateNamePrefix + boost::lexical_cast<std::string>(lsaType);

    // Actual testing done here -- signal function callback
    ndn::util::signal::ScopedConnection connection = this->sync.onNewLsa->connect(
      [&] (const ndn::Name& routerName, const uint64_t& sequenceNumber,
           const ndn::Name& originRouter) {
        BOOST_CHECK_EQUAL(ndn::Name{updateName}, routerName);
        BOOST_CHECK_EQUAL(sequenceNumber, syncSeqNo);
      });

    this->receiveUpdate(updateName, syncSeqNo);
  }
}

/* Tests that when SyncLogicHandler in HR mode receives an LSA of
   either Coordinate or Name type that appears to be newer, it will
   emit to its signal with those LSA details.
 */
BOOST_FIXTURE_TEST_CASE(UpdateForOtherHR, SyncLogicFixture)
{
  this->conf.setHyperbolicState(HYPERBOLIC_STATE_ON);

  uint64_t syncSeqNo = 1;
  std::vector<Lsa::Type> lsaTypes = {Lsa::Type::NAME, Lsa::Type::COORDINATE};

  for (const Lsa::Type& lsaType : lsaTypes) {
    std::string updateName = this->updateNamePrefix + boost::lexical_cast<std::string>(lsaType);

    ndn::util::signal::ScopedConnection connection = this->sync.onNewLsa->connect(
      [&] (const ndn::Name& routerName, const uint64_t& sequenceNumber,
           const ndn::Name& originRouter) {
        BOOST_CHECK_EQUAL(ndn::Name{updateName}, routerName);
        BOOST_CHECK_EQUAL(sequenceNumber, syncSeqNo);
      });

    this->receiveUpdate(updateName, syncSeqNo);
  }
}

/* Tests that when SyncLogicHandler in HR-dry mode receives an LSA of
   any type that appears to be newer, it will emit to its signal with
   those LSA details.
 */
BOOST_FIXTURE_TEST_CASE(UpdateForOtherHRDry, SyncLogicFixture)
{
  this->conf.setHyperbolicState(HYPERBOLIC_STATE_DRY_RUN);

  uint64_t syncSeqNo = 1;

  for (const Lsa::Type& lsaType : this->lsaTypes) {
    std::string updateName = this->updateNamePrefix + boost::lexical_cast<std::string>(lsaType);

    ndn::util::signal::ScopedConnection connection = this->sync.onNewLsa->connect(
      [&] (const ndn::Name& routerName, const uint64_t& sequenceNumber,
           const ndn::Name& originRouter) {
        BOOST_CHECK_EQUAL(ndn::Name{updateName}, routerName);
        BOOST_CHECK_EQUAL(sequenceNumber, syncSeqNo);
      });

    this->receiveUpdate(updateName, syncSeqNo);
  }
}

/* Tests that when SyncLogicHandler receives an update for an LSA with
   details matching this router's details, it will *not* emit to its
   signal those LSA details.
 */
BOOST_FIXTURE_TEST_CASE(NoUpdateForSelf, SyncLogicFixture)
{
  const uint64_t sequenceNumber = 1;

  for (const Lsa::Type& lsaType : this->lsaTypes) {
    // To ensure that we get correctly-separated components, create
    // and modify a Name to hand off.
    ndn::Name updateName = ndn::Name{this->conf.getLsaPrefix()};
    updateName.append(this->conf.getSiteName())
              .append(this->conf.getRouterName())
              .append(boost::lexical_cast<std::string>(lsaType));

    ndn::util::signal::ScopedConnection connection = this->sync.onNewLsa->connect(
      [&] (const ndn::Name& routerName, const uint64_t& sequenceNumber,
           const ndn::Name& originRouter) {
        BOOST_FAIL("Updates for self should not be emitted!");
      });

    this->receiveUpdate(updateName.toUri(), sequenceNumber);
  }
}

/* Tests that when SyncLogicHandler receives an update for an LSA with
   details that do not match the expected format, it will *not* emit
   to its signal those LSA details.
 */
BOOST_FIXTURE_TEST_CASE(MalformedUpdate, SyncLogicFixture)
{
  const uint64_t sequenceNumber = 1;

  for (const Lsa::Type& lsaType : this->lsaTypes) {
    ndn::Name updateName{this->conf.getSiteName()};
    updateName.append(this->conf.getRouterName()).append(boost::lexical_cast<std::string>(lsaType));

    ndn::util::signal::ScopedConnection connection = this->sync.onNewLsa->connect(
      [&] (const ndn::Name& routerName, const uint64_t& sequenceNumber,
           const ndn::Name& originRouter) {
        BOOST_FAIL("Malformed updates should not be emitted!");
      });

    this->receiveUpdate(updateName.toUri(), sequenceNumber);
  }
}

/* Tests that when SyncLogicHandler receives an update for an LSA with
   details that do not appear to be new, it will *not* emit to its
   signal those LSA details.
 */
BOOST_FIXTURE_TEST_CASE(LsaNotNew, SyncLogicFixture)
{
  auto testLsaAlwaysFalse = [] (const ndn::Name& routerName, const Lsa::Type& lsaType,
                                const uint64_t& sequenceNumber) {
    return false;
  };

  const uint64_t sequenceNumber = 1;
  SyncLogicHandler sync{this->face, testLsaAlwaysFalse, this->conf};
    ndn::util::signal::ScopedConnection connection = sync.onNewLsa->connect(
      [&] (const ndn::Name& routerName, const uint64_t& sequenceNumber,
           const ndn::Name& originRouter) {
        BOOST_FAIL("An update for an LSA with non-new sequence number should not emit!");
      });

  std::string updateName = this->updateNamePrefix + boost::lexical_cast<std::string>(Lsa::Type::NAME);

  this->receiveUpdate(updateName, sequenceNumber);
}

/* Tests that SyncLogicHandler successfully concatenates configured
   variables together to form the necessary prefixes to advertise
   through sync.
 */
BOOST_FIXTURE_TEST_CASE(UpdatePrefix, SyncLogicFixture)
{
  ndn::Name expectedPrefix = this->conf.getLsaPrefix();
  expectedPrefix.append(this->conf.getSiteName());
  expectedPrefix.append(this->conf.getRouterName());

  BOOST_CHECK_EQUAL(this->sync.m_nameLsaUserPrefix,
                    ndn::Name(expectedPrefix).append(boost::lexical_cast<std::string>(Lsa::Type::NAME)));
  BOOST_CHECK_EQUAL(this->sync.m_adjLsaUserPrefix,
                    ndn::Name(expectedPrefix).append(boost::lexical_cast<std::string>(Lsa::Type::ADJACENCY)));
  BOOST_CHECK_EQUAL(this->sync.m_coorLsaUserPrefix,
                    ndn::Name(expectedPrefix).append(boost::lexical_cast<std::string>(Lsa::Type::COORDINATE)));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
