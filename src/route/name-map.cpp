/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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

#include "name-map.hpp"
#include "nlsr.hpp"
#include "adjacent.hpp"
#include "lsa/lsa.hpp"
#include "lsdb.hpp"

namespace nlsr {

void
NameMap::addEntry(const ndn::Name& rtrName)
{
  int32_t mappingNo = static_cast<int32_t>(m_bimap.size());
  m_bimap.by<ndn::Name>().insert({rtrName, mappingNo});
}

std::optional<ndn::Name>
NameMap::getRouterNameByMappingNo(int32_t mn) const
{
  auto it = m_bimap.by<MappingNo>().find(mn);
  if (it == m_bimap.by<MappingNo>().end()) {
    return std::nullopt;
  }
  return it->get<ndn::Name>();
}

std::optional<int32_t>
NameMap::getMappingNoByRouterName(const ndn::Name& rtrName) const
{
  auto it = m_bimap.by<ndn::Name>().find(rtrName);
  if (it == m_bimap.by<ndn::Name>().end()) {
    return std::nullopt;
  }
  return it->get<MappingNo>();
}

std::ostream&
operator<<(std::ostream& os, const NameMap& map)
{
  os << "---------------NameMap---------------";
  for (const auto& entry : map.m_bimap) {
    os << "\nMapEntry: ( Router: " << entry.get<ndn::Name>()
       << " Mapping No: " << entry.get<NameMap::MappingNo>() << " )";
  }
  return os;
}

} // namespace nlsr
