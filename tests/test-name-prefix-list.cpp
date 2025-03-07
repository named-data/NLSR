/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
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
 */

#include "name-prefix-list.hpp"
#include "tests/boost-test.hpp"

namespace nlsr::tests {

BOOST_AUTO_TEST_SUITE(TestNpl)

/*
  The NamePrefixList will provide a container of all the names it has,
  without the sources for those names.

  The NamePrefixList will return a container with all the sources for
  a given name, with an empty container for a non-existent name.
 */
BOOST_AUTO_TEST_CASE(Ctor_Insert_Size_GetNames_GetSources)
{
  ndn::Name name1{"/ndn/test/prefix1"};
  ndn::Name name2{"/ndn/test/prefix2"};
  ndn::Name name3{"/ndn/test/prefix3"};
  std::list<ndn::Name> expectedNames{name1, name2, name3};

  NamePrefixList list1{name1, name2, name3};
  BOOST_CHECK_EQUAL(list1.size(), 3);
  BOOST_TEST(list1.getNames() == expectedNames, boost::test_tools::per_element());

  std::vector<std::string> sources1{"static", "readvertise"};
  std::vector<std::string> sources2{"static", "nlsrc"};
  std::vector<std::string> sources3{"static"};

  NamePrefixList list2;
  auto list2InsertNameSources = [&] (const ndn::Name& name, ndn::span<const std::string> sources) {
    for (const auto& source : sources) {
      list2.insert(name, source);
    }
  };
  list2InsertNameSources(name1, sources1);
  list2InsertNameSources(name2, sources2);
  list2InsertNameSources(name3, sources3);
  BOOST_CHECK_EQUAL(list2.size(), 3);
  BOOST_TEST(list2.getNames() == expectedNames, boost::test_tools::per_element());

  std::sort(sources1.begin(), sources1.end());
  std::sort(sources2.begin(), sources2.end());
  std::sort(sources3.begin(), sources3.end());
  BOOST_TEST(list2.getSources(name1) == sources1, boost::test_tools::per_element());
  BOOST_TEST(list2.getSources(name2) == sources2, boost::test_tools::per_element());
  BOOST_TEST(list2.getSources(name3) == sources3, boost::test_tools::per_element());

  auto noSources = list2.getSources("/not/a/prefix");
  BOOST_CHECK_EQUAL(noSources.size(), 0);
}

/*
  The NamePrefixList can have names inserted and removed from it.

  The NamePrefixList will not delete a name as long as it at least one
  source.
 */
BOOST_AUTO_TEST_CASE(Insert_Erase)
{
  ndn::Name name1{"/ndn/test/prefix1"};
  ndn::Name name2{"/ndn/test/prefix2"};

  NamePrefixList list;
  BOOST_CHECK_EQUAL(list.insert(name2), true);
  BOOST_CHECK_EQUAL(list.size(), 1);
  BOOST_CHECK_EQUAL(list.insert(name2), false);

  list.insert(name1, "nlsr.conf");
  BOOST_CHECK_EQUAL(list.size(), 2);
  list.insert(name1, "readvertise");
  list.insert(name1, "prefix-update");
  BOOST_CHECK_EQUAL(list.size(), 2);
  list.erase(name1, "prefix-update");
  BOOST_CHECK_EQUAL(list.size(), 2);

  BOOST_TEST(list.getSources(name1) == (std::set<std::string>{"nlsr.conf", "readvertise"}),
             boost::test_tools::per_element());

  BOOST_CHECK_EQUAL(list.erase(name2), true);
  BOOST_CHECK_EQUAL(list.erase(name2), false);

  list.erase(name1, "nlsr.conf");
  list.erase(name1, "readvertise");
  BOOST_CHECK_EQUAL(list.size(), 0);
  BOOST_CHECK_EQUAL(list.getSources(name1).size(), 0);
}

/*
  Two NamePrefixLists will be considered equal if they contain the
  same names with the same costs. Sources for names are ignored.
 */
BOOST_AUTO_TEST_CASE(OperatorEquals)
{
  ndn::Name name1("/ndn/test/name1");
  ndn::Name name2("/ndn/test/name2");
  ndn::Name name3("/ndn/some/other/name1");
  NamePrefixList list1{name1, name2, name3};

  NamePrefixList list2;
  BOOST_CHECK_NE(list1, list2);

  list2.insert(name1);
  list2.insert(name1, "A1");
  list2.insert(name2, "B0");
  list2.insert(name2, "B1");
  list2.insert(name3, "C0");
  BOOST_CHECK_EQUAL(list1, list2);

  list2.erase(name3, "C0");
  BOOST_CHECK_NE(list1, list2);

  list2.insert(name3, "C0");
  list1.insert(PrefixInfo(name1, 10));
  list1.insert(PrefixInfo(name2, 20));
  BOOST_CHECK_NE(list1, list2);

  list2.insert(PrefixInfo(name1, 10));
  list2.insert(PrefixInfo(name2, 20));
  BOOST_CHECK_EQUAL(list1, list2);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
