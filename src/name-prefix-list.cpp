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

#include "name-prefix-list.hpp"
#include "common.hpp"

namespace nlsr {

NamePrefixList::NamePrefixList() = default;

NamePrefixList::NamePrefixList(const std::initializer_list<ndn::Name>& names)
{
  std::vector<NamePrefixList::NamePair> namePairs;
  std::transform(names.begin(), names.end(), std::back_inserter(namePairs),
    [] (const ndn::Name& name) {
      return NamePrefixList::NamePair{name, {""}};
    });
  m_names = std::move(namePairs);
}

NamePrefixList::NamePrefixList(const std::initializer_list<NamePrefixList::NamePair>& namesAndSources)
  : m_names(namesAndSources)
{
}

NamePrefixList::~NamePrefixList()
{
}

std::vector<NamePrefixList::NamePair>::iterator
NamePrefixList::get(const ndn::Name& name)
{
  return std::find_if(m_names.begin(), m_names.end(),
                      [&] (const NamePrefixList::NamePair& pair) {
                        return name == std::get<NamePrefixList::NamePairIndex::NAME>(pair);
                      });
}

std::vector<std::string>::iterator
NamePrefixList::getSource(const std::string& source, std::vector<NamePair>::iterator& entry)
{
  return std::find_if(std::get<NamePairIndex::SOURCES>(*entry).begin(),
                      std::get<NamePairIndex::SOURCES>(*entry).end(),
                      [&] (const std::string& containerSource) {
                        return source == containerSource;
                      });
}

bool
NamePrefixList::insert(const ndn::Name& name, const std::string& source)
{
  auto pairItr = get(name);
  if (pairItr == m_names.end()) {
    std::vector<std::string> sources{source};
    m_names.push_back(std::tie(name, sources));
    return true;
  }
  else {
    std::vector<std::string>& sources = std::get<NamePrefixList::NamePairIndex::SOURCES>(*pairItr);
    auto sourceItr = getSource(source, pairItr);
    if (sourceItr == sources.end()) {
      sources.push_back(source);
      return true;
    }
  }
  return false;
}

bool
NamePrefixList::remove(const ndn::Name& name, const std::string& source)
{
  auto pairItr = get(name);
  if (pairItr != m_names.end()) {
    std::vector<std::string>& sources = std::get<NamePrefixList::NamePairIndex::SOURCES>(*pairItr);
    auto sourceItr = getSource(source, pairItr);
    if (sourceItr != sources.end()) {
      sources.erase(sourceItr);
      if (sources.size() == 0) {
        m_names.erase(pairItr);
      }
      return true;
    }
  }
  return false;
}

bool
NamePrefixList::operator==(const NamePrefixList& other) const
{
  return m_names == other.m_names;
}

void
NamePrefixList::sort()
{
  std::sort(m_names.begin(), m_names.end());
}

std::list<ndn::Name>
NamePrefixList::getNames() const
{
  std::list<ndn::Name> names;
  for (const auto& namePair : m_names) {
    names.push_back(std::get<NamePrefixList::NamePairIndex::NAME>(namePair));
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
  auto it = std::find_if(m_names.begin(), m_names.end(),
                         [&] (const NamePrefixList::NamePair& pair) {
                           return name == std::get<NamePrefixList::NamePairIndex::NAME>(pair);
                         });
  if (it != m_names.end()) {
    return std::get<NamePrefixList::NamePairIndex::SOURCES>(*it);
  }
  else {
    return std::vector<std::string>{};
  }
}

std::ostream&
operator<<(std::ostream& os, const NamePrefixList& list) {
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
