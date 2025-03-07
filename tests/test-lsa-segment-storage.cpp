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

#include "nlsr.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

#include <ndn-cxx/ims/in-memory-storage-fifo.hpp>
#include <ndn-cxx/util/segmenter.hpp>

namespace nlsr::tests {

class LsaSegmentStorageFixture : public IoKeyChainFixture
{
public:
  LsaSegmentStorageFixture()
  {
    afterSegmentValidatedConn = lsdb.afterSegmentValidatedSignal.connect([this] (auto&&...) {
      ++numValidationSignal;
    });

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
  makeLsaContent(ndn::Name interestName, int numNames = 1000)
  {
    const ndn::time::seconds refreshTime{LSA_REFRESH_TIME_DEFAULT};

    NamePrefixList npl1;
    for (int i = 0; i < numNames; ++i) {
      npl1.insert("name1-" + std::to_string(i));
    }
    NameLsa nameLsa("/ndn/other-site/%C1.Router/other-router", 12,
                    ndn::time::system_clock::now() + refreshTime, npl1);

    interestName.appendVersion();
    ndn::Segmenter segmenter(m_keyChain, ndn::security::SigningInfo());
    auto segments = segmenter.segment(nameLsa.wireEncode(), interestName,
                                      ndn::MAX_NDN_PACKET_SIZE / 2, refreshTime);
    for (const auto& seg : segments) {
      ims.insert(*seg, refreshTime);
    }
  }

  void
  sendReplies()
  {
    for (const auto& interest : face.sentInterests) {
      if (auto data = ims.find(interest.getName()); data) {
        face.put(*data);
      }
    }
    face.sentInterests.clear();
  }

public:
  ndn::DummyClientFace face{m_io, m_keyChain, {true, true}};
  ConfParameter conf{face, m_keyChain};
  DummyConfFileProcessor confProcessor{conf};
  Nlsr nlsr{face, m_keyChain, conf};
  Lsdb& lsdb{nlsr.m_lsdb};
  ndn::InMemoryStorageFifo ims{100};

  int numValidationSignal = 0;
  ndn::signal::ScopedConnection afterSegmentValidatedConn;
};

BOOST_FIXTURE_TEST_SUITE(TestLsaSegmentStorage, LsaSegmentStorageFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
  ndn::Name lsaInterestName("/ndn/NLSR/LSA/other-site/%C1.Router/other-router/NAME");
  lsaInterestName.appendNumber(12);

  lsdb.expressInterest(lsaInterestName, 0, 0);
  advanceClocks(ndn::time::milliseconds(10));

  makeLsaContent(lsaInterestName);
  //Does seem to be generating 6 segments, but how to validate correct contents?
  BOOST_CHECK_EQUAL(ims.size(), 6);

  sendReplies();
  // 1st segment
  advanceClocks(ndn::time::milliseconds(10));
  // 2nd and 3rd segments
  sendReplies();
  advanceClocks(ndn::time::milliseconds(10));
  // 4th and 5th segments
  sendReplies();
  // 6th segment
  advanceClocks(ndn::time::milliseconds(10));

  // 3 data segments should be in the storage
  BOOST_CHECK_EQUAL(lsdb.m_lsaStorage.size(), 6);
  BOOST_CHECK_EQUAL(numValidationSignal, 6);
  numValidationSignal = 0;

  // Remove older LSA from storage upon receiving that of higher sequence
  ndn::Name lsaInterestName2("/ndn/NLSR/LSA/other-site/%C1.Router/other-router/NAME");
  lsaInterestName2.appendNumber(13);
  lsdb.expressInterest(lsaInterestName2, 0, 0);
  advanceClocks(ndn::time::milliseconds(10));

  makeLsaContent(lsaInterestName2, 1);
  sendReplies();
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

} // namespace nlsr::tests
