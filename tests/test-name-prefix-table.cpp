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

#include "route/name-prefix-table.hpp"
#include "nlsr.hpp"
#include "test-common.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

class NamePrefixTableFixture : public UnitTestTimeFixture
{
public:
  NamePrefixTableFixture()
    : face(m_ioService, m_keyChain)
    , nlsr(m_ioService, m_scheduler, face, m_keyChain)
    , lsdb(nlsr.getLsdb())
    , npt(nlsr.getNamePrefixTable())
  {
    INIT_LOGGERS("/tmp", "DEBUG");
  }

public:
  ndn::util::DummyClientFace face;
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

  Adjacent thisRouter(conf.getRouterPrefix(), ndn::FaceUri("udp4://10.0.0.1"), 0, Adjacent::STATUS_ACTIVE, 0, 0);

  ndn::Name buptRouterName("/ndn/cn/edu/bupt/%C1.Router/bupthub");
  Adjacent bupt(buptRouterName, ndn::FaceUri("udp4://10.0.0.2"), 0, Adjacent::STATUS_ACTIVE, 0, 0);

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

  NamePrefixList buptNames{buptAdvertisedName};

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
  NamePrefixTable& npt = nlsr.getNamePrefixTable();
  RoutingTablePoolEntry rtpe1("router1");

  npt.addRtpeToPool(rtpe1);

  BOOST_CHECK_EQUAL(npt.m_rtpool.size(), 1);
  BOOST_CHECK_EQUAL(*(npt.m_rtpool.find("router1")->second), rtpe1);
}

BOOST_FIXTURE_TEST_CASE(RemoveEntryFromPool, NamePrefixTableFixture)
{
  NamePrefixTable& npt = nlsr.getNamePrefixTable();
  RoutingTablePoolEntry rtpe1("router1", 0);
  std::shared_ptr<RoutingTablePoolEntry> rtpePtr = npt.addRtpeToPool(rtpe1);

  npt.addRtpeToPool(rtpe1);

  npt.deleteRtpeFromPool(rtpePtr);

  BOOST_CHECK_EQUAL(npt.m_rtpool.size(), 0);
  BOOST_CHECK_EQUAL(npt.m_rtpool.count("router1"), 0);
}

BOOST_FIXTURE_TEST_CASE(AddRoutingEntryToNptEntry, NamePrefixTableFixture)
{
  NamePrefixTable& npt = nlsr.getNamePrefixTable();
  RoutingTablePoolEntry rtpe1("/ndn/memphis/rtr1", 0);
  std::shared_ptr<RoutingTablePoolEntry> rtpePtr = npt.addRtpeToPool(rtpe1);
  NamePrefixTableEntry npte1("/ndn/memphis/rtr2");

  npt.addEntry("/ndn/memphis/rtr2", "/ndn/memphis/rtr1");

  NamePrefixTable::NptEntryList::iterator nItr =
    std::find_if(npt.m_table.begin(),
                 npt.m_table.end(),
                 [&] (const std::shared_ptr<NamePrefixTableEntry>& entry) {
                   return entry->getNamePrefix() == npte1.getNamePrefix();
                 });

  std::list<std::shared_ptr<RoutingTablePoolEntry>> rtpeList = (*nItr)->getRteList();
  std::list<std::shared_ptr<RoutingTablePoolEntry>>::iterator rItr =
    std::find(rtpeList.begin(),
              rtpeList.end(),
              rtpePtr);
  BOOST_CHECK_EQUAL(**rItr, *rtpePtr);
}

BOOST_FIXTURE_TEST_CASE(RemoveRoutingEntryFromNptEntry, NamePrefixTableFixture)
{
  NamePrefixTable& npt = nlsr.getNamePrefixTable();
  RoutingTablePoolEntry rtpe1("/ndn/memphis/rtr1", 0);

  NamePrefixTableEntry npte1("/ndn/memphis/rtr2");
  npt.m_table.push_back(make_shared<NamePrefixTableEntry>(npte1));

  npt.addEntry("/ndn/memphis/rtr2", "/ndn/memphis/rtr1");
  npt.addEntry("/ndn/memphis/rtr2", "/ndn/memphis/altrtr");

  npt.removeEntry("/ndn/memphis/rtr2", "/ndn/memphis/rtr1");

  NamePrefixTable::NptEntryList::iterator nItr =
    std::find_if(npt.m_table.begin(),
                 npt.m_table.end(),
                 [&] (const std::shared_ptr<NamePrefixTableEntry>& entry) {
                   return entry->getNamePrefix() == npte1.getNamePrefix();
                 });

  std::list<std::shared_ptr<RoutingTablePoolEntry>> rtpeList = (*nItr)->getRteList();

  BOOST_CHECK_EQUAL(rtpeList.size(), 1);
  BOOST_CHECK_EQUAL(npt.m_rtpool.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(AddNptEntryPtrToRoutingEntry, NamePrefixTableFixture)
{
  NamePrefixTable& npt = nlsr.getNamePrefixTable();
  NamePrefixTableEntry npte1("/ndn/memphis/rtr2");
  npt.m_table.push_back(make_shared<NamePrefixTableEntry>(npte1));

  npt.addEntry("/ndn/memphis/rtr2", "/ndn/memphis/rtr1");

  NamePrefixTable::NptEntryList::iterator nItr =
    std::find_if(npt.m_table.begin(),
                 npt.m_table.end(),
                 [&] (const std::shared_ptr<NamePrefixTableEntry>& entry) {
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
  NamePrefixTable& npt = nlsr.getNamePrefixTable();
  NamePrefixTableEntry npte1("/ndn/memphis/rtr1");
  NamePrefixTableEntry npte2("/ndn/memphis/rtr2");
  RoutingTableEntry rte1("/ndn/memphis/destination1");
  npt.m_table.push_back(make_shared<NamePrefixTableEntry>(npte1));
  npt.m_table.push_back(make_shared<NamePrefixTableEntry>(npte2));

  npt.addEntry(npte1.getNamePrefix(), rte1.getDestination());
  // We have to add two entries, otherwise the routing pool entry will be deleted.
  npt.addEntry(npte2.getNamePrefix(), rte1.getDestination());
  npt.removeEntry(npte2.getNamePrefix(), rte1.getDestination());

  NamePrefixTable::NptEntryList::iterator nItr =
    std::find_if(npt.m_table.begin(),
                 npt.m_table.end(),
                 [&] (const std::shared_ptr<NamePrefixTableEntry>& entry) {
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
  NamePrefixTable& namePrefixTable = nlsr.getNamePrefixTable();
  RoutingTable& routingTable = nlsr.getRoutingTable();
  const ndn::Name destination = ndn::Name{"/ndn/destination1"};
  NextHop hop1{"upd4://10.0.0.1", 0};
  NextHop hop2{"udp4://10.0.0.2", 1};
  NextHop hop3{"udp4://10.0.0.3", 2};
  const NamePrefixTableEntry entry1{"/ndn/router1"};
  namePrefixTable.addEntry(entry1.getNamePrefix(), destination);

  routingTable.addNextHop(destination, hop1);
  routingTable.addNextHop(destination, hop2);

  namePrefixTable.updateWithNewRoute(routingTable.m_rTable);

  // At this point the NamePrefixTableEntry should have two NextHops.
  auto nameIterator = std::find_if(namePrefixTable.begin(), namePrefixTable.end(),
                                   [&] (const std::shared_ptr<NamePrefixTableEntry>& entry) {
                                     return entry1.getNamePrefix() == entry->getNamePrefix();
                                   });
  BOOST_REQUIRE(nameIterator != namePrefixTable.end());

  auto iterator = namePrefixTable.m_rtpool.find(destination);
  BOOST_REQUIRE(iterator != namePrefixTable.m_rtpool.end());
  auto nextHops = (iterator->second)->getNexthopList();
  BOOST_CHECK_EQUAL(nextHops.size(), 2);

  // Add the other NextHop
  routingTable.addNextHop(destination, hop3);
  namePrefixTable.updateWithNewRoute(routingTable.m_rTable);

  // At this point the NamePrefixTableEntry should have three NextHops.
  nameIterator = std::find_if(namePrefixTable.begin(), namePrefixTable.end(),
                              [&] (const std::shared_ptr<NamePrefixTableEntry>& entry) {
                                return entry1.getNamePrefix() == entry->getNamePrefix();
                              });
  BOOST_REQUIRE(nameIterator != namePrefixTable.end());
  iterator = namePrefixTable.m_rtpool.find(destination);
  BOOST_REQUIRE(iterator != namePrefixTable.m_rtpool.end());
  nextHops = (iterator->second)->getNexthopList();
  BOOST_CHECK_EQUAL(nextHops.size(), 3);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
