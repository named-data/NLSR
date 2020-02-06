/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2020,  The University of Memphis,
 *                           Regents of the University of California
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

#include "name-prefix-list.hpp"
#include "tests/boost-test.hpp"

namespace nlsr {
namespace test {

BOOST_AUTO_TEST_SUITE(TestNpl)

/*
  The NamePrefixList can have names inserted and removed from it.
 */
BOOST_AUTO_TEST_CASE(NplSizeAndRemove)
{
  ndn::Name a{"testname"};
  ndn::Name b{"name"};

  NamePrefixList npl1{a, b};

  BOOST_CHECK_EQUAL(npl1.size(), 2);

  npl1.remove(b);

  BOOST_CHECK_EQUAL(npl1.size(), 1);
}

/*
  Two NamePrefixLists will be considered equal if they contain the
  same names. Sources for names are ignored.
 */
BOOST_AUTO_TEST_CASE(OperatorEquals)
{
  ndn::Name name1("/ndn/test/name1");
  ndn::Name name2("/ndn/test/name2");
  ndn::Name name3("/ndn/some/other/name1");
  NamePrefixList list1{name1, name2, name3};
  NamePrefixList list2{name1, name2, name3};

  BOOST_CHECK_EQUAL(list1, list2);
}

/*
  The NamePrefixList will provide a container of all the names it has,
  without the sources for those names.
 */
BOOST_AUTO_TEST_CASE(GetNames)
{
  const ndn::Name name1{"/ndn/test/prefix1"};
  const ndn::Name name2{"/ndn/test/prefix2"};
  const ndn::Name name3{"/ndn/test/prefix3"};
  NamePrefixList list{name1, name2, name3};

  std::vector<ndn::Name> referenceNames{name1, name2, name3};

  auto names = list.getNames();
  BOOST_REQUIRE_EQUAL(names.size(), 3);
  // Verify that all of the names are in the list.
  for (const auto& name : names) {
    bool didMatch = false;
    for (const auto& referenceName : referenceNames) {
      didMatch = didMatch || (name == referenceName);
    }
    BOOST_CHECK(didMatch);
  }
}

/*
  The NamePrefixList will count the number of sources for a given
  name, with zero for a non-existent name.
 */
BOOST_AUTO_TEST_CASE(countSources)
{
  const ndn::Name name1{"/ndn/test/prefix1"};
  const ndn::Name invalidName{"/not/a/prefix"};
  NamePrefixList list;
  list.insert(name1, "nlsr.conf");
  list.insert(name1, "readvertise");
  list.insert(name1, "prefix-update");

  BOOST_CHECK_EQUAL(list.countSources(name1), 3);
  BOOST_CHECK_EQUAL(list.countSources(invalidName), 0);
}

/*
  The NamePrefixList will return a container with all the sources for
  a given name, with an empty container for a non-existent name.
 */
BOOST_AUTO_TEST_CASE(getSources)
{
  NamePrefixList list;
  const ndn::Name name1{"/ndn/test/prefix1"};
  const ndn::Name invalidName{"/not/a/prefix"};

  list.insert(name1, "nlsr.conf");
  list.insert(name1, "readvertise");
  list.insert(name1, "prefix-update");
  std::vector<std::string> referenceSources{"nlsr.conf", "readvertise", "prefix-update"};

  const std::vector<std::string> sources = list.getSources(name1);
  BOOST_REQUIRE_EQUAL(list.countSources(name1), 3);
  for (const auto& source : sources) {
    bool didMatch = false;
    for (const auto& referenceSource : referenceSources) {
      didMatch = didMatch || (source == referenceSource);
    }
    BOOST_CHECK(didMatch);
  }

  std::vector<std::string> noSources = list.getSources(invalidName);
  BOOST_REQUIRE_EQUAL(noSources.size(), 0);
}

/*
  The NamePrefixList will not delete a name as long as it at least one
  source.
 */
BOOST_AUTO_TEST_CASE(RemainingSourcesAfterRemoval)
{
  NamePrefixList list;
  const ndn::Name name1{"/ndn/test/prefix1"};
  list.insert(name1, "nlsr.conf");
  list.insert(name1, "readvertise");
  list.insert(name1, "prefix-update");

  list.remove(name1, "prefix-update");

  std::vector<std::string> referenceSources{"nlsr.conf", "readvertise", "prefix-update"};
  const std::vector<std::string> sources = list.getSources(name1);
  BOOST_REQUIRE_EQUAL(list.countSources(name1), 2);
  for (const auto& source : sources) {
    bool didMatch = false;
    for (const auto& referenceSource : referenceSources) {
      didMatch = didMatch || (source == referenceSource);
    }
    BOOST_CHECK(didMatch);
  }
}

BOOST_AUTO_TEST_CASE(BraceInitializerCtors)
{
  const ndn::Name name1{"/ndn/test/prefix1"};
  const ndn::Name name2{"/ndn/test/prefix2"};
  const ndn::Name name3{"/ndn/test/prefix3"};
  std::list<ndn::Name> testList{name1, name2, name3};

  const std::vector<std::string> sources1{"static", "readvertise"};
  const std::vector<std::string> sources2{"static", "nlsrc"};
  const std::vector<std::string> sources3{"static"};

  NamePrefixList list1{name1, name2, name3};
  auto list = list1.getNames();
  BOOST_CHECK_EQUAL(list1.size(), 3);
  BOOST_CHECK(testList == list);

  NamePrefixList list2{ NamePrefixList::NamePair{name1, sources1},
      NamePrefixList::NamePair{name2, sources2}, NamePrefixList::NamePair{name3, sources3} };
  auto name1Sources = list2.getSources(name1);
  BOOST_CHECK(sources1 == name1Sources);
  auto name2Sources = list2.getSources(name2);
  BOOST_CHECK(sources2 == name2Sources);
  auto name3Sources = list2.getSources(name3);
  BOOST_CHECK(sources3 == name3Sources);

  const std::vector<ndn::Name> namesVector{name1, name2, name3};
  NamePrefixList list3(namesVector);
  BOOST_CHECK(list1 == list3);

  const std::list<ndn::Name> namesList{name1, name2, name3};
  NamePrefixList list4(namesList);
  BOOST_CHECK(list1 == list4);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
