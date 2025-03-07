/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
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

#include "route/name-prefix-table.hpp"
#include "name-prefix-list.hpp"
#include "route/fib.hpp"
#include "route/routing-table.hpp"
#include "lsdb.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

namespace nlsr::tests {

class NamePrefixTableFixture : public IoKeyChainFixture
{
public:
  NamePrefixTableFixture()
    : lsdb(face, m_keyChain, conf)
    , fib(face, m_scheduler, conf.getAdjacencyList(), conf, m_keyChain)
    , rt(m_scheduler, lsdb, conf)
    , npt(conf.getRouterPrefix(), fib, rt, rt.afterRoutingChange, lsdb.onLsdbModified)
  {
  }

  bool
  isNameInNpt(const ndn::Name& name)
  {
    auto it = std::find_if(npt.begin(), npt.end(),
                           [&] (const auto& entry) { return name == entry->getNamePrefix(); });
    return it != npt.end();
  }

private:
  ndn::Scheduler m_scheduler{m_io};

public:
  ndn::DummyClientFace face{m_io, m_keyChain};
  ConfParameter conf{face, m_keyChain};
  DummyConfFileProcessor confProcessor{conf};

  Lsdb lsdb;
  Fib fib;
  RoutingTable rt;
  NamePrefixTable npt;
};

BOOST_AUTO_TEST_SUITE(TestNamePrefixTable)

BOOST_FIXTURE_TEST_CASE(Bupt, NamePrefixTableFixture)
{
  rt.m_routingCalcInterval = 0_s;

  Adjacent thisRouter(conf.getRouterPrefix(), ndn::FaceUri("udp4://10.0.0.1"), 0, Adjacent::STATUS_ACTIVE, 0, 0);

  ndn::Name buptRouterName("/ndn/cn/edu/bupt/%C1.Router/bupthub");
  Adjacent bupt(buptRouterName, ndn::FaceUri("udp4://10.0.0.2"), 0, Adjacent::STATUS_ACTIVE, 0, 0);

  // This router's Adjacency LSA
  conf.getAdjacencyList().insert(bupt);
  AdjLsa thisRouterAdjLsa(thisRouter.getName(), 1, time::system_clock::now() + 3600_s, conf.getAdjacencyList());
  lsdb.installLsa(std::make_shared<AdjLsa>(thisRouterAdjLsa));

  // BUPT Adjacency LSA
  AdjacencyList buptAdjacencies;
  buptAdjacencies.insert(thisRouter);
  AdjLsa buptAdjLsa(buptRouterName, 1, time::system_clock::now() + 5_s, buptAdjacencies);
  lsdb.installLsa(std::make_shared<AdjLsa>(buptAdjLsa));

  // BUPT Name LSA
  ndn::Name buptAdvertisedName("/ndn/cn/edu/bupt");
  NamePrefixList buptNames{buptAdvertisedName};
  NameLsa buptNameLsa(buptRouterName, 1, time::system_clock::now() + 5_s, buptNames);
  lsdb.installLsa(std::make_shared<NameLsa>(buptNameLsa));

  // Advance clocks to expire LSAs
  this->advanceClocks(15_s);

  // LSA expirations should cause NPT entries to be completely removed
  auto it = npt.begin();
  BOOST_CHECK(it == npt.end());

  // Install new name LSA
  NameLsa buptNewNameLsa(buptRouterName, 12, time::system_clock::now() + 3600_s, buptNames);
  lsdb.installLsa(std::make_shared<NameLsa>(buptNewNameLsa));

  this->advanceClocks(1_s);

  // Install new adjacency LSA
  AdjLsa buptNewAdjLsa(buptRouterName, 12, time::system_clock::now() + 3600_s, buptAdjacencies);
  lsdb.installLsa(std::make_shared<AdjLsa>(buptNewAdjLsa));

  this->advanceClocks(1_s);

  // Each NPT entry should have a destination router
  it = npt.begin();
  BOOST_REQUIRE_EQUAL((*it)->getNamePrefix(), buptRouterName);
  BOOST_REQUIRE_EQUAL((*it)->getRteList().size(), 1);
  BOOST_CHECK_EQUAL((*(*it)->getRteList().begin())->getDestination(), buptRouterName);

  ++it;
  BOOST_REQUIRE_EQUAL((*it)->getNamePrefix(), buptAdvertisedName);
  BOOST_REQUIRE_EQUAL((*it)->getRteList().size(), 1);
  BOOST_CHECK_EQUAL((*(*it)->getRteList().begin())->getDestination(), buptRouterName);
}

BOOST_FIXTURE_TEST_CASE(AddEntryToPool, NamePrefixTableFixture)
{
  RoutingTablePoolEntry rtpe1("router1");

  npt.addRtpeToPool(rtpe1);

  BOOST_CHECK_EQUAL(npt.m_rtpool.size(), 1);
  BOOST_CHECK_EQUAL(*(npt.m_rtpool.find("router1")->second), rtpe1);
}

BOOST_FIXTURE_TEST_CASE(RemoveEntryFromPool, NamePrefixTableFixture)
{
  RoutingTablePoolEntry rtpe1("router1", 0);
  std::shared_ptr<RoutingTablePoolEntry> rtpePtr = npt.addRtpeToPool(rtpe1);

  npt.addRtpeToPool(rtpe1);

  npt.deleteRtpeFromPool(rtpePtr);

  BOOST_CHECK_EQUAL(npt.m_rtpool.size(), 0);
  BOOST_CHECK_EQUAL(npt.m_rtpool.count("router1"), 0);
}

BOOST_FIXTURE_TEST_CASE(AddRoutingEntryToNptEntry, NamePrefixTableFixture)
{
  RoutingTablePoolEntry rtpe1("/ndn/memphis/rtr1", 0);
  std::shared_ptr<RoutingTablePoolEntry> rtpePtr = npt.addRtpeToPool(rtpe1);
  NamePrefixTableEntry npte1("/ndn/memphis/rtr2");

  npt.addEntry("/ndn/memphis/rtr2", "/ndn/memphis/rtr1");

  auto nItr = std::find_if(npt.m_table.begin(),
                           npt.m_table.end(),
                           [&] (const auto& entry) {
                             return entry->getNamePrefix() == npte1.getNamePrefix();
                           });

  std::list<std::shared_ptr<RoutingTablePoolEntry>> rtpeList = (*nItr)->getRteList();
  auto rItr = std::find(rtpeList.begin(), rtpeList.end(), rtpePtr);
  BOOST_CHECK_EQUAL(**rItr, *rtpePtr);
}

BOOST_FIXTURE_TEST_CASE(RemoveRoutingEntryFromNptEntry, NamePrefixTableFixture)
{
  RoutingTablePoolEntry rtpe1("/ndn/memphis/rtr1", 0);

  NamePrefixTableEntry npte1("/ndn/memphis/rtr2");
  npt.m_table.push_back(std::make_shared<NamePrefixTableEntry>(npte1));

  npt.addEntry("/ndn/memphis/rtr2", "/ndn/memphis/rtr1");
  npt.addEntry("/ndn/memphis/rtr2", "/ndn/memphis/altrtr");

  npt.removeEntry("/ndn/memphis/rtr2", "/ndn/memphis/rtr1");

  auto nItr = std::find_if(npt.m_table.begin(),
                           npt.m_table.end(),
                           [&] (const auto& entry) {
                             return entry->getNamePrefix() == npte1.getNamePrefix();
                           });

  std::list<std::shared_ptr<RoutingTablePoolEntry>> rtpeList = (*nItr)->getRteList();

  BOOST_CHECK_EQUAL(rtpeList.size(), 1);
  BOOST_CHECK_EQUAL(npt.m_rtpool.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(AddNptEntryPtrToRoutingEntry, NamePrefixTableFixture)
{
  NamePrefixTableEntry npte1("/ndn/memphis/rtr2");
  npt.m_table.push_back(std::make_shared<NamePrefixTableEntry>(npte1));

  npt.addEntry("/ndn/memphis/rtr2", "/ndn/memphis/rtr1");

  auto nItr = std::find_if(npt.m_table.begin(),
                           npt.m_table.end(),
                           [&] (const auto& entry) {
                             return entry->getNamePrefix() == npte1.getNamePrefix();
                           });

  std::list<std::shared_ptr<RoutingTablePoolEntry>> rtpeList = (*nItr)->getRteList();

  BOOST_CHECK_EQUAL(rtpeList.size(), 1);

  auto& namePrefixPtrs = rtpeList.front()->namePrefixTableEntries;

  auto nptIterator = namePrefixPtrs.find(npte1.getNamePrefix());
  BOOST_REQUIRE(nptIterator != namePrefixPtrs.end());
  auto nptSharedPtr = nptIterator->second.lock();
  BOOST_CHECK_EQUAL(*nptSharedPtr, npte1);
}

BOOST_FIXTURE_TEST_CASE(RemoveNptEntryPtrFromRoutingEntry, NamePrefixTableFixture)
{
  NamePrefixTableEntry npte1("/ndn/memphis/rtr1");
  NamePrefixTableEntry npte2("/ndn/memphis/rtr2");
  RoutingTableEntry rte1("/ndn/memphis/destination1");
  npt.m_table.push_back(std::make_shared<NamePrefixTableEntry>(npte1));
  npt.m_table.push_back(std::make_shared<NamePrefixTableEntry>(npte2));

  npt.addEntry(npte1.getNamePrefix(), rte1.getDestination());
  // We have to add two entries, otherwise the routing pool entry will be deleted.
  npt.addEntry(npte2.getNamePrefix(), rte1.getDestination());
  npt.removeEntry(npte2.getNamePrefix(), rte1.getDestination());

  auto nItr = std::find_if(npt.m_table.begin(),
                           npt.m_table.end(),
                           [&] (const auto& entry) {
                             return entry->getNamePrefix() == npte1.getNamePrefix();
                           });

  std::list<std::shared_ptr<RoutingTablePoolEntry>> rtpeList = (*nItr)->getRteList();

  BOOST_CHECK_EQUAL(rtpeList.size(), 1);

  auto& namePrefixPtrs = rtpeList.front()->namePrefixTableEntries;

  // We should have removed the second one
  BOOST_CHECK_EQUAL(namePrefixPtrs.size(), 1);

  auto nptIterator = namePrefixPtrs.find(npte1.getNamePrefix());

  BOOST_REQUIRE(nptIterator != namePrefixPtrs.end());
  auto nptSharedPtr = nptIterator->second.lock();
  BOOST_CHECK_EQUAL(*nptSharedPtr, npte1);
}

BOOST_FIXTURE_TEST_CASE(RoutingTableUpdate, NamePrefixTableFixture)
{
  const ndn::Name destination = ndn::Name{"/ndn/destination1"};
  NextHop hop1{ndn::FaceUri("upd4://10.0.0.1"), 0};
  NextHop hop2{ndn::FaceUri("upd4://10.0.0.2"), 1};
  NextHop hop3{ndn::FaceUri("upd4://10.0.0.3"), 2};
  const NamePrefixTableEntry entry1{"/ndn/router1"};
  npt.addEntry(entry1.getNamePrefix(), destination);

  rt.addNextHop(destination, hop1);
  rt.addNextHop(destination, hop2);

  npt.updateWithNewRoute(rt.m_rTable);

  // At this point the NamePrefixTableEntry should have two NextHops.
  auto nameIterator = std::find_if(npt.begin(), npt.end(),
                                   [&] (const std::shared_ptr<NamePrefixTableEntry>& entry) {
                                     return entry1.getNamePrefix() == entry->getNamePrefix();
                                   });
  BOOST_REQUIRE(nameIterator != npt.end());

  auto iterator = npt.m_rtpool.find(destination);
  BOOST_REQUIRE(iterator != npt.m_rtpool.end());
  auto nextHops = (iterator->second)->getNexthopList();
  BOOST_CHECK_EQUAL(nextHops.size(), 2);

  // Add the other NextHop
  rt.addNextHop(destination, hop3);
  npt.updateWithNewRoute(rt.m_rTable);

  // At this point the NamePrefixTableEntry should have three NextHops.
  nameIterator = std::find_if(npt.begin(), npt.end(),
                              [&] (const std::shared_ptr<NamePrefixTableEntry>& entry) {
                                return entry1.getNamePrefix() == entry->getNamePrefix();
                              });
  BOOST_REQUIRE(nameIterator != npt.end());
  iterator = npt.m_rtpool.find(destination);
  BOOST_REQUIRE(iterator != npt.m_rtpool.end());
  nextHops = (iterator->second)->getNexthopList();
  BOOST_CHECK_EQUAL(nextHops.size(), 3);
}

BOOST_FIXTURE_TEST_CASE(UpdateFromLsdb, NamePrefixTableFixture)
{
  auto testTimePoint = time::system_clock::now();
  NamePrefixList npl1;
  ndn::Name n1("name1");
  ndn::Name n2("name2");
  PrefixInfo p1 = PrefixInfo(n1, 0);
  PrefixInfo p2 = PrefixInfo(n2, 0);
  ndn::Name router1("/router1/1");

  npl1.insert(p1);
  npl1.insert(p2);

  NameLsa nlsa1(router1, 12, testTimePoint, npl1);
  std::shared_ptr<Lsa> lsaPtr = std::make_shared<NameLsa>(nlsa1);

  BOOST_CHECK(npt.begin() == npt.end());
  npt.updateFromLsdb(lsaPtr, LsdbUpdate::INSTALLED, {}, {});
  BOOST_CHECK_EQUAL(npt.m_table.size(), 3); // Router + 2 names

  BOOST_CHECK(isNameInNpt(n1));
  BOOST_CHECK(isNameInNpt(n2));

  ndn::Name n3("name3");
  PrefixInfo p3 = PrefixInfo(n3, 0);
  auto nlsa = std::static_pointer_cast<NameLsa>(lsaPtr);
  nlsa->removeName(p2);
  nlsa->addName(p3);
  npt.updateFromLsdb(lsaPtr, LsdbUpdate::UPDATED, {p3}, {p2});
  BOOST_CHECK(isNameInNpt(n1));
  BOOST_CHECK(!isNameInNpt(n2)); // Removed
  BOOST_CHECK(isNameInNpt(n3));

  BOOST_CHECK_EQUAL(npt.m_table.size(), 3); // Still router + 2 names

  npt.updateFromLsdb(lsaPtr, LsdbUpdate::REMOVED, {}, {});
  BOOST_CHECK_EQUAL(npt.m_table.size(), 0);

  // Adj and Coordinate LSAs router
  ndn::Name router2("/router2/2");
  AdjLsa adjLsa(router2, 12, testTimePoint, conf.getAdjacencyList());
  lsaPtr = std::make_shared<AdjLsa>(adjLsa);
  BOOST_CHECK(npt.begin() == npt.end());
  npt.updateFromLsdb(lsaPtr, LsdbUpdate::INSTALLED, {}, {});
  BOOST_CHECK_EQUAL(npt.m_table.size(), 1);
  BOOST_CHECK(isNameInNpt(router2));

  npt.updateFromLsdb(lsaPtr, LsdbUpdate::REMOVED, {}, {});
  BOOST_CHECK_EQUAL(npt.m_table.size(), 0);

  ndn::Name router3("/router3/3");
  CoordinateLsa corLsa(router3, 12, testTimePoint, 2, {3});
  lsaPtr = std::make_shared<CoordinateLsa>(corLsa);
  BOOST_CHECK(npt.begin() == npt.end());
  npt.updateFromLsdb(lsaPtr, LsdbUpdate::INSTALLED, {}, {});
  BOOST_CHECK_EQUAL(npt.m_table.size(), 1);
  BOOST_CHECK(isNameInNpt(router3));

  npt.updateFromLsdb(lsaPtr, LsdbUpdate::REMOVED, {}, {});
  BOOST_CHECK_EQUAL(npt.m_table.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
