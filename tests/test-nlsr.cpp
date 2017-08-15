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

#include "nlsr.hpp"
#include "test-common.hpp"
#include "control-commands.hpp"
#include "logger.hpp"

#include <ndn-cxx/mgmt/nfd/face-event-notification.hpp>

namespace nlsr {
namespace test {

using std::shared_ptr;

class NlsrFixture : public MockNfdMgmtFixture
{
public:
  NlsrFixture()
    : nlsr(g_ioService, g_scheduler, std::ref(*face), g_keyChain)
    , lsdb(nlsr.getLsdb())
    , neighbors(nlsr.getAdjacencyList())
    , nSuccessCallbacks(0)
    , nFailureCallbacks(0)
  {
  }

  void
  receiveHelloData(const ndn::Name& sender, const ndn::Name& receiver)
  {
    ndn::Name dataName(sender);
    dataName.append("NLSR").append("INFO").append(receiver.wireEncode()).appendVersion();

    std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>(dataName);

    nlsr.m_helloProtocol.onContentValidated(data);
 }

public:
  Nlsr nlsr;
  Lsdb& lsdb;
  AdjacencyList& neighbors;
  uint32_t nSuccessCallbacks;
  uint32_t nFailureCallbacks;

};

BOOST_FIXTURE_TEST_SUITE(TestNlsr, NlsrFixture)

BOOST_AUTO_TEST_CASE(HyperbolicOn_ZeroCostNeighbors)
{
  // Simulate loading configuration file
  Adjacent neighborA("/ndn/neighborA", ndn::util::FaceUri("udp4://10.0.0.1"), 25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", ndn::util::FaceUri("udp4://10.0.0.2"), 10, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  Adjacent neighborC("/ndn/neighborC", ndn::util::FaceUri("udp4://10.0.0.3"), 17, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborC);

  nlsr.getConfParameter().setHyperbolicState(HYPERBOLIC_STATE_ON);
  nlsr.getConfParameter().setNetwork(ndn::Name("/test"));

  nlsr.initialize();

  std::list<Adjacent> neighborList = neighbors.getAdjList();
  for (std::list<Adjacent>::iterator it = neighborList.begin(); it != neighborList.end(); ++it) {
    BOOST_CHECK_EQUAL(it->getLinkCost(), 0);
  }
}

BOOST_AUTO_TEST_CASE(HyperbolicOff_LinkStateCost)
{
  // Simulate loading configuration file
  Adjacent neighborA("/ndn/neighborA", ndn::util::FaceUri("udp4://10.0.0.1"), 25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", ndn::util::FaceUri("udp4://10.0.0.2"), 10, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  Adjacent neighborC("/ndn/neighborC", ndn::util::FaceUri("udp4://10.0.0.3"), 17, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborC);

  nlsr.initialize();

  std::list<Adjacent> neighborList = neighbors.getAdjList();
  for (std::list<Adjacent>::iterator it = neighborList.begin(); it != neighborList.end(); ++it) {
    BOOST_CHECK(it->getLinkCost() != 0);
  }
}

BOOST_AUTO_TEST_CASE(SetEventIntervals)
{
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

BOOST_AUTO_TEST_CASE(FaceCreateEvent)
{
  // Setting constants for the unit test
  const uint32_t faceId = 1;
  const std::string faceUri = "udp4://10.0.0.1:6363";
  Adjacent neighbor("/ndn/neighborA", ndn::util::FaceUri(faceUri), 10, Adjacent::STATUS_INACTIVE, 0, 0);
  BOOST_REQUIRE_EQUAL(nlsr.getAdjacencyList().insert(neighbor), 0);

  this->advanceClocks(ndn::time::milliseconds(1));

  // Build, sign, and send the Face Event
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_CREATED)
    .setRemoteUri(faceUri)
    .setFaceId(faceId);
  std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>("/localhost/nfd/faces/events/%FE%00");
  data->setContent(event.wireEncode());
  nlsr.getKeyChain().sign(*data);
  face->receive(*data);

  // Move the clocks forward so that the Face processes the event.
  this->advanceClocks(ndn::time::milliseconds(1));

  // Need to explicitly provide a FaceUri object, because the
  // conversion will attempt to create Name objects.
  auto iterator = nlsr.getAdjacencyList().findAdjacent(ndn::util::FaceUri(faceUri));
  BOOST_REQUIRE(iterator != nlsr.getAdjacencyList().end());
  BOOST_CHECK_EQUAL(iterator->getFaceId(), faceId);
}

BOOST_AUTO_TEST_CASE(FaceCreateEventNoMatch)
{
  // Setting constants for the unit test
  const uint32_t faceId = 1;
  const std::string eventUri = "udp4://10.0.0.1:6363";
  const std::string neighborUri = "udp4://10.0.0.2:6363";
  Adjacent neighbor("/ndn/neighborA", ndn::util::FaceUri(neighborUri), 10, Adjacent::STATUS_INACTIVE, 0, 0);
  nlsr.getAdjacencyList().insert(neighbor);

  // Build, sign, and send the Face Event
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_CREATED)
    .setRemoteUri(eventUri)
    .setFaceId(faceId);
  std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>("/localhost/nfd/faces/events/%FE%00");
  data->setContent(event.wireEncode());
  nlsr.getKeyChain().sign(*data);
  face->receive(*data);

  // Move the clocks forward so that the Face processes the event.
  this->advanceClocks(ndn::time::milliseconds(1));

  // The Face URIs did not match, so this neighbor should be unconfigured.
  auto iterator = nlsr.getAdjacencyList().findAdjacent(ndn::util::FaceUri(neighborUri));
  BOOST_REQUIRE(iterator != nlsr.getAdjacencyList().end());
  BOOST_CHECK_EQUAL(iterator->getFaceId(), 0);
}

BOOST_AUTO_TEST_CASE(FaceCreateEventAlreadyConfigured)
{
  // Setting constants for the unit test
  const uint32_t eventFaceId = 1;
  const uint32_t neighborFaceId = 2;
  const std::string faceUri = "udp4://10.0.0.1:6363";
  Adjacent neighbor("/ndn/neighborA", ndn::util::FaceUri(faceUri), 10, Adjacent::STATUS_ACTIVE, 0, neighborFaceId);
  nlsr.getAdjacencyList().insert(neighbor);

  // Build, sign, and send the Face Event
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_CREATED)
    .setRemoteUri(faceUri)
    .setFaceId(eventFaceId);
  std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>("/localhost/nfd/faces/events/%FE%00");
  data->setContent(event.wireEncode());
  nlsr.getKeyChain().sign(*data);
  face->receive(*data);

  // Move the clocks forward so that the Face processes the event.
  this->advanceClocks(ndn::time::milliseconds(1));

  // Since the neighbor was already configured, this (simply erroneous) event should have no effect.
  auto iterator = nlsr.getAdjacencyList().findAdjacent(ndn::util::FaceUri(faceUri));
  BOOST_REQUIRE(iterator != nlsr.getAdjacencyList().end());
  BOOST_CHECK_EQUAL(iterator->getFaceId(), neighborFaceId);
}

BOOST_FIXTURE_TEST_CASE(FaceDestroyEvent, UnitTestTimeFixture)
{
  std::shared_ptr<ndn::util::DummyClientFace> face = std::make_shared<ndn::util::DummyClientFace>(g_ioService);
  Nlsr nlsr(g_ioService, g_scheduler, std::ref(*face), g_keyChain);
  Lsdb& lsdb = nlsr.getLsdb();

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
  Adjacent failNeighbor("/ndn/neighborA", ndn::util::FaceUri("udp4://10.0.0.1"), 10, Adjacent::STATUS_ACTIVE, 0,
                        destroyFaceId);
  neighbors.insert(failNeighbor);

  // Create an additional neighbor so an adjacency LSA can be built after the face is destroyed
  Adjacent otherNeighbor("/ndn/neighborB", ndn::util::FaceUri("udp4://10.0.0.2"), 10, Adjacent::STATUS_ACTIVE, 0, 256);
  neighbors.insert(otherNeighbor);

  nlsr.initialize();

  // Simulate successful HELLO responses
  lsdb.scheduleAdjLsaBuild();

  // Set up adjacency LSAs
  // This router
  Adjacent thisRouter(conf.getRouterPrefix(), ndn::util::FaceUri("udp4://10.0.0.3"), 10, Adjacent::STATUS_ACTIVE, 0, 256);

  AdjLsa ownAdjLsa(conf.getRouterPrefix(), 10, ndn::time::system_clock::now(), 1, neighbors);
  lsdb.installAdjLsa(ownAdjLsa);

  // Router that will fail
  AdjacencyList failAdjacencies;
  failAdjacencies.insert(thisRouter);

  AdjLsa failAdjLsa("/ndn/neighborA", 10,
                     ndn::time::system_clock::now() + ndn::time::seconds(3600), 1, failAdjacencies);

  lsdb.installAdjLsa(failAdjLsa);

  // Other router
  AdjacencyList otherAdjacencies;
  otherAdjacencies.insert(thisRouter);

  AdjLsa otherAdjLsa("/ndn/neighborB", 10,
                     ndn::time::system_clock::now() + ndn::time::seconds(3600), 1, otherAdjacencies);

  lsdb.installAdjLsa(otherAdjLsa);

  // Run the scheduler to build an adjacency LSA
  this->advanceClocks(ndn::time::milliseconds(1));

  // Make sure an adjacency LSA was built
  ndn::Name key = ndn::Name(nlsr.getConfParameter().getRouterPrefix()).append(AdjLsa::TYPE_STRING);
  AdjLsa* lsa = lsdb.findAdjLsa(key);
  BOOST_REQUIRE(lsa != nullptr);

  uint32_t lastAdjLsaSeqNo = lsa->getLsSeqNo();
  nlsr.getLsdb().getSequencingManager().setAdjLsaSeq(lastAdjLsaSeqNo);

  // Make sure the routing table was calculated
  RoutingTableEntry* rtEntry = nlsr.getRoutingTable().findRoutingTableEntry(failNeighbor.getName());
  BOOST_REQUIRE(rtEntry != nullptr);
  BOOST_REQUIRE_EQUAL(rtEntry->getNexthopList().getSize(), 1);

  // Receive FaceEventDestroyed notification
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_DESTROYED)
       .setFaceId(destroyFaceId);

  std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>("/localhost/nfd/faces/events/%FE%00");
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

  lsa = lsdb.findAdjLsa(key);
  BOOST_REQUIRE(lsa != nullptr);

  BOOST_CHECK_EQUAL(lsa->getLsSeqNo(), lastAdjLsaSeqNo + 1);

  // Make sure the routing table was recalculated
  rtEntry = nlsr.getRoutingTable().findRoutingTableEntry(failNeighbor.getName());
  BOOST_CHECK(rtEntry == nullptr);
}

BOOST_AUTO_TEST_CASE(GetCertificate)
{
  // Create certificate
  ndn::Name identity("/TestNLSR/identity");
  identity.appendVersion();

  ndn::KeyChain keyChain;
  keyChain.createIdentity(identity);
  ndn::Name certName = keyChain.getDefaultCertificateNameForIdentity(identity);
  std::shared_ptr<ndn::IdentityCertificate> certificate = keyChain.getCertificate(certName);

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

BOOST_AUTO_TEST_CASE(SetRouterCommandPrefix)
{
  // Simulate loading configuration file
  ConfParameter& conf = nlsr.getConfParameter();
  conf.setNetwork("/ndn");
  conf.setSiteName("/site");
  conf.setRouterName("/%C1.router/this-router");

  nlsr.initialize();

  BOOST_CHECK_EQUAL(nlsr.getLsdbDatasetHandler().getRouterNameCommandPrefix(),
                    ndn::Name("/ndn/site/%C1.router/this-router/lsdb"));
}

BOOST_AUTO_TEST_CASE(BuildAdjLsaAfterHelloResponse)
{
  // Configure NLSR
  ConfParameter& conf = nlsr.getConfParameter();
  conf.setNetwork("/ndn");
  conf.setSiteName("/site");

  ndn::Name routerName("/%C1.Router/this-router");
  conf.setRouterName(routerName);

  conf.setAdjLsaBuildInterval(1);

  // Add neighbors
  // Router A
  ndn::Name neighborAName("/ndn/site/%C1.router/routerA");
  Adjacent neighborA(neighborAName, ndn::util::FaceUri("udp4://10.0.0.1"), 0, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  // Router B
  ndn::Name neighborBName("/ndn/site/%C1.router/routerB");
  Adjacent neighborB(neighborBName, ndn::util::FaceUri("udp4://10.0.0.1"), 0, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  nlsr.initialize();
  this->advanceClocks(ndn::time::milliseconds(1));

  // Receive HELLO response from Router A
  receiveHelloData(neighborAName, conf.getRouterPrefix());
  this->advanceClocks(ndn::time::seconds(1));

  ndn::Name lsaKey = ndn::Name(conf.getRouterPrefix()).append(AdjLsa::TYPE_STRING);

  // Adjacency LSA should be built even though other router is INACTIVE
  AdjLsa* lsa = lsdb.findAdjLsa(lsaKey);
  BOOST_REQUIRE(lsa != nullptr);
  BOOST_CHECK_EQUAL(lsa->getAdl().getSize(), 1);

  // Receive HELLO response from Router B
  receiveHelloData(neighborBName, conf.getRouterPrefix());

  // Both routers become INACTIVE and HELLO Interests have timed out
  for (Adjacent& adjacency : neighbors.getAdjList()) {
    adjacency.setStatus(Adjacent::STATUS_INACTIVE);
    adjacency.setInterestTimedOutNo(HELLO_RETRIES_DEFAULT);
  }

  this->advanceClocks(ndn::time::seconds(1));

  // Adjacency LSA should have been removed since this router's adjacencies are INACTIVE
  // and have timed out
  lsa = lsdb.findAdjLsa(lsaKey);
  BOOST_CHECK(lsa == nullptr);

  // Receive HELLO response from Router A and B
  receiveHelloData(neighborAName, conf.getRouterPrefix());
  receiveHelloData(neighborBName, conf.getRouterPrefix());
  this->advanceClocks(ndn::time::seconds(1));

  // Adjacency LSA should be built
  lsa = lsdb.findAdjLsa(lsaKey);
  BOOST_REQUIRE(lsa != nullptr);
  BOOST_CHECK_EQUAL(lsa->getAdl().getSize(), 2);
}

BOOST_AUTO_TEST_CASE(CanonizeUris)
{
  ndn::Name neighborAName("/ndn/site/%C1.router/routerA");
  ndn::util::FaceUri faceUriA("udp://10.0.0.1");
  Adjacent neighborA(neighborAName, faceUriA, 0, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  ndn::Name neighborBName("/ndn/site/%C1.router/routerB");
  ndn::util::FaceUri faceUriB("udp://10.0.0.2");
  Adjacent neighborB(neighborBName, faceUriB, 0, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  int nCanonizationsLeft = nlsr.getAdjacencyList().getAdjList().size();
  std::function<void(std::list<Adjacent>::iterator)> thenCallback =
    [this, &thenCallback, &nCanonizationsLeft] (std::list<Adjacent>::iterator iterator) {
      nCanonizationsLeft--;
      nlsr.canonizeNeighborUris(iterator, thenCallback);
  };
  nlsr.canonizeNeighborUris(nlsr.getAdjacencyList().getAdjList().begin(),
                            [thenCallback] (std::list<Adjacent>::iterator iterator) {
                              thenCallback(iterator);
                            });
  while (nCanonizationsLeft != 0) {
    this->advanceClocks(ndn::time::milliseconds(1));
  }

  BOOST_CHECK_EQUAL(nlsr.getAdjacencyList().getAdjacent(neighborAName).getFaceUri(),
                    ndn::util::FaceUri("udp4://10.0.0.1:6363"));

  BOOST_CHECK_EQUAL(nlsr.getAdjacencyList().getAdjacent(neighborBName).getFaceUri(),
                    ndn::util::FaceUri("udp4://10.0.0.2:6363"));
}

BOOST_AUTO_TEST_CASE(FaceDatasetFetchSuccess)
{
  bool hasResult = false;
  nlsr.m_validator.m_shouldValidate = false;

  nlsr.initializeFaces([&hasResult] (const std::vector<ndn::nfd::FaceStatus>& faces) {
      hasResult = true;
      BOOST_CHECK_EQUAL(faces.size(), 2);
      BOOST_CHECK_EQUAL(faces.front().getFaceId(), 25401);
      BOOST_CHECK_EQUAL(faces.back().getFaceId(), 25402);
    },
    [] (uint32_t code, const std::string& reason) {});

  this->advanceClocks(ndn::time::milliseconds(500));

  ndn::nfd::FaceStatus payload1;
  payload1.setFaceId(25401);
  ndn::nfd::FaceStatus payload2;
  payload2.setFaceId(25402);
  this->sendDataset("/localhost/nfd/faces/list", payload1, payload2);

  this->advanceClocks(ndn::time::milliseconds(500));
  BOOST_CHECK(hasResult);
}

BOOST_AUTO_TEST_CASE(FaceDatasetFetchFailure)
{
  nlsr.m_validator.m_shouldValidate = false;
  nlsr.initializeFaces([](const std::vector<ndn::nfd::FaceStatus>& faces) {},
    [this](uint32_t code, const std::string& reason){
      this->nFailureCallbacks++;
    });
  this->advanceClocks(ndn::time::milliseconds(500));

  ndn::Name payload;
  this->sendDataset("/localhost/nfd/faces/list", payload);
  this->advanceClocks(ndn::time::milliseconds(500));

  BOOST_CHECK_EQUAL(nFailureCallbacks, 1);
  BOOST_CHECK_EQUAL(nSuccessCallbacks, 0);
}

BOOST_AUTO_TEST_CASE(FaceDatasetProcess)
{
  Adjacent neighborA("/ndn/neighborA", ndn::util::FaceUri("udp4://192.168.0.100:6363"), 25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", ndn::util::FaceUri("udp4://192.168.0.101:6363"), 10, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  ndn::nfd::FaceStatus payload1;
  payload1.setFaceId(1)
    .setRemoteUri("udp4://192.168.0.100:6363");
  ndn::nfd::FaceStatus payload2;
  payload2.setFaceId(2)
    .setRemoteUri("udp4://192.168.0.101:6363");
  std::vector<ndn::nfd::FaceStatus> faceStatuses = {payload1, payload2};

  nlsr.processFaceDataset(faceStatuses);
  this->advanceClocks(ndn::time::milliseconds(100));

  AdjacencyList adjList = nlsr.getAdjacencyList();

  BOOST_CHECK_EQUAL(adjList.getAdjacent("/ndn/neighborA").getFaceId(), payload1.getFaceId());
  BOOST_CHECK_EQUAL(adjList.getAdjacent("/ndn/neighborB").getFaceId(), payload2.getFaceId());
}

BOOST_AUTO_TEST_CASE(UnconfiguredNeighbor)
{
  Adjacent neighborA("/ndn/neighborA", ndn::util::FaceUri("udp4://192.168.0.100:6363"), 25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  ndn::nfd::FaceStatus payload;
  payload.setFaceId(1)
    .setRemoteUri("udp4://192.168.0.101:6363"); // Note dissimilar Face URI.
  std::vector<ndn::nfd::FaceStatus> faceStatuses = {payload};

  nlsr.processFaceDataset(faceStatuses);
  this->advanceClocks(ndn::time::milliseconds(100));

  AdjacencyList adjList = nlsr.getAdjacencyList();

  BOOST_CHECK_EQUAL(adjList.getAdjacent("/ndn/neighborA").getFaceId(), 0);
}

BOOST_AUTO_TEST_CASE(FaceDatasetPeriodicFetch)
{
  int nNameMatches = 0;
  ndn::Name datasetPrefix("/localhost/nfd/faces/list");
  ndn::nfd::CommandOptions options;
  ndn::time::milliseconds defaultTimeout = options.getTimeout();

  int fetchInterval(1);
  ConfParameter& conf = nlsr.getConfParameter();
  conf.setFaceDatasetFetchInterval(fetchInterval);
  conf.setFaceDatasetFetchTries(0);

  nlsr.initializeFaces(std::bind(&Nlsr::processFaceDataset, &nlsr, _1),
                       std::bind(&Nlsr::onFaceDatasetFetchTimeout, &nlsr, _1, _2, 0));

  // Elapse the default timeout time of the interest.
  this->advanceClocks(defaultTimeout);

  // Check that we have one interest for face list in the sent interests.
  for (const ndn::Interest& interest : face->sentInterests) {
    if (datasetPrefix.isPrefixOf(interest.getName())) {
      nNameMatches++;
    }
  }
  BOOST_CHECK_EQUAL(nNameMatches, 1);

  // Elapse the clock by the reschedule time (that we set)
  this->advanceClocks(ndn::time::seconds(fetchInterval));
  // Elapse the default timeout on the interest.
  this->advanceClocks(defaultTimeout);
  // Plus a little more to let the events process.
  this->advanceClocks(ndn::time::seconds(1));

  // Check that we now have two interests
  nNameMatches = 0;
  for (const ndn::Interest& interest : face->sentInterests) {
    if (datasetPrefix.isPrefixOf(interest.getName())) {
      nNameMatches++;
    }
  }
  BOOST_CHECK_EQUAL(nNameMatches, 2);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
