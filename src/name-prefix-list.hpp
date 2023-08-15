/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023,  The University of Memphis,
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

#ifndef NLSR_NAME_PREFIX_LIST_HPP
#define NLSR_NAME_PREFIX_LIST_HPP

#include "test-access-control.hpp"

#include <ndn-cxx/name.hpp>

#include <boost/operators.hpp>

#include <initializer_list>
#include <list>
#include <map>
#include <set>
#include <string>

namespace nlsr {

class NamePrefixList : private boost::equality_comparable<NamePrefixList>
{
public:
  NamePrefixList();

  explicit
  NamePrefixList(std::initializer_list<ndn::Name> names);

  /*! \brief Inserts name and source combination.
      \retval true Name and source combination is inserted.
      \retval false Name and source combination already exists.
   */
  bool
  insert(const ndn::Name& name, const std::string& source = "");

  /*! \brief Deletes name and source combination
      \retval true Name and source combination is deleted.
      \retval false Name and source combination does not exist.
   */
  bool
  erase(const ndn::Name& name, const std::string& source = "");

  size_t
  size() const
  {
    return m_namesSources.size();
  }

  std::list<ndn::Name>
  getNames() const;

#ifdef WITH_TESTS
  /*! Returns the sources that this name has.
      If the name does not exist, returns an empty container.
   */
  std::set<std::string>
  getSources(const ndn::Name& name) const;
#endif

  void
  clear()
  {
    m_namesSources.clear();
  }

private: // non-member operators
  // NOTE: the following "hidden friend" operators are available via
  //       argument-dependent lookup only and must be defined inline.
  // boost::equality_comparable provides != operators.

  friend bool
  operator==(const NamePrefixList& lhs, const NamePrefixList& rhs)
  {
    return lhs.getNames() == rhs.getNames();
  }

private:
  std::map<ndn::Name, std::set<std::string>> m_namesSources;

  friend std::ostream&
  operator<<(std::ostream& os, const NamePrefixList& list);
};

} // namespace nlsr

#endif // NLSR_NAME_PREFIX_LIST_HPP
