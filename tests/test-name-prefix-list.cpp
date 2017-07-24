/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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
#include <boost/test/unit_test.hpp>

namespace nlsr {
namespace test {

BOOST_AUTO_TEST_SUITE(TestNpl)

/*
  The NamePrefixList can have names inserted and removed from it.
 */
BOOST_AUTO_TEST_CASE(NplSizeAndRemove)
{
  NamePrefixList npl1;

  std::string a = "testname";
  std::string b = "name";

  npl1.insert(a);
  npl1.insert(b);

  BOOST_CHECK_EQUAL(npl1.getSize(), 2);

  npl1.remove(b);

  BOOST_CHECK_EQUAL(npl1.getSize(), 1);
}

/*
  Two NamePrefixLists will be considered equal if they contain the
  same names. Sources for names are ignored.
 */
BOOST_AUTO_TEST_CASE(OperatorEquals)
{
  NamePrefixList list1;
  NamePrefixList list2;
  ndn::Name name1("/ndn/test/name1");
  ndn::Name name2("/ndn/test/name2");
  ndn::Name name3("/ndn/some/other/name1");

  list1.insert(name1);
  list1.insert(name2);
  list1.insert(name3);

  list2.insert(name1);
  list2.insert(name2);
  list2.insert(name3);

  BOOST_CHECK_EQUAL(list1, list2);
}

/*
  The NamePrefixList will provide a container of all the names it has,
  without the sources for those names.
 */
BOOST_AUTO_TEST_CASE(GetNames)
{
  NamePrefixList list;
  const ndn::Name name1{"/ndn/test/prefix1"};
  const ndn::Name name2{"/ndn/test/prefix2"};
  const ndn::Name name3{"/ndn/test/prefix3"};
  list.insert(name1);
  list.insert(name2);
  list.insert(name3);

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
  NamePrefixList list;
  const ndn::Name name1{"/ndn/test/prefix1"};
  const ndn::Name invalidName{"/not/a/prefix"};
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

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
