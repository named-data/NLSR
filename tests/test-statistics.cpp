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

#include "statistics.hpp"
#include "test-common.hpp"
#include "hello-protocol.hpp"
#include "lsdb.hpp"
#include "nlsr.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

class StatisticsFixture : public UnitTestTimeFixture
{
public:
  StatisticsFixture()
    : face(m_ioService, m_keyChain)
    , nlsr(m_ioService, m_scheduler, face, m_keyChain)
    , lsdb(nlsr.getLsdb())
    , hello(nlsr.m_helloProtocol)
    , conf(nlsr.getConfParameter())
    , collector(nlsr.getStatsCollector())
  {
    conf.setNetwork("/ndn");
    conf.setSiteName("/site");
    conf.setRouterName("/%C1.Router/this-router");
    conf.buildRouterPrefix();

    addIdentity(conf.getRouterPrefix());

    nlsr.initialize();

    this->advanceClocks(ndn::time::milliseconds(1), 10);
    face.sentInterests.clear();
  }

  /*!
   * \brief Checks if lsa interest was received and data for interest was sent
   *
   * \param interestPrefix is an interest name prefix
   * \param lsaType indicates whether the lsa is a name, adjacency, or coordinate
   * \param seqNo sequence number that will be appended to an interest name
   * \param receivedInterestType is the specific Statisitcs::PacketType interest that is received
   * \param sentDataType is the Statistics::PacketType data being sent upon interest process
   *
   * This is a general function that can be used for all three types of lsa. Calling processInterest()
   * from lsdb will cause the statsCollector to increment the incoming interest type and increment the
   * outgoing data type.
   */
  void
  receiveInterestAndCheckSentStats(const std::string& interestPrefix,
                                   const std::string& lsaType,
                                   uint32_t seqNo,
                                   Statistics::PacketType receivedInterestType,
                                   Statistics::PacketType sentDataType)
  {
    size_t rcvBefore = collector.getStatistics().get(receivedInterestType);
    size_t sentBefore = collector.getStatistics().get(sentDataType);

    ndn::Name interestName = ndn::Name(ndn::Name(interestPrefix + lsaType).appendNumber(seqNo));
    lsdb.processInterest(ndn::Name(), ndn::Interest(interestName));
    this->advanceClocks(ndn::time::milliseconds(1), 10);

    BOOST_CHECK_EQUAL(collector.getStatistics().get(receivedInterestType), rcvBefore + 1);
    BOOST_CHECK_EQUAL(collector.getStatistics().get(sentDataType), sentBefore + 1);
  }

  /*!
   * \brief Checks if statistics update after an lsa interest is sent
   *
   * \param prefix is an interest prefix
   * \param lsaType indicates whether the lsa is a name, adjacency, or coordinate
   * \param seqNo is the sequence number
   * \param statsType is a statistical PacketType
   *
   * The function is called to initiate an expressInterest call in lsdb and to check if the
   * expected statistical packetType was incremented.
   */
  void
  sendInterestAndCheckStats(const std::string& prefix,
                            const std::string& lsaType,
                            uint32_t seqNo,
                            Statistics::PacketType statsType)
  {
    size_t sentBefore = collector.getStatistics().get(statsType);

    lsdb.expressInterest(ndn::Name(prefix + lsaType).appendNumber(seqNo), 0,
                         ndn::time::steady_clock::TimePoint::min());
    this->advanceClocks(ndn::time::milliseconds(1), 10);

    BOOST_CHECK_EQUAL(collector.getStatistics().get(statsType), sentBefore + 1);
  }

public:
  ndn::util::DummyClientFace face;
  Nlsr nlsr;

  Lsdb& lsdb;
  HelloProtocol& hello;
  ConfParameter& conf;
  StatsCollector& collector;
};

BOOST_FIXTURE_TEST_SUITE(TestStatistics, StatisticsFixture)


// A statistical PacketType is directly incremented (without signals).
BOOST_AUTO_TEST_CASE(StatsIncrement)
{
  Statistics stats;
  BOOST_CHECK_EQUAL(stats.get(Statistics::PacketType::SENT_HELLO_INTEREST), 0);
  stats.increment(Statistics::PacketType::SENT_HELLO_INTEREST);
  BOOST_CHECK_EQUAL(stats.get(Statistics::PacketType::SENT_HELLO_INTEREST), 1);
}

/*
 * After a PacketType has been incremented, the resetAll() function is called, which sets all
 * statistical packetType counts to 0
 */
BOOST_AUTO_TEST_CASE(StatsReset)
{
  Statistics stats;
  stats.increment(Statistics::PacketType::SENT_HELLO_INTEREST);
  stats.resetAll();
  BOOST_CHECK_EQUAL(stats.get(Statistics::PacketType::SENT_HELLO_INTEREST), 0);
}

/*
 * This tests hello interests and hello data statistical collection by constructing an adjacency lsa
 * and calling functions that trigger the sending and receiving hello of interests/data.
 */
BOOST_AUTO_TEST_CASE(SendHelloInterest)
{
  Adjacent other("/ndn/router/other", ndn::FaceUri("udp4://other"), 25, Adjacent::STATUS_INACTIVE, 0, 0);

  // This router's Adjacency LSA
  nlsr.getAdjacencyList().insert(other);

  ndn::Name otherName(other.getName());
  otherName.append("NLSR");
  otherName.append("INFO");
  otherName.append(conf.getRouterPrefix().wireEncode());

  hello.expressInterest(otherName, 1);
  this->advanceClocks(ndn::time::milliseconds(1), 10);

  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::SENT_HELLO_INTEREST), 1);

  ndn::Name thisName(conf.getRouterPrefix());
  thisName.append("NLSR");
  thisName.append("INFO");
  thisName.append(other.getName().wireEncode());

  ndn::Interest interest(thisName);
  hello.processInterest(ndn::Name(), interest);

  this->advanceClocks(ndn::time::milliseconds(1), 10);

  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::RCV_HELLO_INTEREST), 1);
  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::SENT_HELLO_DATA), 1);

  // Receive Hello Data
  ndn::Name dataName = otherName;

  ndn::Data data(dataName);
  hello.onContentValidated(data);

  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::RCV_HELLO_DATA), 1);
}

/*
 * An interest is sent for each lsa type (name, adjacency, coordinate). The respective statistics are
 * totaled and checked.
 */
BOOST_AUTO_TEST_CASE(LsdbSendLsaInterest)
{
  const std::string interestPrefix("/ndn/NLSR/LSA/site/%C1.Router/router/");
  uint32_t seqNo = 1;

  // Adjacency LSA
  sendInterestAndCheckStats(interestPrefix, std::to_string(Lsa::Type::ADJACENCY), seqNo,
                            Statistics::PacketType::SENT_ADJ_LSA_INTEREST);

  // Coordinate LSA
  sendInterestAndCheckStats(interestPrefix, std::to_string(Lsa::Type::COORDINATE), seqNo,
                            Statistics::PacketType::SENT_COORD_LSA_INTEREST);

  // Name LSA
  sendInterestAndCheckStats(interestPrefix, std::to_string(Lsa::Type::NAME), seqNo,
                            Statistics::PacketType::SENT_NAME_LSA_INTEREST);

  // 3 total lsa interests were sent
  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::SENT_LSA_INTEREST), 3);
}

/*
 * Tests the statistics collected upon processing incoming lsa
 * interests and respective outgoing data. This process will trigger
 * both an increment for received lsa interest and sent lsa data.
 *
 * /sa receiveInterestAndCheckSentStats
 */
BOOST_AUTO_TEST_CASE(LsdbReceiveInterestSendData)
{
  std::string routerName("/ndn/site/%C1.Router/router");
  ndn::time::system_clock::TimePoint MAX_TIME = ndn::time::system_clock::TimePoint::max();
  uint32_t seqNo = 1;

  // Adjacency LSA
  Adjacent adjacency("adjacency");
  adjacency.setStatus(Adjacent::STATUS_ACTIVE);

  AdjacencyList adjacencies;
  adjacencies.insert(adjacency);

  AdjLsa adjLsa(routerName, seqNo, MAX_TIME, 1, adjacencies);
  lsdb.installAdjLsa(adjLsa);

  const std::string interestPrefix("/ndn/NLSR/LSA/site/%C1.Router/router/");

  // Receive Adjacency LSA Interest
  receiveInterestAndCheckSentStats(interestPrefix,
                                   std::to_string(Lsa::Type::ADJACENCY),
                                   seqNo,
                                   Statistics::PacketType::RCV_ADJ_LSA_INTEREST,
                                   Statistics::PacketType::SENT_ADJ_LSA_DATA);

  // Name LSA
  NamePrefixList prefixes{ndn::Name{"/ndn/name"}};

  NameLsa nameLsa(routerName, seqNo, MAX_TIME, prefixes);
  lsdb.installNameLsa(nameLsa);

  // Receive Name LSA Interest
  receiveInterestAndCheckSentStats(interestPrefix,
                                   std::to_string(Lsa::Type::NAME),
                                   seqNo,
                                   Statistics::PacketType::RCV_NAME_LSA_INTEREST,
                                   Statistics::PacketType::SENT_NAME_LSA_DATA);

  // Coordinate LSA
  std::vector<double> angles = {20.0, 30.0};
  CoordinateLsa coordLsa(routerName, seqNo, MAX_TIME, 2.5, angles);
  lsdb.installCoordinateLsa(coordLsa);

  // Receive Adjacency LSA Interest
  receiveInterestAndCheckSentStats(interestPrefix,
                                   std::to_string(Lsa::Type::COORDINATE),
                                   seqNo,
                                   Statistics::PacketType::RCV_COORD_LSA_INTEREST,
                                   Statistics::PacketType::SENT_COORD_LSA_DATA);

  // 3 different lsa type interests should be received
  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::RCV_LSA_INTEREST), 3);

  // data should have been sent 3x, once per lsa type
  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::SENT_LSA_DATA), 3);
}

/*
 * Data for each lsa type (name, adjacency, coordinate) is sent to the lsdb and statistics are
 * checked to verify the respective statistical PacketType has been received.
 */
BOOST_AUTO_TEST_CASE(LsdbReceiveData)
{
  ndn::Name routerName("/ndn/cs/%C1.Router/router1");
  uint32_t seqNo = 1;
  ndn::time::system_clock::TimePoint MAX_TIME = ndn::time::system_clock::TimePoint::max();

  // adjacency lsa
  ndn::Name adjInterest("/ndn/NLSR/LSA/cs/%C1.Router/router1/ADJACENCY/");
  adjInterest.appendNumber(seqNo);
  AdjLsa aLsa(routerName, seqNo, MAX_TIME, 1, nlsr.getAdjacencyList());
  lsdb.installAdjLsa(aLsa);

  const ndn::ConstBufferPtr aBuffer = std::make_shared<ndn::Buffer>(aLsa.serialize().c_str(),
                                                                    aLsa.serialize().size());
  lsdb.afterFetchLsa(aBuffer, adjInterest);
  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::RCV_ADJ_LSA_DATA), 1);

  // coordinate lsa
  ndn::Name coordInterest("/ndn/NLSR/LSA/cs/%C1.Router/router1/COORDINATE/");
  coordInterest.appendNumber(seqNo);
  std::vector<double> angles = {20.0, 30.0};
  CoordinateLsa cLsa(routerName, seqNo, MAX_TIME, 2.5, angles);
  lsdb.installCoordinateLsa(cLsa);

  const ndn::ConstBufferPtr cBuffer = std::make_shared<ndn::Buffer>(cLsa.serialize().c_str(),
                                                                   cLsa.serialize().size());
  lsdb.afterFetchLsa(cBuffer, coordInterest);
  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::RCV_COORD_LSA_DATA), 1);

  // name lsa
  ndn::Name interestName("/ndn/NLSR/LSA/cs/%C1.Router/router1/NAME/");
  interestName.appendNumber(seqNo);
  NameLsa nLsa(routerName, seqNo, MAX_TIME, nlsr.getNamePrefixList());
  lsdb.installNameLsa(nLsa);

  const ndn::ConstBufferPtr nBuffer = std::make_shared<ndn::Buffer>(nLsa.serialize().c_str(),
                                                                   nLsa.serialize().size());
  lsdb.afterFetchLsa(nBuffer, interestName);
  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::RCV_NAME_LSA_DATA), 1);

  // 3 lsa data types should be received
  BOOST_CHECK_EQUAL(collector.getStatistics().get(Statistics::PacketType::RCV_LSA_DATA), 3);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
