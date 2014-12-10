/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 **/

#include "test-common.hpp"
#include "dummy-face.hpp"

#include "nlsr.hpp"
#include "communication/sync-logic-handler.hpp"

namespace nlsr {
namespace test {

using ndn::DummyFace;
using ndn::shared_ptr;

class SyncLogicFixture : public BaseFixture
{
public:
  SyncLogicFixture()
    : face(ndn::makeDummyFace())
    , nlsr(g_ioService, g_scheduler, ndn::ref(*face))
    , sync(*face, nlsr.getLsdb(), nlsr.getConfParameter(), nlsr.getSequencingManager())
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
  receiveUpdate(std::string prefix, uint64_t seqNo)
  {
    Sync::MissingDataInfo info = {prefix, 0, seqNo};

    std::vector<Sync::MissingDataInfo> updates;
    updates.push_back(info);

    face->processEvents(ndn::time::milliseconds(1));
    face->m_sentInterests.clear();

    sync.onNsyncUpdate(updates, NULL);

    face->processEvents(ndn::time::milliseconds(1));
  }

public:
  shared_ptr<DummyFace> face;
  Nlsr nlsr;
  SyncLogicHandler sync;

  const std::string CONFIG_NETWORK;
  const std::string CONFIG_SITE;
  const std::string CONFIG_ROUTER_NAME;
};

BOOST_FIXTURE_TEST_SUITE(TestSyncLogicHandler, SyncLogicFixture)

BOOST_AUTO_TEST_CASE(UpdateForOther)
{
  std::string updateName = nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_SITE + "/%C1.Router/other-router/";

  receiveUpdate(updateName, 1);

  std::vector<ndn::Interest>& interests = face->m_sentInterests;
  BOOST_REQUIRE_EQUAL(interests.size(), 3);

  std::vector<ndn::Interest>::iterator it = interests.begin();
  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + "name/");

  ++it;
  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + "adjacency/");

  ++it;
  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + "coordinate/");
}

BOOST_AUTO_TEST_CASE(NoUpdateForSelf)
{
  std::string updateName = nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_SITE + CONFIG_ROUTER_NAME;

  receiveUpdate(updateName, 1);

  std::vector<ndn::Interest>& interests = face->m_sentInterests;
  BOOST_CHECK_EQUAL(interests.size(), 0);
}

BOOST_AUTO_TEST_CASE(MalformedUpdate)
{
  std::string updateName = CONFIG_SITE + nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_ROUTER_NAME;

  std::vector<ndn::Interest>& interests = face->m_sentInterests;
  BOOST_CHECK_EQUAL(interests.size(), 0);
}

BOOST_AUTO_TEST_CASE(SequenceNumber)
{
  std::string originRouter = CONFIG_NETWORK + CONFIG_SITE + "/%C1.Router/other-router/";

  Lsdb& lsdb = nlsr.getLsdb();

  // Install Name LSA
  NamePrefixList nameList;
  NameLsa lsa(originRouter, "name", 999, ndn::time::system_clock::TimePoint::max(), nameList);
  lsdb.installNameLsa(lsa);

  // Install Adj LSA
  AdjacencyList adjList;
  AdjLsa adjLsa(originRouter, "adjacency", 1000, ndn::time::system_clock::TimePoint::max(),
                3 , adjList);
  lsdb.installAdjLsa(adjLsa);

  // Install Cor LSA
  CoordinateLsa corLsa(originRouter, "coordinate", 1000, ndn::time::system_clock::TimePoint::max(),
                       0,0);
  lsdb.installCoordinateLsa(corLsa);

  std::string updateName = nlsr.getConfParameter().getLsaPrefix().toUri() +
                           CONFIG_SITE + "/%C1.Router/other-router/";

  // Lower NameLSA sequence number
  uint64_t lowerSeqNo = static_cast<uint64_t>(998) << 40;
  receiveUpdate(updateName, lowerSeqNo);

  std::vector<ndn::Interest>& interests = face->m_sentInterests;
  BOOST_REQUIRE_EQUAL(interests.size(), 0);

  // Same NameLSA sequence number
  uint64_t sameSeqNo = static_cast<uint64_t>(999) << 40;
  receiveUpdate(updateName, sameSeqNo);

  interests = face->m_sentInterests;
  BOOST_REQUIRE_EQUAL(interests.size(), 0);

  // Higher NameLSA sequence number
  uint64_t higherSeqNo = static_cast<uint64_t>(1000) << 40;
  receiveUpdate(updateName, higherSeqNo);

  interests = face->m_sentInterests;
  BOOST_REQUIRE_EQUAL(interests.size(), 1);

  std::vector<ndn::Interest>::iterator it = interests.begin();
  BOOST_CHECK_EQUAL(it->getName().getPrefix(-1), updateName + "name/");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
