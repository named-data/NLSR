/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  Regents of the University of California,
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

#include "lsa-segment-storage.hpp"
#include "test-common.hpp"
#include "nlsr.hpp"
#include "name-prefix-list.hpp"

#include <boost/test/unit_test.hpp>

namespace nlsr {
namespace test {

class LsaSegmentStorageFixture : public UnitTestTimeFixture
{
public:
  LsaSegmentStorageFixture()
    : face(m_ioService, m_keyChain)
    , nlsr(m_ioService, m_scheduler, face, m_keyChain)
    , lsdb(nlsr.getLsdb())
    , lsaStorage(lsdb.getLsaStorage())
  {
  }

  static shared_ptr<ndn::Data>
  makeLsaSegment(const ndn::Name& baseName, uint64_t segmentNo, bool isFinal)
  {
    const uint8_t buffer[] = "Hello, world!";

    ndn::Name lsaDataName(baseName);
    lsaDataName.appendSegment(segmentNo);
    auto lsaSegment = make_shared<ndn::Data>(ndn::Name(lsaDataName));
    lsaSegment->setContent(buffer, sizeof(buffer));
    if (isFinal) {
      lsaSegment->setFinalBlockId(lsaSegment->getName()[-1]);
    }

    return signData(lsaSegment);
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
  Nlsr nlsr;
  Lsdb& lsdb;
  LsaSegmentStorage& lsaStorage;
};

BOOST_FIXTURE_TEST_SUITE(TestLsaSegmentStorage, LsaSegmentStorageFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  ndn::Name lsaInterestName("/ndn/NLSR/LSA/other-site/%C1.Router/other-router/NAME");
  lsaInterestName.appendNumber(12);

  ndn::Name lsaDataName(lsaInterestName);
  lsaDataName.appendVersion();

  for (uint64_t segmentNo = 0; segmentNo < 4; ++segmentNo) {
    auto lsaData = makeLsaSegment(lsaDataName, segmentNo, segmentNo == 4);
    lsaStorage.afterFetcherSignalEmitted(*lsaData);
  }

  // receive interest for other-router's LSA that is stored in this router's storage
  for (uint64_t segmentNo = 0; segmentNo < 4; ++segmentNo) {
    receiveLsaInterest(lsaInterestName, segmentNo, segmentNo == 0);
  }

  // 4 data segments should be sent in response to 4 interests
  BOOST_CHECK_EQUAL(face.sentData.size(), 4);
}

BOOST_AUTO_TEST_CASE(DeleteOldLsa)
{
  ndn::Name lsaDataName("/ndn/NLSR/LSA/other-site/%C1.Router/other-router/NAME");
  uint64_t segmentNo = 0;

  uint64_t oldSeqNo = 12;
  ndn::Name oldLsaDataName(lsaDataName);
  oldLsaDataName.appendNumber(oldSeqNo);
  oldLsaDataName.appendVersion();

  auto oldLsaData = makeLsaSegment(oldLsaDataName, segmentNo, true);
  lsaStorage.afterFetcherSignalEmitted(*oldLsaData);
  advanceClocks(ndn::time::milliseconds(1), 10);

  uint64_t newSeqNo = 13;
  ndn::Name newLsaDataName(lsaDataName);
  newLsaDataName.appendNumber(newSeqNo);
  newLsaDataName.appendVersion();

  auto newLsaData = makeLsaSegment(newLsaDataName, segmentNo, true);
  lsaStorage.afterFetcherSignalEmitted(*newLsaData);
  advanceClocks(ndn::time::milliseconds(1), 10);

  ndn::Name lsaInterestName(lsaDataName);

  ndn::Name oldLsaInterestName(lsaInterestName);
  oldLsaInterestName.appendNumber(oldSeqNo);
  receiveLsaInterest(oldLsaInterestName, segmentNo, true);

  advanceClocks(ndn::time::milliseconds(1), 10);

  BOOST_CHECK_EQUAL(face.sentData.size(), 0);

  ndn::Name newLsaInterestName(lsaInterestName);
  newLsaInterestName.appendNumber(newSeqNo);
  receiveLsaInterest(newLsaInterestName, segmentNo, true);

  advanceClocks(ndn::time::milliseconds(1), 10);

  BOOST_CHECK_EQUAL(face.sentData.size(), 1);
}

BOOST_AUTO_TEST_SUITE_END() // TestLsaSegmentStorage

} // namespace test
} // namespace nlsr
