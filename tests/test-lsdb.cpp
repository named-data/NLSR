/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 * \author Ashlesh Gawande <agawande@memphis.edu>
 *
 **/

#include "test-common.hpp"
#include "dummy-face.hpp"

#include "lsdb.hpp"
#include "nlsr.hpp"
#include "lsa.hpp"
#include "name-prefix-list.hpp"
#include <boost/test/unit_test.hpp>
#include <ndn-cxx/util/time.hpp>

namespace nlsr {
namespace test {

using ndn::DummyFace;
using ndn::shared_ptr;

class LsdbFixture : public BaseFixture
{
public:
  LsdbFixture()
    : face(ndn::makeDummyFace())
    , nlsr(g_ioService, g_scheduler, ndn::ref(*face))
    , sync(*face, nlsr.getLsdb(), nlsr.getConfParameter())
    , lsdb(nlsr.getLsdb())
    , conf(nlsr.getConfParameter())
    , REGISTER_COMMAND_PREFIX("/localhost/nfd/rib")
    , REGISTER_VERB("register")
  {
    conf.setNetwork("/ndn");
    conf.setSiteName("/site");
    conf.setRouterName("/%C1.router/this-router");

    nlsr.initialize();

    face->processEvents(ndn::time::milliseconds(1));
    face->m_sentInterests.clear();
  }

  void
  extractParameters(ndn::Interest& interest, ndn::Name::Component& verb,
                    ndn::nfd::ControlParameters& extractedParameters)
  {
    const ndn::Name& name = interest.getName();
    verb = name[REGISTER_COMMAND_PREFIX.size()];
    const ndn::Name::Component& parameterComponent = name[REGISTER_COMMAND_PREFIX.size() + 1];

    ndn::Block rawParameters = parameterComponent.blockFromValue();
    extractedParameters.wireDecode(rawParameters);
  }

  void
  areNamePrefixListsEqual(NamePrefixList& lhs, NamePrefixList& rhs)
  {
    typedef std::list<ndn::Name> NameList;

    NameList& lhsList = lhs.getNameList();
    NameList& rhsList = rhs.getNameList();

    BOOST_REQUIRE_EQUAL(lhsList.size(), rhsList.size());

    NameList::iterator i = lhsList.begin();
    NameList::iterator j = rhsList.begin();

    for (; i != lhsList.end(); ++i, ++j) {
      BOOST_CHECK_EQUAL(*i, *j);
    }
  }

public:
  shared_ptr<DummyFace> face;
  Nlsr nlsr;
  SyncLogicHandler sync;

  Lsdb& lsdb;
  ConfParameter& conf;

  ndn::Name REGISTER_COMMAND_PREFIX;
  ndn::Name::Component REGISTER_VERB;
};

BOOST_FIXTURE_TEST_SUITE(TestLsdb, LsdbFixture)

BOOST_AUTO_TEST_CASE(LsdbRemoveAndExists)
{
  INIT_LOGGERS("/tmp", "DEBUG");

  ndn::time::system_clock::TimePoint testTimePoint =  ndn::time::system_clock::now();
  NamePrefixList npl1;

  std::string s1 = "name1";
  std::string s2 = "name2";
  std::string router1 = "router1/1";

  npl1.insert(s1);
  npl1.insert(s2);

  //For NameLsa lsType is name.
  //12 is seqNo, randomly generated.
  //1800 is the default life time.
  NameLsa nlsa1(ndn::Name("/router1/1"), std::string("name"), 12, testTimePoint, npl1);

  Lsdb lsdb1(nlsr, g_scheduler);

  lsdb1.installNameLsa(nlsa1);
  lsdb1.writeNameLsdbLog();

  BOOST_CHECK(lsdb1.doesLsaExist(ndn::Name("/router1/1/name"), std::string("name")));

  lsdb1.removeNameLsa(router1);

  BOOST_CHECK_EQUAL(lsdb1.doesLsaExist(ndn::Name("/router1/1"), std::string("name")), false);
}

BOOST_AUTO_TEST_CASE(RegisterSyncPrefixOnFirstAdjLsaBuild)
{
  // Should register Sync prefix
  lsdb.buildAndInstallOwnAdjLsa();
  face->processEvents(ndn::time::milliseconds(1));

  std::vector<ndn::Interest>& interests = face->m_sentInterests;

  BOOST_REQUIRE(interests.size() > 0);

  ndn::nfd::ControlParameters extractedParameters;
  ndn::Name::Component verb;
  extractParameters(interests[0], verb, extractedParameters);

  BOOST_CHECK_EQUAL(verb, REGISTER_VERB);
  BOOST_CHECK_EQUAL(extractedParameters.getName(), conf.getChronosyncPrefix());

  // Should not register Sync prefix
  face->m_sentInterests.clear();
  lsdb.buildAndInstallOwnAdjLsa();
  face->processEvents(ndn::time::milliseconds(1));

  BOOST_CHECK_EQUAL(interests.size(), 0);
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
  ndn::time::system_clock::TimePoint MAX_TIME = ndn::time::system_clock::TimePoint::max();

  NameLsa lsa(otherRouter, "name", 1, MAX_TIME, prefixes);
  lsdb.installNameLsa(lsa);

  BOOST_REQUIRE_EQUAL(lsdb.doesLsaExist(otherRouter + "/name", "name"), true);
  NamePrefixList& nameList = lsdb.findNameLsa(otherRouter + "/name")->getNpl();

  areNamePrefixListsEqual(nameList, prefixes);

  // Add a prefix: name3
  ndn::Name name3("/ndn/name3");
  prefixes.insert(name3);

  NameLsa addLsa(otherRouter, "name", 2, MAX_TIME, prefixes);
  lsdb.installNameLsa(addLsa);

  // Lsa should include name1, name2, and name3
  areNamePrefixListsEqual(nameList, prefixes);

  // Remove a prefix: name2
  prefixes.remove(name2);

  NameLsa removeLsa(otherRouter, "name", 3, MAX_TIME, prefixes);
  lsdb.installNameLsa(removeLsa);

  // Lsa should include name1 and name3
  areNamePrefixListsEqual(nameList, prefixes);

  // Add and remove a prefix: add name2, remove name3
  prefixes.insert(name2);
  prefixes.remove(name3);

  NameLsa addAndRemoveLsa(otherRouter, "name", 4, MAX_TIME, prefixes);
  lsdb.installNameLsa(addAndRemoveLsa);

  // Lsa should include name1 and name2
  areNamePrefixListsEqual(nameList, prefixes);

  // Install a completely new list of prefixes
  ndn::Name name4("/ndn/name4");
  ndn::Name name5("/ndn/name5");

  NamePrefixList newPrefixes;
  newPrefixes.insert(name4);
  newPrefixes.insert(name5);

  NameLsa newLsa(otherRouter, "name", 5, MAX_TIME, newPrefixes);
  lsdb.installNameLsa(newLsa);

  // Lsa should include name4 and name5
  areNamePrefixListsEqual(nameList, newPrefixes);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
