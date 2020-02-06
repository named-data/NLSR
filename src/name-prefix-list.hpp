/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2020,  The University of Memphis,
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

#ifndef NLSR_NAME_PREFIX_LIST_HPP
#define NLSR_NAME_PREFIX_LIST_HPP

#include "test-access-control.hpp"

#include <list>
#include <string>
#include <ndn-cxx/name.hpp>

namespace nlsr {

class NamePrefixList
{
public:
  using NamePair = std::tuple<ndn::Name, std::vector<std::string>>;
  enum NamePairIndex {
    NAME,
    SOURCES
  };

  NamePrefixList();

  NamePrefixList(const std::initializer_list<ndn::Name>& names);

  NamePrefixList(const std::initializer_list<NamePrefixList::NamePair>& namesAndSources);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  template<class ContainerType>
  NamePrefixList(const ContainerType& names)
  {
    for (const auto& elem : names) {
      m_names.push_back(NamePair{elem, {""}});
    }
  }

public:
  ~NamePrefixList();

  /*! \brief inserts name into NamePrefixList
      \retval true If the name was successfully inserted.
      \retval false If the name could not be inserted.
   */
  bool
  insert(const ndn::Name& name, const std::string& source = "");

  /*! \brief removes name from NamePrefixList
      \retval true If the name is removed
      \retval false If the name failed to be removed.
   */
  bool
  remove(const ndn::Name& name, const std::string& source = "");

  void
  sort();

  size_t
  size() const
  {
    return m_names.size();
  }

  std::list<ndn::Name>
  getNames() const;

  bool
  operator==(const NamePrefixList& other) const;

  /*! Returns how many unique sources this name has.

    \retval 0 if the name is not in the list, else the number of sources.
   */
  uint32_t
  countSources(const ndn::Name& name) const;

  /*! Returns the sources that this name has.

    \retval an empty vector if the name is not in the list, else a
    vector containing the sources.
   */
  const std::vector<std::string>
  getSources(const ndn::Name& name) const;

  void
  clear()
  {
    m_names.clear();
  }

private:
  /*! Obtain an iterator to the entry matching name.

    \note We could do this quite easily inline with a lambda, but this
    is slightly more efficient.
   */
  std::vector<NamePair>::iterator
  get(const ndn::Name& name);

  /*! Obtain an iterator to a specific source in an entry
   */
  std::vector<std::string>::iterator
  getSource(const std::string& source, std::vector<NamePair>::iterator& entry);

  std::vector<NamePair> m_names;
};

std::ostream&
operator<<(std::ostream& os, const NamePrefixList& list);

} // namespace nlsr

#endif // NLSR_NAME_PREFIX_LIST_HPP
