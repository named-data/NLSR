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

#include "lsdb.hpp"
#include "lsa/lsa.hpp"
#include "name-prefix-list.hpp"

#include "tests/io-key-chain-fixture.hpp"
#include "tests/test-common.hpp"

#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/security/validator-null.hpp>
#include <ndn-cxx/util/segment-fetcher.hpp>

#include <unistd.h>

namespace nlsr::tests {

class LsdbFixture : public IoKeyChainFixture
{
public:
  LsdbFixture()
    : face(m_io, m_keyChain, {true, true})
    , conf(face, m_keyChain)
    , confProcessor(conf)
    , lsdb(face, m_keyChain, conf)
  {
    m_keyChain.createIdentity("/ndn/site/%C1.Router/this-router");

    advanceClocks(10_ms);
    face.sentInterests.clear();
  }

  void
  isFirstNameLsaEqual(const Lsdb& otherLsdb)
  {
    auto selfLsaRange = lsdb.getLsdbIterator<NameLsa>();
    auto otherLsaRange = otherLsdb.getLsdbIterator<NameLsa>();

    if (selfLsaRange.first != selfLsaRange.second && otherLsaRange.first != otherLsaRange.second) {
      auto ownLsa = std::static_pointer_cast<NameLsa>(*selfLsaRange.first);
      auto otherLsa = std::static_pointer_cast<NameLsa>(*otherLsaRange.first);
      BOOST_CHECK_EQUAL(ownLsa->getNpl(), otherLsa->getNpl());
      return;
    }
    BOOST_CHECK(false);
  }

  void
  checkSignalResult(LsdbUpdate updateType,
                    const std::shared_ptr<Lsa>& lsaPtr,
                    const std::list<PrefixInfo>& namesToAdd,
                    const std::list<PrefixInfo>& namesToRemove)
  {
    BOOST_CHECK(updateHappened);
    BOOST_CHECK_EQUAL(lsaPtrCheck->getOriginRouter(), lsaPtr->getOriginRouter());
    BOOST_CHECK_EQUAL(lsaPtrCheck->getType(), lsaPtr->getType());
    BOOST_CHECK(updateType == updateTypeCheck);
    BOOST_CHECK(namesToAdd == namesToAddCheck);
    BOOST_CHECK(namesToRemove == namesToRemoveCheck);
    updateHappened = false;
  }

  void
  connectSignal()
  {
    lsdb.onLsdbModified.connect(
      [&] (std::shared_ptr<Lsa> lsa, LsdbUpdate updateType,
           const auto& namesToAdd, const auto& namesToRemove) {
        lsaPtrCheck = lsa;
        updateTypeCheck = updateType;
        namesToAddCheck = namesToAdd;
        namesToRemoveCheck = namesToRemove;
        updateHappened = true;
      }
    );
  }

public:
  ndn::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  Lsdb lsdb;

  LsdbUpdate updateTypeCheck = LsdbUpdate::INSTALLED;
  std::list<PrefixInfo> namesToAddCheck;
  std::list<PrefixInfo> namesToRemoveCheck;
  std::shared_ptr<Lsa> lsaPtrCheck = nullptr;
  bool updateHappened = false;
};

BOOST_FIXTURE_TEST_SUITE(TestLsdb, LsdbFixture)

BOOST_AUTO_TEST_CASE(LsdbSync)
{
  ndn::Name interestName("/ndn/NLSR/LSA/cs/%C1.Router/router2/name");
  uint64_t oldSeqNo = 82;

  ndn::Name oldInterestName = interestName;
  oldInterestName.appendNumber(oldSeqNo);

  lsdb.expressInterest(oldInterestName, 0, 0);
  advanceClocks(10_ms);

  std::vector<ndn::Interest>& interests = face.sentInterests;

  BOOST_REQUIRE(interests.size() > 0);

  bool didFindInterest = false;
  for (const auto& interest : interests) {
    didFindInterest = didFindInterest || interest.getName() == oldInterestName;
  }

  BOOST_CHECK(didFindInterest);
  interests.clear();

  auto deadline = ndn::time::steady_clock::now() + ndn::time::seconds(LSA_REFRESH_TIME_MAX);

  // Simulate an LSA interest timeout
  lsdb.onFetchLsaError(ndn::SegmentFetcher::ErrorCode::INTEREST_TIMEOUT, "Timeout",
                       oldInterestName, 0, deadline, interestName, oldSeqNo);
  advanceClocks(10_ms);

  BOOST_REQUIRE(interests.size() > 0);

  didFindInterest = false;
  for (const auto& interest : interests) {
    didFindInterest = didFindInterest || interest.getName() == oldInterestName;
  }

  BOOST_CHECK(didFindInterest);
  interests.clear();

  uint64_t newSeqNo = 83;

  ndn::Name newInterestName = interestName;
  newInterestName.appendNumber(newSeqNo);

  lsdb.expressInterest(newInterestName, 0, 0);
  advanceClocks(10_ms);

  BOOST_REQUIRE(interests.size() > 0);

  didFindInterest = false;
  for (const auto& interest : interests) {
    didFindInterest = didFindInterest || interest.getName() == newInterestName;
  }

  BOOST_CHECK(didFindInterest);

  interests.clear();

  // Simulate an LSA interest timeout where the sequence number is outdated
  lsdb.onFetchLsaError(ndn::SegmentFetcher::ErrorCode::INTEREST_TIMEOUT, "Timeout",
                       oldInterestName, 0, deadline, interestName, oldSeqNo);
  advanceClocks(10_ms);

  // Interest should not be expressed for outdated sequence number
  BOOST_CHECK_EQUAL(interests.size(), 0);
}

BOOST_AUTO_TEST_CASE(LsdbSegmentedData)
{
  // Add a lot of NameLSAs to exceed max packet size
  ndn::Name originRouter("/ndn/site/%C1.Router/this-router");

  auto nameLsa = lsdb.findLsa<NameLsa>(originRouter);
  BOOST_REQUIRE(nameLsa != nullptr);
  uint64_t seqNo = nameLsa->getSeqNo();

  ndn::Name prefix("/ndn/edu/memphis/netlab/research/nlsr/test/prefix/");

  int nPrefixes = 0;
  while (nameLsa->wireEncode().size() < ndn::MAX_NDN_PACKET_SIZE) {
    nameLsa->addName(PrefixInfo(ndn::Name(prefix).appendNumber(++nPrefixes), 0));
    break;
  }
  lsdb.installLsa(nameLsa);

  // Create another Lsdb and expressInterest
  ndn::DummyClientFace face2(m_io, m_keyChain, {true, true});
  face.linkTo(face2);

  ConfParameter conf2(face2, m_keyChain);
  std::string config = R"CONF(
              trust-anchor
                {
                  type any
                }
            )CONF";
  conf2.getValidator().load(config, "config-file-from-string");

  Lsdb lsdb2(face2, m_keyChain, conf2);

  advanceClocks(ndn::time::milliseconds(10), 10);

  ndn::Name interestName("/localhop/ndn/nlsr/LSA/site/%C1.Router/this-router/NAME");
  interestName.appendNumber(seqNo);
  lsdb2.expressInterest(interestName, 0/*= timeout count*/, 0);

  advanceClocks(ndn::time::milliseconds(200), 20);

  isFirstNameLsaEqual(lsdb2);
}

BOOST_AUTO_TEST_CASE(SegmentLsaData)
{
  ndn::Name originRouter("/ndn/site/%C1.Router/this-router");

  auto lsa = lsdb.findLsa<NameLsa>(originRouter);
  uint64_t seqNo = lsa->getSeqNo();

  ndn::Name prefix("/ndn/edu/memphis/netlab/research/nlsr/test/prefix/");

  int nPrefixes = 0;
  while (lsa->wireEncode().size() < ndn::MAX_NDN_PACKET_SIZE) {
    lsa->addName(PrefixInfo(ndn::Name(prefix).appendNumber(++nPrefixes), 0));
  }
  lsdb.installLsa(lsa);

  ndn::Block expectedDataContent = lsa->wireEncode();

  ndn::Name interestName("/localhop/ndn/nlsr/LSA/site/%C1.Router/this-router/NAME/");
  interestName.appendNumber(seqNo);

  ndn::DummyClientFace face2(m_io, m_keyChain, {true, true});
  face.linkTo(face2);

  auto fetcher = ndn::SegmentFetcher::start(face2, ndn::Interest(interestName),
                                            ndn::security::getAcceptAllValidator());
  fetcher->onComplete.connect([&expectedDataContent] (ndn::ConstBufferPtr bufferPtr) {
                                ndn::Block block(bufferPtr);
                                BOOST_CHECK_EQUAL(expectedDataContent, block);
                              });

  advanceClocks(ndn::time::milliseconds(1), 100);
  fetcher->stop();
}

BOOST_AUTO_TEST_CASE(ReceiveSegmentedLsaData)
{
  ndn::Name router("/ndn/cs/%C1.Router/router1");
  uint64_t seqNo = 12;
  NamePrefixList prefixList;

  NameLsa lsa(router, seqNo, ndn::time::system_clock::now(), prefixList);

  ndn::Name prefix("/prefix/");

  for (int nPrefixes = 0; nPrefixes < 3; ++nPrefixes) {
    lsa.addName(PrefixInfo(ndn::Name(prefix).appendNumber(nPrefixes), 0));
  }

  ndn::Name interestName("/localhop/ndn/nlsr/LSA/cs/%C1.Router/router1/NAME/");
  interestName.appendNumber(seqNo);

  ndn::Block block = lsa.wireEncode();
  lsdb.afterFetchLsa(block.getBuffer(), interestName);

  auto foundLsa = std::static_pointer_cast<NameLsa>(lsdb.findLsa(lsa.getOriginRouter(), lsa.getType()));
  BOOST_REQUIRE(foundLsa != nullptr);

  BOOST_CHECK_EQUAL(foundLsa->wireEncode(), lsa.wireEncode());
}

BOOST_AUTO_TEST_CASE(LsdbRemoveAndExists)
{
  auto testTimePoint = ndn::time::system_clock::now();
  NamePrefixList npl1;

  std::string s1 = "name1";
  std::string s2 = "name2";
  std::string router1 = "/router1/1";

  npl1.insert(s1);
  npl1.insert(s2);

  // For NameLsa lsType is name.
  // 12 is seqNo, randomly generated.
  // 1800 seconds is the default life time.
  NameLsa nlsa1(router1, 12, testTimePoint, npl1);

  Lsdb& lsdb1(lsdb);

  lsdb1.installLsa(std::make_shared<NameLsa>(nlsa1));

  BOOST_CHECK(lsdb1.doesLsaExist(router1, Lsa::Type::NAME));

  lsdb1.removeLsa(router1, Lsa::Type::NAME);

  BOOST_CHECK_EQUAL(lsdb1.doesLsaExist(router1, Lsa::Type::NAME), false);
}

BOOST_AUTO_TEST_CASE(InstallNameLsa)
{
  // Install lsa with name1 and name2
  ndn::Name name1("/ndn/name1");
  ndn::Name name2("/ndn/name2");

  NamePrefixList prefixes;
  prefixes.insert(name1);
  prefixes.insert(name2);

  std::string otherRouter("/ndn/site/%C1.router/other-router");
  const auto MAX_TIME = ndn::time::system_clock::time_point::max();

  NameLsa lsa(otherRouter, 1, MAX_TIME, prefixes);
  lsdb.installLsa(std::make_shared<NameLsa>(lsa));

  BOOST_REQUIRE_EQUAL(lsdb.doesLsaExist(otherRouter, Lsa::Type::NAME), true);
  NamePrefixList& nameList = std::static_pointer_cast<NameLsa>(lsdb.findLsa(otherRouter, Lsa::Type::NAME))->getNpl();

  BOOST_CHECK_EQUAL(nameList, prefixes);

  // Add a prefix: name3
  ndn::Name name3("/ndn/name3");
  prefixes.insert(name3);

  NameLsa addLsa(otherRouter, 2, MAX_TIME, prefixes);
  lsdb.installLsa(std::make_shared<NameLsa>(addLsa));

  // Lsa should include name1, name2, and name3
  BOOST_CHECK_EQUAL(nameList, prefixes);

  // Remove a prefix: name2
  prefixes.erase(name2);

  NameLsa removeLsa(otherRouter, 3, MAX_TIME, prefixes);
  lsdb.installLsa(std::make_shared<NameLsa>(removeLsa));

  // Lsa should include name1 and name3
  BOOST_CHECK_EQUAL(nameList, prefixes);

  // Add and remove a prefix: add name2, remove name3
  prefixes.insert(name2);
  prefixes.erase(name3);

  NameLsa addAndRemoveLsa(otherRouter, 4, MAX_TIME, prefixes);
  lsdb.installLsa(std::make_shared<NameLsa>(addAndRemoveLsa));

  // Lsa should include name1 and name2
  BOOST_CHECK_EQUAL(nameList, prefixes);

  // Install a completely new list of prefixes
  ndn::Name name4("/ndn/name4");
  ndn::Name name5("/ndn/name5");

  NamePrefixList newPrefixes;
  newPrefixes.insert(name4);
  newPrefixes.insert(name5);

  NameLsa newLsa(otherRouter, 5, MAX_TIME, newPrefixes);
  lsdb.installLsa(std::make_shared<NameLsa>(newLsa));

  // Lsa should include name4 and name5
  BOOST_CHECK_EQUAL(nameList, newPrefixes);
}

BOOST_AUTO_TEST_CASE(TestIsLsaNew)
{
  ndn::Name originRouter("/ndn/memphis/%C1.Router/other-router");

  // Install Name LSA
  NamePrefixList nameList;
  NameLsa lsa(originRouter, 999, ndn::time::system_clock::time_point::max(), nameList);

  lsdb.installLsa(std::make_shared<NameLsa>(lsa));

  // Lower NameLSA sequence number
  uint64_t lowerSeqNo = 998;
  BOOST_CHECK(!lsdb.isLsaNew(originRouter, Lsa::Type::NAME, lowerSeqNo));

  // Same NameLSA sequence number
  uint64_t sameSeqNo = 999;
  BOOST_CHECK(!lsdb.isLsaNew(originRouter, Lsa::Type::NAME, sameSeqNo));

  // Higher NameLSA sequence number
  uint64_t higherSeqNo = 1000;
  BOOST_CHECK(lsdb.isLsaNew(originRouter, Lsa::Type::NAME, higherSeqNo));
}

BOOST_AUTO_TEST_CASE(LsdbSignals)
{
  connectSignal();
  auto testTimePoint = ndn::time::system_clock::now() + 3600_s;
  ndn::Name router2("/router2");
  AdjLsa adjLsa(router2, 12, testTimePoint, conf.getAdjacencyList());
  std::shared_ptr<Lsa> lsaPtr = std::make_shared<AdjLsa>(adjLsa);
  lsdb.installLsa(lsaPtr);
  checkSignalResult(LsdbUpdate::INSTALLED, lsaPtr, {}, {});

  adjLsa.setSeqNo(13);
  updateHappened = false;
  lsaPtr = std::make_shared<AdjLsa>(adjLsa);
  lsdb.installLsa(lsaPtr);
  BOOST_CHECK(!updateHappened);

  Adjacent adj("Neighbor1");
  adjLsa.setSeqNo(14);
  adjLsa.addAdjacent(adj);
  lsaPtr = std::make_shared<AdjLsa>(adjLsa);
  lsdb.installLsa(lsaPtr);
  checkSignalResult(LsdbUpdate::UPDATED, lsaPtr, {}, {});

  lsdb.removeLsa(lsaPtrCheck->getOriginRouter(), Lsa::Type::ADJACENCY);
  checkSignalResult(LsdbUpdate::REMOVED, lsaPtr, {}, {});

  // Name LSA
  NamePrefixList npl1{"name1", "name2"};
  NameLsa nameLsa(router2, 12, testTimePoint, npl1);
  lsaPtr = std::make_shared<NameLsa>(nameLsa);
  lsdb.installLsa(lsaPtr);
  checkSignalResult(LsdbUpdate::INSTALLED, lsaPtr, {}, {});

  nameLsa.setSeqNo(13);
  lsaPtr = std::make_shared<NameLsa>(nameLsa);
  lsdb.installLsa(lsaPtr);
  BOOST_CHECK(!updateHappened);

  NamePrefixList npl2{"name2", "name3"};
  NameLsa nameLsa2(router2, 14, testTimePoint, npl2);
  lsaPtr = std::make_shared<NameLsa>(nameLsa2);
  lsdb.installLsa(lsaPtr);
  checkSignalResult(LsdbUpdate::UPDATED, lsaPtr, {PrefixInfo(ndn::Name("/name3"), 0)}, {PrefixInfo(ndn::Name("/name1"), 0)});

  lsdb.removeLsa(lsaPtrCheck->getOriginRouter(), Lsa::Type::NAME);
  checkSignalResult(LsdbUpdate::REMOVED, lsaPtr, {}, {});

  // Coordinate LSA
  lsaPtr = std::make_shared<CoordinateLsa>(CoordinateLsa("router1", 12, testTimePoint, 2.5, {30}));
  lsdb.installLsa(lsaPtr);
  checkSignalResult(LsdbUpdate::INSTALLED, lsaPtr, {}, {});

  lsaPtr = std::make_shared<CoordinateLsa>(CoordinateLsa("router1", 13, testTimePoint, 2.5, {30}));
  lsdb.installLsa(lsaPtr);
  BOOST_CHECK(!updateHappened);

  lsaPtr = std::make_shared<CoordinateLsa>(CoordinateLsa("router1", 14, testTimePoint, 2.5, {40}));
  lsdb.installLsa(lsaPtr);
  checkSignalResult(LsdbUpdate::UPDATED, lsaPtr, {}, {});

  lsdb.removeLsa(lsaPtrCheck->getOriginRouter(), Lsa::Type::COORDINATE);
  checkSignalResult(LsdbUpdate::REMOVED, lsaPtr, {}, {});
}

BOOST_AUTO_TEST_SUITE_END() // TestLsdb

} // namespace nlsr::tests
