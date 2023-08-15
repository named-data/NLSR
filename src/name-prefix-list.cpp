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

#include "name-prefix-list.hpp"
#include "common.hpp"

namespace nlsr {

NamePrefixList::NamePrefixList() = default;

NamePrefixList::NamePrefixList(ndn::span<const ndn::Name> names)
{
  for (const auto& name : names) {
    insert(name);
  }
}

bool
NamePrefixList::insert(const ndn::Name& name, const std::string& source)
{
  auto& sources = m_namesSources[name];
  return sources.insert(source).second;
}

bool
NamePrefixList::erase(const ndn::Name& name, const std::string& source)
{
  auto it = m_namesSources.find(name);
  if (it == m_namesSources.end()) {
    return false;
  }

  bool isRemoved = it->second.erase(source);
  if (it->second.empty()) {
    m_namesSources.erase(it);
  }
  return isRemoved;
}

std::list<ndn::Name>
NamePrefixList::getNames() const
{
  std::list<ndn::Name> names;
  for (const auto& [name, sources] : m_namesSources) {
    names.push_back(name);
  }
  return names;
}

uint32_t
NamePrefixList::countSources(const ndn::Name& name) const
{
  return getSources(name).size();
}

const std::vector<std::string>
NamePrefixList::getSources(const ndn::Name& name) const
{
  auto it = m_namesSources.find(name);
  if (it == m_namesSources.end()) {
    return {};
  }

  std::vector<std::string> result;
  for (const auto& source : it->second) {
    result.push_back(source);
  }
  return result;
}

std::ostream&
operator<<(std::ostream& os, const NamePrefixList& list)
{
  os << "Name prefix list: {\n";
  for (const auto& name : list.getNames()) {
    os << name << "\n"
       << "Sources:\n";
    for (const auto& source : list.getSources(name)) {
      os << "  " << source << "\n";
    }
  }
  os << "}" << std::endl;
  return os;
}

} // namespace nlsr
