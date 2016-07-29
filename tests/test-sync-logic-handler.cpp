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

#include "test-common.hpp"

#include "nlsr.hpp"
#include "communication/sync-logic-handler.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

using ndn::shared_ptr;

class SyncLogicFixture : public BaseFixture
{
public:
  SyncLogicFixture()
    : face(make_shared<ndn::util::DummyClientFace>())
    , nlsr(g_ioService, g_scheduler, ndn::ref(*face))
    , sync(nlsr.getSyncLogicHandler())
    , CONFIG_NETWORK("/ndn")
    , CONFIG_SITE("/site")
    , CONFIG_ROUTER_NAME("/%C1.Router/this-router")
  {
    nlsr.getConfParameter().setNetwork(CONFIG_NETWORK);
    nlsr.getConfParameter().setSiteName(CONFIG_SITE);
    nlsr.getConfParameter().setRouterName(CONFIG_ROUTER_NAME);
    nlsr.getConfParameter().buildRouterPrefix();
  }

  void
  receiveUpdate(std::string prefix, uint64_t seqNo, SyncLogicHandler& p_sync)
  {
    Sync::MissingDataInfo info = {prefix, 0, seqNo};

    std::vector<Sync::MissingDataInfo> updates;
    updates.push_back(info);

    face->processEvents(ndn::time::milliseconds(1));
    face->sentInterests.clear();

    p_sync.onNsyncUpdate(updates, NULL);

    face->processEvents(ndn::time::milliseconds(1));
  }

public:
  shared_ptr<ndn::util::DummyClientFace> face;
  Nlsr nlsr;
  SyncLogicHandler& sync;

  const std::string CONFIG_NETWORK;
  const std::string CONFIG_SITE;
  const std::string CONFIG_ROUTER_NAME;
};

BOOST_FIXTURE_TEST_SUITE(TestSyncLogicHandler, SyncLogicFixture)

BOOST_AUTO_TEST_CASE(UpdateForOtherLS)
{
  std::string updateName = nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_SITE + "/%C1.Router/other-router/";

  uint64_t syncSeqNo = 1;
  receiveUpdate(updateName, syncSeqNo, sync);

  std::vector<ndn::Interest>& interests = face->sentInterests;

  std::vector<ndn::Interest>::iterator it = interests.begin();

  BOOST_REQUIRE_EQUAL(interests.size(), 2);

  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + NameLsa::TYPE_STRING + "/");

  ++it;
  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + AdjLsa::TYPE_STRING + "/");
}

BOOST_AUTO_TEST_CASE(UpdateForOtherHR)
{
  Nlsr nlsr_hr(g_ioService, g_scheduler, ndn::ref(*face));
  SyncLogicHandler& sync_hr(nlsr_hr.getSyncLogicHandler());

  nlsr_hr.getConfParameter().setNetwork(CONFIG_NETWORK);
  nlsr_hr.getConfParameter().setSiteName(CONFIG_SITE);
  nlsr_hr.getConfParameter().setRouterName(CONFIG_ROUTER_NAME);
  nlsr_hr.getConfParameter().buildRouterPrefix();

  nlsr_hr.getConfParameter().setHyperbolicState(HYPERBOLIC_STATE_ON);

  nlsr_hr.initialize();

  std::string updateName = nlsr_hr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_SITE + "/%C1.Router/other-router/";

  uint64_t syncSeqNo = 0;
  syncSeqNo = syncSeqNo | (static_cast<uint64_t>(1) << 20);

  receiveUpdate(updateName, syncSeqNo, sync_hr);

  std::vector<ndn::Interest>& interests = face->sentInterests;
  std::vector<ndn::Interest>::iterator it = interests.begin();

  BOOST_REQUIRE_EQUAL(interests.size(), 2);

  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + NameLsa::TYPE_STRING + "/");

  ++it;
  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + CoordinateLsa::TYPE_STRING + "/");
}

BOOST_AUTO_TEST_CASE(UpdateForOtherHRDry)
{

  Nlsr nlsr_hrdry(g_ioService, g_scheduler, ndn::ref(*face));
  SyncLogicHandler& sync_hrdry(nlsr_hrdry.getSyncLogicHandler());

  nlsr_hrdry.getConfParameter().setNetwork(CONFIG_NETWORK);
  nlsr_hrdry.getConfParameter().setSiteName(CONFIG_SITE);
  nlsr_hrdry.getConfParameter().setRouterName(CONFIG_ROUTER_NAME);
  nlsr_hrdry.getConfParameter().buildRouterPrefix();

  nlsr_hrdry.getConfParameter().setHyperbolicState(HYPERBOLIC_STATE_DRY_RUN);

  nlsr_hrdry.initialize();

  std::string updateName = nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_SITE + "/%C1.Router/other-router/";


  uint64_t syncSeqNo = 1;
  syncSeqNo = syncSeqNo | (static_cast<uint64_t>(1) << 20);
  receiveUpdate(updateName, syncSeqNo, sync_hrdry);

  std::vector<ndn::Interest>& interests = face->sentInterests;
  std::vector<ndn::Interest>::iterator it = interests.begin();

  BOOST_REQUIRE_EQUAL(interests.size(), 3);

  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + NameLsa::TYPE_STRING + "/");

  if (nlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_ON) {
      ++it;
      BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + CoordinateLsa::TYPE_STRING + "/");
  }
  else if (nlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_OFF) {
      ++it;
      BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + AdjLsa::TYPE_STRING + "/");
  }
  else {
      ++it;
      BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + CoordinateLsa::TYPE_STRING + "/");

      ++it;
      BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + AdjLsa::TYPE_STRING + "/");
  }
}

BOOST_AUTO_TEST_CASE(NoUpdateForSelf)
{
  std::string updateName = nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_SITE + CONFIG_ROUTER_NAME;

  receiveUpdate(updateName, 1, sync);

  std::vector<ndn::Interest>& interests = face->sentInterests;
  BOOST_CHECK_EQUAL(interests.size(), 0);
}

BOOST_AUTO_TEST_CASE(MalformedUpdate)
{
  std::string updateName = CONFIG_SITE + nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_ROUTER_NAME;

  std::vector<ndn::Interest>& interests = face->sentInterests;
  BOOST_CHECK_EQUAL(interests.size(), 0);
}

BOOST_AUTO_TEST_CASE(SequenceNumber)
{
  std::string originRouter = CONFIG_NETWORK + CONFIG_SITE + "/%C1.Router/other-router/";

  Lsdb& lsdb = nlsr.getLsdb();

  // Install Name LSA
  NamePrefixList nameList;
  NameLsa lsa(originRouter, 999, ndn::time::system_clock::TimePoint::max(), nameList);
  lsdb.installNameLsa(lsa);

  // Install Adj LSA
  AdjacencyList adjList;
  AdjLsa adjLsa(originRouter, 1000, ndn::time::system_clock::TimePoint::max(),
                3 , adjList);
  lsdb.installAdjLsa(adjLsa);

  // Install Cor LSA
  CoordinateLsa corLsa(originRouter, 1000, ndn::time::system_clock::TimePoint::max(),
                       0,0);
  lsdb.installCoordinateLsa(corLsa);

  std::string updateName = nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_SITE + "/%C1.Router/other-router/";

  // Lower NameLSA sequence number
  uint64_t lowerSeqNo = static_cast<uint64_t>(998) << 40;
  receiveUpdate(updateName, lowerSeqNo, sync);

  std::vector<ndn::Interest>& interests = face->sentInterests;
  BOOST_REQUIRE_EQUAL(interests.size(), 0);

  // Same NameLSA sequence number
  uint64_t sameSeqNo = static_cast<uint64_t>(999) << 40;
  receiveUpdate(updateName, sameSeqNo, sync);

  interests = face->sentInterests;
  BOOST_REQUIRE_EQUAL(interests.size(), 0);

  // Higher NameLSA sequence number
  uint64_t higherSeqNo = static_cast<uint64_t>(1000) << 40;
  receiveUpdate(updateName, higherSeqNo, sync);

  interests = face->sentInterests;
  BOOST_REQUIRE_EQUAL(interests.size(), 1);

  std::vector<ndn::Interest>::iterator it = interests.begin();
  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + "name/");
}

BOOST_AUTO_TEST_CASE(UpdatePrefix)
{
  ndn::Name expectedPrefix = nlsr.getConfParameter().getLsaPrefix();
  expectedPrefix.append(CONFIG_SITE);
  expectedPrefix.append(CONFIG_ROUTER_NAME);

  nlsr.initialize();

  BOOST_CHECK_EQUAL(sync.m_updatePrefix, expectedPrefix);
}

BOOST_AUTO_TEST_CASE(CreateSyncSocketOnInitialization) // Bug #2649
{
  nlsr.initialize();

  // Make sure an adjacency LSA has not been built yet
  ndn::Name key = ndn::Name(nlsr.getConfParameter().getRouterPrefix()).append(AdjLsa::TYPE_STRING);
  AdjLsa* lsa = nlsr.getLsdb().findAdjLsa(key);
  BOOST_REQUIRE(lsa == nullptr);

  // Publish a routing update before an Adjacency LSA is built
  BOOST_CHECK_NO_THROW(sync.publishRoutingUpdate());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
