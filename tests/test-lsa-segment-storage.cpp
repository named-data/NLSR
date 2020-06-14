/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test-common.hpp"
#include "nlsr.hpp"
#include "name-prefix-list.hpp"
// #include "lsa.hpp"

namespace nlsr {
namespace test {

class LsaSegmentStorageFixture : public UnitTestTimeFixture
{
public:
  LsaSegmentStorageFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , conf(face, m_keyChain)
    , confProcessor(conf)
    , nlsr(face, m_keyChain, conf)
    , lsdb(nlsr.m_lsdb)
    , segmentPublisher(face, m_keyChain)
    , numValidationSignal(0)
    , afterSegmentValidatedConnection(lsdb.afterSegmentValidatedSignal.connect(
                                      [this] (const ndn::Data& data) { ++numValidationSignal; }))
  {
    std::string config = R"CONF(
                            trust-anchor
                              {
                                type any
                              }
                          )CONF";
    conf.m_validator.load(config, "config-file-from-string");

    this->advanceClocks(ndn::time::milliseconds(10));
    face.sentInterests.clear();
  }

  void
  makeLsaContent(const ndn::Name& interestName, int numNames = 1000)
  {
    NamePrefixList npl1;
    for (int i = 0; i < numNames; ++i) {
      npl1.insert("name1-" + std::to_string(i));
    }
    NameLsa nameLsa("/ndn/other-site/%C1.Router/other-router", 12,
                    ndn::time::system_clock::now() + ndn::time::seconds(LSA_REFRESH_TIME_DEFAULT),
                    npl1);
    segmentPublisher.publish(interestName, interestName, nameLsa.wireEncode(),
                             ndn::time::seconds(LSA_REFRESH_TIME_DEFAULT));
  }

  void
  receiveLsaInterest(const ndn::Name& baseInterestName, uint64_t segmentNo,
                     bool isSegmentZero)
  {
    if (isSegmentZero) {
      lsdb.processInterest(ndn::Name(), ndn::Interest(baseInterestName));
    }
    else {
      ndn::Name nextInterestName(baseInterestName);
      nextInterestName.appendSegment(segmentNo);
      lsdb.processInterest(ndn::Name(), ndn::Interest(nextInterestName));
      advanceClocks(ndn::time::milliseconds(1), 10);
    }
  }

public:
  ndn::util::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  Nlsr nlsr;
  Lsdb& lsdb;
  psync::SegmentPublisher segmentPublisher;

  int numValidationSignal;
  ndn::util::signal::ScopedConnection afterSegmentValidatedConnection;
};

BOOST_FIXTURE_TEST_SUITE(TestLsaSegmentStorage, LsaSegmentStorageFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  ndn::Name lsaInterestName("/ndn/NLSR/LSA/other-site/%C1.Router/other-router/NAME");
  lsaInterestName.appendNumber(12);

  lsdb.expressInterest(lsaInterestName, 0);
  advanceClocks(ndn::time::milliseconds(10));

  makeLsaContent(lsaInterestName);
  advanceClocks(ndn::time::milliseconds(10));

  for (const auto& interest : face.sentInterests) {
    segmentPublisher.replyFromStore(interest.getName());
    advanceClocks(ndn::time::milliseconds(10));
  }

  // 3 data segments should be in the storage
  BOOST_CHECK_EQUAL(lsdb.m_lsaStorage.size(), 3);
  BOOST_CHECK_EQUAL(numValidationSignal, 3);
  numValidationSignal = 0;
  face.sentInterests.clear();

  // Remove older LSA from storage upon receiving that of higher sequence
  ndn::Name lsaInterestName2("/ndn/NLSR/LSA/other-site/%C1.Router/other-router/NAME");
  lsaInterestName2.appendNumber(13);
  lsdb.expressInterest(lsaInterestName2, 0);
  advanceClocks(ndn::time::milliseconds(10));

  makeLsaContent(lsaInterestName2, 1);
  advanceClocks(ndn::time::milliseconds(10));
  // Should have cleared all the three segments for the previous interest w/ seq 12
  // And add one segment for this sequence 13
  BOOST_CHECK_EQUAL(lsdb.m_lsaStorage.size(), 1);
  BOOST_CHECK_EQUAL(numValidationSignal, 1);

  // Scheduled removal of LSA
  advanceClocks(ndn::time::seconds(LSA_REFRESH_TIME_DEFAULT));
  BOOST_CHECK_EQUAL(lsdb.m_lsaStorage.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END() // TestLsaSegmentStorage

} // namespace test
} // namespace nlsr
