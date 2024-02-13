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

#include "nlsr.hpp"
#include "logger.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

#include <ndn-cxx/mgmt/nfd/face-event-notification.hpp>

namespace nlsr::tests {

class MockNfdMgmtFixture : public IoKeyChainFixture
{
public:
  /** \brief send one WireEncodable in reply to StatusDataset request
   *  \param prefix dataset prefix without version and segment
   *  \param payload payload block
   *  \note payload must fit in one Data
   *  \pre Interest for dataset has been expressed, sendDataset has not been invoked
   */
  template<typename T>
  void
  sendDataset(const ndn::Name& prefix, const T& payload)
  {
    BOOST_CONCEPT_ASSERT((ndn::WireEncodable<T>));

    this->sendDatasetReply(prefix, payload.wireEncode());
  }

  /** \brief send two WireEncodables in reply to StatusDataset request
   *  \param prefix dataset prefix without version and segment
   *  \param payload1 first vector item
   *  \param payload2 second vector item
   *  \note all payloads must fit in one Data
   *  \pre Interest for dataset has been expressed, sendDataset has not been invoked
   */
  template<typename T1, typename T2>
  void
  sendDataset(const ndn::Name& prefix, const T1& payload1, const T2& payload2)
  {
    BOOST_CONCEPT_ASSERT((ndn::WireEncodable<T1>));
    BOOST_CONCEPT_ASSERT((ndn::WireEncodable<T2>));

    ndn::encoding::EncodingBuffer buffer;
    payload2.wireEncode(buffer);
    payload1.wireEncode(buffer);

    this->sendDatasetReply(prefix, buffer);
  }

  /** \brief send a payload in reply to StatusDataset request
   *  \param name dataset prefix without version and segment
   *  \param contentArgs passed to Data::setContent
   */
  template<typename ...ContentArgs>
  void
  sendDatasetReply(ndn::Name name, ContentArgs&&... contentArgs)
  {
    name.appendVersion().appendSegment(0);

    // These warnings assist in debugging when nfdc does not receive StatusDataset.
    // They usually indicate a misspelled prefix or incorrect timing in the test case.
    if (m_face.sentInterests.empty()) {
      BOOST_WARN_MESSAGE(false, "no Interest expressed");
    }
    else {
      BOOST_WARN_MESSAGE(m_face.sentInterests.back().getName().isPrefixOf(name),
                         "last Interest " << m_face.sentInterests.back().getName() <<
                         " cannot be satisfied by this Data " << name);
    }

    auto data = std::make_shared<ndn::Data>(name);
    data->setFreshnessPeriod(1_s);
    data->setFinalBlock(name[-1]);
    data->setContent(std::forward<ContentArgs>(contentArgs)...);
    signData(*data);
    m_face.receive(*data);
  }

public:
  ndn::DummyClientFace m_face{m_io, m_keyChain, {true, true}};
};

class NlsrFixture : public MockNfdMgmtFixture
{
public:
  NlsrFixture()
    : conf(m_face, m_keyChain)
    , confProcessor(conf)
    , nlsr(m_face, m_keyChain, conf)
    , lsdb(nlsr.m_lsdb)
    , neighbors(conf.getAdjacencyList())
    , nSuccessCallbacks(0)
    , nFailureCallbacks(0)
  {
    m_keyChain.createIdentity(conf.getRouterPrefix());
  }

  void
  receiveHelloData(const ndn::Name& sender, const ndn::Name& receiver)
  {
    ndn::Name dataName(sender);
    dataName.append("NLSR")
            .append("INFO")
            .append(ndn::tlv::GenericNameComponent, receiver.wireEncode())
            .appendVersion();

    ndn::Data data(dataName);
    nlsr.m_helloProtocol.onContentValidated(data);
  }

public:
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  Nlsr nlsr;
  Lsdb& lsdb;
  AdjacencyList& neighbors;
  uint32_t nSuccessCallbacks;
  uint32_t nFailureCallbacks;
  ndn::signal::ScopedConnection connection;
};

BOOST_FIXTURE_TEST_SUITE(TestNlsr, NlsrFixture)

BOOST_AUTO_TEST_CASE(HyperbolicOn_ZeroCostNeighbors)
{
  // Simulate loading configuration file
  Adjacent neighborA("/ndn/neighborA", ndn::FaceUri("udp4://10.0.0.1"), 25,
                     Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", ndn::FaceUri("udp4://10.0.0.2"), 10,
                     Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  Adjacent neighborC("/ndn/neighborC", ndn::FaceUri("udp4://10.0.0.3"), 17,
                     Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborC);

  conf.setHyperbolicState(HYPERBOLIC_STATE_ON);

  Nlsr nlsr2(m_face, m_keyChain, conf);

  for (const auto& neighbor : neighbors.getAdjList()) {
    BOOST_CHECK_EQUAL(neighbor.getLinkCost(), 0);
  }
}

BOOST_AUTO_TEST_CASE(HyperbolicOff_LinkStateCost)
{
  // Simulate loading configuration file
  Adjacent neighborA("/ndn/neighborA", ndn::FaceUri("udp4://10.0.0.1"), 25,
                     Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", ndn::FaceUri("udp4://10.0.0.2"), 10,
                     Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  Adjacent neighborC("/ndn/neighborC", ndn::FaceUri("udp4://10.0.0.3"), 17,
                     Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborC);

  Nlsr nlsr2(m_face, m_keyChain, conf);

  for (const auto& neighbor : neighbors.getAdjList()) {
    BOOST_CHECK_NE(neighbor.getLinkCost(), 0);
  }
}

BOOST_AUTO_TEST_CASE(SetEventIntervals)
{
  // Simulate loading configuration file
  conf.setAdjLsaBuildInterval(3);
  conf.setRoutingCalcInterval(9);

  Nlsr nlsr2(m_face, m_keyChain, conf);

  const Lsdb& lsdb = nlsr2.m_lsdb;
  const RoutingTable& rt = nlsr2.m_routingTable;

  BOOST_CHECK_EQUAL(lsdb.m_adjLsaBuildInterval, 3_s);
  BOOST_CHECK_EQUAL(rt.m_routingCalcInterval, 9_s);
}

BOOST_AUTO_TEST_CASE(FaceCreateEvent)
{
  // Setting constants for the unit test
  const uint32_t faceId = 128;
  const std::string faceUri = "udp4://10.0.0.1:6363";

  Adjacent neighbor("/ndn/neighborA", ndn::FaceUri(faceUri), 10,
                    Adjacent::STATUS_INACTIVE, 0, 0);

  BOOST_REQUIRE_EQUAL(conf.getAdjacencyList().insert(neighbor), true);

  this->advanceClocks(10_ms);

  // Build, sign, and send the Face Event
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_CREATED)
    .setRemoteUri(faceUri)
    .setFaceId(faceId);
  auto data = std::make_shared<ndn::Data>(ndn::Name("/localhost/nfd/faces/events").appendSequenceNumber(0));
  data->setFreshnessPeriod(1_s);
  data->setContent(event.wireEncode());
  m_keyChain.sign(*data);
  m_face.receive(*data);

  // Move the clocks forward so that the Face processes the event.
  this->advanceClocks(10_ms);

  // Need to explicitly provide a FaceUri object, because the
  // conversion will attempt to create Name objects.
  auto iterator = conf.getAdjacencyList().findAdjacent(ndn::FaceUri(faceUri));
  BOOST_REQUIRE(iterator != conf.getAdjacencyList().end());
  BOOST_CHECK_EQUAL(iterator->getFaceId(), faceId);
}

BOOST_AUTO_TEST_CASE(FaceCreateEventNoMatch)
{
  // Setting constants for the unit test
  const uint32_t faceId = 128;
  const std::string eventUri = "udp4://10.0.0.1:6363";
  const std::string neighborUri = "udp4://10.0.0.2:6363";

  Adjacent neighbor("/ndn/neighborA", ndn::FaceUri(neighborUri), 10,
                    Adjacent::STATUS_INACTIVE, 0, 0);

  conf.getAdjacencyList().insert(neighbor);

  // Build, sign, and send the Face Event
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_CREATED)
    .setRemoteUri(eventUri)
    .setFaceId(faceId);
  auto data = std::make_shared<ndn::Data>(ndn::Name("/localhost/nfd/faces/events").appendSequenceNumber(0));
  data->setFreshnessPeriod(1_s);
  data->setContent(event.wireEncode());
  m_keyChain.sign(*data);
  m_face.receive(*data);

  // Move the clocks forward so that the Face processes the event.
  this->advanceClocks(10_ms);

  // The Face URIs did not match, so this neighbor should be unconfigured.
  auto iterator = conf.getAdjacencyList().findAdjacent(ndn::FaceUri(neighborUri));
  BOOST_REQUIRE(iterator != conf.getAdjacencyList().end());
  BOOST_CHECK_EQUAL(iterator->getFaceId(), 0);
}

BOOST_AUTO_TEST_CASE(FaceCreateEventAlreadyConfigured)
{
  const uint32_t neighborFaceId = 128;
  const std::string faceUri = "udp4://10.0.0.1:6363";

  Adjacent neighbor("/ndn/neighborA", ndn::FaceUri(faceUri), 10,
                    Adjacent::STATUS_ACTIVE, 0, 0);
  conf.getAdjacencyList().insert(neighbor);

  // Let NLSR start the face monitor
  this->advanceClocks(10_ms);

  // Build, sign, and send the Face Event
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_CREATED)
    .setRemoteUri(faceUri)
    .setFaceId(neighborFaceId);
  auto data = std::make_shared<ndn::Data>(ndn::Name("/localhost/nfd/faces/events").appendSequenceNumber(0));
  data->setFreshnessPeriod(1_s);
  data->setContent(event.wireEncode());
  m_keyChain.sign(*data);
  m_face.receive(*data);

  // Move the clocks forward so that the Face processes the event.
  this->advanceClocks(10_ms);

  auto iterator = conf.getAdjacencyList().findAdjacent(ndn::FaceUri(faceUri));
  BOOST_REQUIRE(iterator != conf.getAdjacencyList().end());
  BOOST_CHECK_EQUAL(iterator->getFaceId(), neighborFaceId);

  // Resend same event notification again
  m_face.sentInterests.clear();
  data->setName(ndn::Name("/localhost/nfd/faces/events").appendSequenceNumber(1));
  m_keyChain.sign(*data);
  m_face.receive(*data);
  this->advanceClocks(10_ms);

  for (const auto& interest : m_face.sentInterests) {
    // Should not re-register prefix since this is the same event notification
    if (ndn::Name("/localhost/nfd/rib/register").isPrefixOf(interest.getName())) {
      BOOST_CHECK(false);
    }
  }
}

BOOST_AUTO_TEST_CASE(FaceDestroyEvent)
{
  // Add active neighbors
  AdjacencyList& neighbors = conf.getAdjacencyList();
  uint64_t destroyFaceId = 128;

  // Create a neighbor whose Face will be destroyed
  Adjacent failNeighbor("/ndn/neighborA", ndn::FaceUri("udp4://10.0.0.1"),
                        10, Adjacent::STATUS_ACTIVE, 0, destroyFaceId);
  neighbors.insert(failNeighbor);

  // Create an additional neighbor so an adjacency LSA can be built after the face is destroyed
  Adjacent otherNeighbor("/ndn/neighborB", ndn::FaceUri("udp4://10.0.0.2"),
                          10, Adjacent::STATUS_ACTIVE, 0, 256);
  neighbors.insert(otherNeighbor);

  // Set HelloInterest lifetime as 10 seconds so that neighbors are not marked INACTIVE
  // upon timeout before this test ends
  conf.setInterestResendTime(10);

  // Simulate successful HELLO responses
  lsdb.scheduleAdjLsaBuild();

  // Set up adjacency LSAs
  // This router
  Adjacent thisRouter(conf.getRouterPrefix(), ndn::FaceUri("udp4://10.0.0.3"),
                      10, Adjacent::STATUS_ACTIVE, 0, 256);

  AdjLsa ownAdjLsa(conf.getRouterPrefix(), 10,
                   ndn::time::system_clock::now(), neighbors);
  lsdb.installLsa(std::make_shared<AdjLsa>(ownAdjLsa));

  // Router that will fail
  AdjacencyList failAdjacencies;
  failAdjacencies.insert(thisRouter);

  AdjLsa failAdjLsa("/ndn/neighborA", 10,
                    ndn::time::system_clock::now() + ndn::time::seconds(3600),
                    failAdjacencies);

  lsdb.installLsa(std::make_shared<AdjLsa>(failAdjLsa));

  // Other router
  AdjacencyList otherAdjacencies;
  otherAdjacencies.insert(thisRouter);

  AdjLsa otherAdjLsa("/ndn/neighborB", 10,
                     ndn::time::system_clock::now() + ndn::time::seconds(3600),
                     otherAdjacencies);

  lsdb.installLsa(std::make_shared<AdjLsa>(otherAdjLsa));

  // Run the scheduler to build an adjacency LSA
  this->advanceClocks(10_ms);

  // Make sure an adjacency LSA was built
  auto lsa = lsdb.findLsa(conf.getRouterPrefix(), Lsa::Type::ADJACENCY);
  BOOST_REQUIRE(lsa != nullptr);

  uint32_t lastAdjLsaSeqNo = lsa->getSeqNo();
  nlsr.m_lsdb.m_sequencingManager.setAdjLsaSeq(lastAdjLsaSeqNo);

  this->advanceClocks(1500_ms, 10);

  // Make sure the routing table was calculated
  RoutingTableEntry* rtEntry = nlsr.m_routingTable.findRoutingTableEntry(failNeighbor.getName());
  BOOST_REQUIRE(rtEntry != nullptr);
  BOOST_REQUIRE_EQUAL(rtEntry->getNexthopList().size(), 1);

  // Receive FaceEventDestroyed notification
  ndn::nfd::FaceEventNotification event;
  event.setKind(ndn::nfd::FACE_EVENT_DESTROYED)
       .setFaceId(destroyFaceId);

  auto data = std::make_shared<ndn::Data>(ndn::Name("/localhost/nfd/faces/events").appendSequenceNumber(0));
  data->setFreshnessPeriod(1_s);
  data->setContent(event.wireEncode());
  m_keyChain.sign(*data);

  m_face.receive(*data);

  // Run the scheduler to build an adjacency LSA
  this->advanceClocks(10_ms);

  Adjacent updatedNeighbor = neighbors.getAdjacent(failNeighbor.getName());

  BOOST_CHECK_EQUAL(updatedNeighbor.getFaceId(), 0);
  BOOST_CHECK_EQUAL(updatedNeighbor.getInterestTimedOutNo(),
                    conf.getInterestRetryNumber());
  BOOST_CHECK_EQUAL(updatedNeighbor.getStatus(), Adjacent::STATUS_INACTIVE);

  lsa = lsdb.findLsa<AdjLsa>(conf.getRouterPrefix());
  BOOST_REQUIRE(lsa != nullptr);

  BOOST_CHECK_EQUAL(lsa->getSeqNo(), lastAdjLsaSeqNo + 1);

  this->advanceClocks(15_s, 10);

  // Make sure the routing table was recalculated
  rtEntry = nlsr.m_routingTable.findRoutingTableEntry(failNeighbor.getName());
  BOOST_CHECK(rtEntry == nullptr);
}

BOOST_AUTO_TEST_CASE(BuildAdjLsaAfterHelloResponse)
{
  // Configure NLSR
  conf.setAdjLsaBuildInterval(1);

  // Add neighbors
  // Router A
  ndn::Name neighborAName("/ndn/site/%C1.Router/routerA");
  Adjacent neighborA(neighborAName, ndn::FaceUri("udp4://10.0.0.1"),
                     0, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  // Router B
  ndn::Name neighborBName("/ndn/site/%C1.Router/routerB");
  Adjacent neighborB(neighborBName, ndn::FaceUri("udp4://10.0.0.1"),
                     0, Adjacent::STATUS_INACTIVE, 0, 0);

  neighbors.insert(neighborB);

  this->advanceClocks(10_ms);

  // Receive HELLO response from Router A
  receiveHelloData(neighborAName, conf.getRouterPrefix());
  this->advanceClocks(1_s, 10);

  // Adjacency LSA should be built even though other router is INACTIVE
  auto lsa = lsdb.findLsa<AdjLsa>(conf.getRouterPrefix());
  BOOST_REQUIRE(lsa != nullptr);
  BOOST_CHECK_EQUAL(lsa->getAdl().size(), 1);

  // Receive HELLO response from Router B
  receiveHelloData(neighborBName, conf.getRouterPrefix());

  // Both routers become INACTIVE and HELLO Interests have timed out
  for (Adjacent& adjacency : neighbors.getAdjList()) {
    adjacency.setStatus(Adjacent::STATUS_INACTIVE);
    adjacency.setInterestTimedOutNo(HELLO_RETRIES_DEFAULT);
  }

  this->advanceClocks(1_s, 10);

  // Adjacency LSA should have been removed since this router's adjacencies are
  // INACTIVE and have timed out
  lsa = lsdb.findLsa<AdjLsa>(conf.getRouterPrefix());
  BOOST_CHECK(lsa == nullptr);

  // Receive HELLO response from Router A and B
  receiveHelloData(neighborAName, conf.getRouterPrefix());
  receiveHelloData(neighborBName, conf.getRouterPrefix());
  this->advanceClocks(1_s, 10);

  // Adjacency LSA should be built
  lsa = lsdb.findLsa<AdjLsa>(conf.getRouterPrefix());
  BOOST_REQUIRE(lsa != nullptr);
  BOOST_CHECK_EQUAL(lsa->getAdl().size(), 2);
}

BOOST_AUTO_TEST_CASE(FaceDatasetFetchSuccess)
{
  bool hasResult = false;

  nlsr.initializeFaces([&hasResult] (const std::vector<ndn::nfd::FaceStatus>& faces) {
      hasResult = true;
      BOOST_CHECK_EQUAL(faces.size(), 2);
      BOOST_CHECK_EQUAL(faces.front().getFaceId(), 25401);
      BOOST_CHECK_EQUAL(faces.back().getFaceId(), 25402);
    },
    [] (uint32_t code, const std::string& reason) {});

  this->advanceClocks(100_ms, 5);

  ndn::nfd::FaceStatus payload1;
  payload1.setFaceId(25401);
  ndn::nfd::FaceStatus payload2;
  payload2.setFaceId(25402);
  this->sendDataset("/localhost/nfd/faces/list", payload1, payload2);

  this->advanceClocks(100_ms, 5);
  BOOST_CHECK(hasResult);
}

BOOST_AUTO_TEST_CASE(FaceDatasetFetchFailure)
{
  nlsr.initializeFaces([](const std::vector<ndn::nfd::FaceStatus>& faces) {},
    [this](uint32_t code, const std::string& reason){
      this->nFailureCallbacks++;
    });
  this->advanceClocks(100_ms, 5);

  ndn::Name payload;
  this->sendDataset("/localhost/nfd/faces/list", payload);
  this->advanceClocks(100_ms, 5);

  BOOST_CHECK_EQUAL(nFailureCallbacks, 1);
  BOOST_CHECK_EQUAL(nSuccessCallbacks, 0);
}

BOOST_AUTO_TEST_CASE(FaceDatasetProcess)
{
  Adjacent neighborA("/ndn/neighborA", ndn::FaceUri("udp4://192.168.0.100:6363"),
                     25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  Adjacent neighborB("/ndn/neighborB", ndn::FaceUri("udp4://192.168.0.101:6363"),
                     10, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborB);

  ndn::nfd::FaceStatus payload1;
  payload1.setFaceId(1)
    .setRemoteUri("udp4://192.168.0.100:6363");
  ndn::nfd::FaceStatus payload2;
  payload2.setFaceId(2)
    .setRemoteUri("udp4://192.168.0.101:6363");
  std::vector<ndn::nfd::FaceStatus> faceStatuses = {payload1, payload2};

  nlsr.processFaceDataset(faceStatuses);

  AdjacencyList adjList = conf.getAdjacencyList();

  BOOST_CHECK_EQUAL(adjList.getAdjacent("/ndn/neighborA").getFaceId(), payload1.getFaceId());
  BOOST_CHECK_EQUAL(adjList.getAdjacent("/ndn/neighborB").getFaceId(), payload2.getFaceId());
}

BOOST_AUTO_TEST_CASE(UnconfiguredNeighbor)
{
  Adjacent neighborA("/ndn/neighborA", ndn::FaceUri("udp4://192.168.0.100:6363"), 25, Adjacent::STATUS_INACTIVE, 0, 0);
  neighbors.insert(neighborA);

  ndn::nfd::FaceStatus payload;
  payload.setFaceId(1)
    .setRemoteUri("udp4://192.168.0.101:6363"); // Note dissimilar Face URI.
  std::vector<ndn::nfd::FaceStatus> faceStatuses = {payload};

  nlsr.processFaceDataset(faceStatuses);
  this->advanceClocks(20_ms, 5);

  AdjacencyList adjList = conf.getAdjacencyList();

  BOOST_CHECK_EQUAL(adjList.getAdjacent("/ndn/neighborA").getFaceId(), 0);
}

BOOST_AUTO_TEST_CASE(FaceDatasetPeriodicFetch)
{
  int nNameMatches = 0;
  ndn::Name datasetPrefix("/localhost/nfd/faces/list");
  ndn::nfd::CommandOptions options;
  ndn::time::milliseconds defaultTimeout = options.getTimeout();

  int fetchInterval(1);
  conf.setFaceDatasetFetchInterval(fetchInterval);
  conf.setFaceDatasetFetchTries(0);

  // Elapse the default timeout time of the interest.
  this->advanceClocks(defaultTimeout);

  // Check that we have one interest for face list in the sent interests.
  for (const auto& interest : m_face.sentInterests) {
    if (datasetPrefix.isPrefixOf(interest.getName())) {
      nNameMatches++;
    }
  }
  BOOST_CHECK_EQUAL(nNameMatches, 1);

  // Elapse the clock by the reschedule time (that we set)
  this->advanceClocks(ndn::time::seconds(fetchInterval));
  // Elapse the default timeout on the interest.
  this->advanceClocks(defaultTimeout);

  // Check that we now have two interests
  nNameMatches = 0;
  for (const auto& interest : m_face.sentInterests) {
    if (datasetPrefix.isPrefixOf(interest.getName())) {
      nNameMatches++;
    }
  }
  BOOST_CHECK_EQUAL(nNameMatches, 2);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
