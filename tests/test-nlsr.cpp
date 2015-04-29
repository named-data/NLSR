/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
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
#include "control-commands.hpp"
#include "dummy-face.hpp"

#include "nlsr.hpp"

#include <ndn-cxx/management/nfd-face-event-notification.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

using ndn::DummyFace;
using ndn::shared_ptr;

BOOST_FIXTURE_TEST_SUITE(TestNlsr, BaseFixture)

BOOST_AUTO_TEST_CASE(HyperbolicOn_ZeroCostNeighbors)
{
  shared_ptr<DummyFace> face = ndn::makeDummyFace();
  Nlsr nlsr(g_ioService, g_scheduler, ndn::ref(*face));

  // Simulate loading configuration file
  AdjacencyList& neighbors = nlsr.getAdjacencyList();

  Adjacent neighborA("/ndn/neighborA", "uri://faceA", 25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", "uri://faceB", 10, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  Adjacent neighborC("/ndn/neighborC", "uri://faceC", 17, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborC);

  nlsr.getConfParameter().setHyperbolicState(HYPERBOLIC_STATE_ON);

  nlsr.initialize();

  std::list<Adjacent> neighborList = neighbors.getAdjList();
  for (std::list<Adjacent>::iterator it = neighborList.begin(); it != neighborList.end(); ++it) {
    BOOST_CHECK_EQUAL(it->getLinkCost(), 0);
  }
}

BOOST_AUTO_TEST_CASE(HyperbolicOff_LinkStateCost)
{
  shared_ptr<DummyFace> face = ndn::makeDummyFace();
  Nlsr nlsr(g_ioService, g_scheduler, ndn::ref(*face));

  // Simulate loading configuration file
  AdjacencyList& neighbors = nlsr.getAdjacencyList();

  Adjacent neighborA("/ndn/neighborA", "uri://faceA", 25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", "uri://faceB", 10, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  Adjacent neighborC("/ndn/neighborC", "uri://faceC", 17, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborC);

  nlsr.initialize();

  std::list<Adjacent> neighborList = neighbors.getAdjList();
  for (std::list<Adjacent>::iterator it = neighborList.begin(); it != neighborList.end(); ++it) {
    BOOST_CHECK(it->getLinkCost() != 0);
  }
}

BOOST_AUTO_TEST_CASE(SetEventIntervals)
{
  shared_ptr<DummyFace> face = ndn::makeDummyFace();
  Nlsr nlsr(g_ioService, g_scheduler, ndn::ref(*face));

  // Simulate loading configuration file
  ConfParameter& conf = nlsr.getConfParameter();
  conf.setAdjLsaBuildInterval(3);
  conf.setFirstHelloInterval(6);
  conf.setRoutingCalcInterval(9);

  nlsr.initialize();

  const Lsdb& lsdb = nlsr.getLsdb();
  const RoutingTable& rt = nlsr.getRoutingTable();

  BOOST_CHECK_EQUAL(lsdb.getAdjLsaBuildInterval(), ndn::time::seconds(3));
  BOOST_CHECK_EQUAL(nlsr.getFirstHelloInterval(), 6);
  BOOST_CHECK_EQUAL(rt.getRoutingCalcInterval(), ndn::time::seconds(9));
}

BOOST_FIXTURE_TEST_CASE(FaceDestroyEvent, UnitTestTimeFixture)
{
  shared_ptr<ndn::util::DummyClientFace> face = ndn::util::makeDummyClientFace(g_ioService);
  Nlsr nlsr(g_ioService, g_scheduler, ndn::ref(*face));

  // Simulate loading configuration file
  ConfParameter& conf = nlsr.getConfParameter();
  conf.setNetwork("/ndn");
  conf.setSiteName("/site");
  conf.setRouterName("/%C1.router/this-router");
  conf.setAdjLsaBuildInterval(0);
  conf.setRoutingCalcInterval(0);

  // Add active neighbors
  AdjacencyList& neighbors = nlsr.getAdjacencyList();

  uint64_t destroyFaceId = 128;

  // Create a neighbor whose Face will be destroyed
  Adjacent failNeighbor("/ndn/neighborA", "uri://faceA", 10, Adjacent::STATUS_ACTIVE, 0,
                        destroyFaceId);
  neighbors.insert(failNeighbor);

  // Create an additional neighbor so an adjacency LSA can be built after the face is destroyed
  Adjacent otherNeighbor("/ndn/neighborB", "uri://faceB", 25, Adjacent::STATUS_ACTIVE, 0, 256);
  neighbors.insert(otherNeighbor);

  nlsr.initialize();

  // Simulate successful HELLO responses
  nlsr.getLsdb().scheduleAdjLsaBuild();

  // Run the scheduler to build an adjacency LSA
  this->advanceClocks(ndn::time::milliseconds(1));

  // Make sure an adjacency LSA was built
  ndn::Name key = ndn::Name(nlsr.getConfParameter().getRouterPrefix()).append(AdjLsa::TYPE_STRING);
  AdjLsa* lsa = nlsr.getLsdb().findAdjLsa(key);
  BOOST_REQUIRE(lsa != nullptr);

  uint32_t lastAdjLsaSeqNo = nlsr.getSequencingManager().getAdjLsaSeq();

  // Make sure the routing table was calculated
  RoutingTableEntry* rtEntry = nlsr.getRoutingTable().findRoutingTableEntry(failNeighbor.getName());
  BOOST_REQUIRE(rtEntry != nullptr);
  BOOST_REQUIRE_EQUAL(rtEntry->getNexthopList().getSize(), 1);

  // Receive FaceEventDestroyed notification
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_DESTROYED)
       .setFaceId(destroyFaceId);

  shared_ptr<ndn::Data> data = make_shared<ndn::Data>("/localhost/nfd/faces/events/%FE%00");
  data->setContent(event.wireEncode());
  nlsr.getKeyChain().sign(*data);

  face->receive(*data);

  // Run the scheduler to build an adjacency LSA
  this->advanceClocks(ndn::time::milliseconds(1));

  Adjacent updatedNeighbor = neighbors.getAdjacent(failNeighbor.getName());

  BOOST_CHECK_EQUAL(updatedNeighbor.getFaceId(), 0);
  BOOST_CHECK_EQUAL(updatedNeighbor.getInterestTimedOutNo(),
                    nlsr.getConfParameter().getInterestRetryNumber());
  BOOST_CHECK_EQUAL(updatedNeighbor.getStatus(), Adjacent::STATUS_INACTIVE);
  BOOST_CHECK_EQUAL(nlsr.getSequencingManager().getAdjLsaSeq(), lastAdjLsaSeqNo + 1);

  // Make sure the routing table was recalculated
  rtEntry = nlsr.getRoutingTable().findRoutingTableEntry(failNeighbor.getName());
  BOOST_CHECK(rtEntry == nullptr);
}

// Bug #2733
// This test checks that when a face for an inactive node is destroyed, an
// Adjacency LSA build does not postpone the LSA refresh and cause RIB
// entries for other nodes' name prefixes to not be refreshed.
//
// This test is invalid when Issue #2732 is implemented since an Adjacency LSA
// refresh will not cause RIB entries for other nodes' name prefixes to be refreshed.
BOOST_FIXTURE_TEST_CASE(FaceDestroyEventInactive, UnitTestTimeFixture)
{
  shared_ptr<ndn::util::DummyClientFace> face = ndn::util::makeDummyClientFace(g_ioService);
  Nlsr nlsr(g_ioService, g_scheduler, ndn::ref(*face));
  Lsdb& lsdb = nlsr.getLsdb();

  // Simulate loading configuration file
  ConfParameter& conf = nlsr.getConfParameter();
  conf.setNetwork("/ndn");
  conf.setSiteName("/site");
  conf.setRouterName("/%C1.router/this-router");
  conf.setFirstHelloInterval(0);
  conf.setAdjLsaBuildInterval(0);
  conf.setRoutingCalcInterval(0);

  // Add neighbors
  AdjacencyList& neighbors = nlsr.getAdjacencyList();

  uint64_t destroyFaceId = 128;

  // Create an inactive neighbor whose Face will be destroyed
  Adjacent failNeighbor("/ndn/neighborA", "uri://faceA", 10, Adjacent::STATUS_INACTIVE, 3,
                        destroyFaceId);
  neighbors.insert(failNeighbor);

  // Create an additional active neighbor so an adjacency LSA can be built
  Adjacent otherNeighbor("/ndn/neighborB", "uri://faceB", 25, Adjacent::STATUS_ACTIVE, 0, 256);
  neighbors.insert(otherNeighbor);

  // Add a name for the neighbor to advertise
  NamePrefixList nameList;
  ndn::Name nameToAdvertise("/ndn/neighborB/name");
  nameList.insert(nameToAdvertise);

  NameLsa nameLsa("/ndn/neighborB", "name", 25, ndn::time::system_clock::now(), nameList);
  lsdb.installNameLsa(nameLsa);

  nlsr.initialize();

  // Simulate successful HELLO responses from neighbor B
  lsdb.scheduleAdjLsaBuild();

  // Run the scheduler to build an adjacency LSA
  this->advanceClocks(ndn::time::milliseconds(1));

  ndn::Name key = ndn::Name(nlsr.getConfParameter().getRouterPrefix()).append(AdjLsa::TYPE_STRING);
  AdjLsa* lsa = lsdb.findAdjLsa(key);
  BOOST_REQUIRE(lsa != nullptr);

  // Cancel previous LSA expiration event
  g_scheduler.cancelEvent(lsa->getExpiringEventId());

  // Set expiration time for own Adjacency LSA to earlier value for unit-test
  //
  // Expiration time is negative to offset the GRACE_PERIOD (10 seconds) automatically applied
  // to expiration times
  ndn::EventId id = lsdb.scheduleAdjLsaExpiration(key, lsa->getLsSeqNo(), -ndn::time::seconds(9));
  lsa->setExpiringEventId(id);

  // Generate a FaceEventDestroyed notification
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_DESTROYED)
       .setFaceId(destroyFaceId);

  shared_ptr<ndn::Data> data = make_shared<ndn::Data>("/localhost/nfd/faces/events/%FE%00");
  data->setContent(event.wireEncode());
  nlsr.getKeyChain().sign(*data);

  // Receive the FaceEventDestroyed notification
  face->receive(*data);

  // Run the scheduler to expire the Adjacency LSA. The expiration should refresh the RIB
  // entries associated with Neighbor B's advertised prefix.
  face->sentInterests.clear();
  this->advanceClocks(ndn::time::seconds(1));

  // The Face should have two sent Interests: the face event and a RIB registration
  BOOST_REQUIRE(face->sentInterests.size() > 0);
  const ndn::Interest& interest = face->sentInterests.back();

  ndn::nfd::ControlParameters parameters;
  ndn::Name::Component verb;
  BOOST_REQUIRE_NO_THROW(extractRibCommandParameters(interest, verb, parameters));

  BOOST_CHECK_EQUAL(verb, ndn::Name::Component("register"));
  BOOST_CHECK_EQUAL(parameters.getName(), nameToAdvertise);
}

BOOST_FIXTURE_TEST_CASE(GetCertificate, UnitTestTimeFixture)
{
  shared_ptr<ndn::util::DummyClientFace> face = ndn::util::makeDummyClientFace(g_ioService);
  Nlsr nlsr(g_ioService, g_scheduler, ndn::ref(*face));

  // Create certificate
  ndn::Name identity("/TestNLSR/identity");
  identity.appendVersion();

  ndn::KeyChain keyChain;
  keyChain.createIdentity(identity);
  ndn::Name certName = keyChain.getDefaultCertificateNameForIdentity(identity);
  shared_ptr<ndn::IdentityCertificate> certificate = keyChain.getCertificate(certName);

  const ndn::Name certKey = certificate->getName().getPrefix(-1);

  BOOST_CHECK(nlsr.getCertificate(certKey) == nullptr);

  // Certificate should be retrievable from the CertificateStore
  nlsr.loadCertToPublish(certificate);

  BOOST_CHECK(nlsr.getCertificate(certKey) != nullptr);

  nlsr.getCertificateStore().clear();

  // Certificate should be retrievable from the cache
  nlsr.addCertificateToCache(certificate);
  this->advanceClocks(ndn::time::milliseconds(10));

  BOOST_CHECK(nlsr.getCertificate(certKey) != nullptr);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
