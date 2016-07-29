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

#include "nlsr.hpp"
#include "test-common.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

class NamePrefixTableFixture : public UnitTestTimeFixture
{
public:
  NamePrefixTableFixture()
    : face(make_shared<ndn::util::DummyClientFace>(g_ioService))
    , nlsr(g_ioService, g_scheduler, ndn::ref(*face))
    , lsdb(nlsr.getLsdb())
    , npt(nlsr.getNamePrefixTable())
  {
    INIT_LOGGERS("/tmp", "DEBUG");
  }

public:
  shared_ptr<ndn::util::DummyClientFace> face;
  Nlsr nlsr;

  Lsdb& lsdb;
  NamePrefixTable& npt;
};

BOOST_AUTO_TEST_SUITE(TestNamePrefixTable)

BOOST_FIXTURE_TEST_CASE(Bupt, NamePrefixTableFixture)
{
  ConfParameter& conf = nlsr.getConfParameter();
  conf.setNetwork("/ndn");
  conf.setSiteName("/router");
  conf.setRouterName("/a");
  conf.buildRouterPrefix();

  RoutingTable& routingTable = nlsr.getRoutingTable();
  routingTable.setRoutingCalcInterval(0);

  NamePrefixTable& npt = nlsr.getNamePrefixTable();

  Adjacent thisRouter(conf.getRouterPrefix(), "udp://face", 0, Adjacent::STATUS_ACTIVE, 0, 0);

  ndn::Name buptRouterName("/ndn/cn/edu/bupt/%C1.Router/bupthub");
  Adjacent bupt(buptRouterName, "udp://bupt", 0, Adjacent::STATUS_ACTIVE, 0, 0);

  // This router's Adjacency LSA
  nlsr.getAdjacencyList().insert(bupt);
  AdjLsa thisRouterAdjLsa(thisRouter.getName(), 1,
                          ndn::time::system_clock::now() + ndn::time::seconds::max(),
                          2,
                          nlsr.getAdjacencyList());

  lsdb.installAdjLsa(thisRouterAdjLsa);

  // BUPT Adjacency LSA
  AdjacencyList buptAdjacencies;
  buptAdjacencies.insert(thisRouter);
  AdjLsa buptAdjLsa(buptRouterName, 1,
                    ndn::time::system_clock::now() + ndn::time::seconds(5),
                    0 , buptAdjacencies);

  lsdb.installAdjLsa(buptAdjLsa);

  // BUPT Name LSA
  ndn::Name buptAdvertisedName("/ndn/cn/edu/bupt");

  NamePrefixList buptNames;
  buptNames.insert(buptAdvertisedName);

  NameLsa buptNameLsa(buptRouterName, 1, ndn::time::system_clock::now(),
                      buptNames);

  lsdb.installNameLsa(buptNameLsa);

  // Advance clocks to expire LSAs
  this->advanceClocks(ndn::time::seconds(15));

  // LSA expirations should cause NPT entries to be completely removed
  NamePrefixTable::const_iterator it = npt.begin();
  BOOST_REQUIRE(it == npt.end());

  // Install new name LSA
  NameLsa buptNewNameLsa(buptRouterName, 12,
                         ndn::time::system_clock::now() + ndn::time::seconds(3600),
                         buptNames);

  lsdb.installNameLsa(buptNewNameLsa);

  this->advanceClocks(ndn::time::seconds(1));

  // Install new adjacency LSA
  AdjLsa buptNewAdjLsa(buptRouterName, 12,
                       ndn::time::system_clock::now() + ndn::time::seconds(3600),
                       0, buptAdjacencies);
  lsdb.installAdjLsa(buptNewAdjLsa);

  this->advanceClocks(ndn::time::seconds(1));

  // Each NPT entry should have a destination router
  it = npt.begin();
  BOOST_REQUIRE_EQUAL(it->getNamePrefix(), buptRouterName);
  BOOST_REQUIRE_EQUAL(it->getRteList().size(), 1);
  BOOST_CHECK_EQUAL(it->getRteList().begin()->getDestination(), buptRouterName);

  ++it;
  BOOST_REQUIRE_EQUAL(it->getNamePrefix(), buptAdvertisedName);
  BOOST_REQUIRE_EQUAL(it->getRteList().size(), 1);
  BOOST_CHECK_EQUAL(it->getRteList().begin()->getDestination(), buptRouterName);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
