/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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

#ifndef NLSR_NAME_PREFIX_TABLE_ENTRY_HPP
#define NLSR_NAME_PREFIX_TABLE_ENTRY_HPP

#include "routing-table-entry.hpp"

#include <list>
#include <utility>

namespace nlsr {

class NamePrefixTableEntry
{
public:
  NamePrefixTableEntry()
  {
  }

  NamePrefixTableEntry(const ndn::Name& namePrefix)
    : m_namePrefix(namePrefix)
    , m_nexthopList()
  {
  }

  const ndn::Name&
  getNamePrefix() const
  {
    return m_namePrefix;
  }

  const std::list<RoutingTableEntry>&
  getRteList() const
  {
    return m_rteList;
  }

  void
  resetRteListNextHop()
  {
    if (m_rteList.size() > 0) {
      for (std::list<RoutingTableEntry>::iterator it = m_rteList.begin();
           it != m_rteList.end(); ++it) {
        (*it).getNexthopList().reset();
      }
    }
  }

  size_t
  getRteListSize()
  {
    return m_rteList.size();
  }

  NexthopList&
  getNexthopList()
  {
    return m_nexthopList;
  }

  void
  generateNhlfromRteList();

  void
  removeRoutingTableEntry(RoutingTableEntry& rte);

  void
  addRoutingTableEntry(RoutingTableEntry& rte);

  void
  writeLog();

private:
  ndn::Name m_namePrefix;
  std::list<RoutingTableEntry> m_rteList;
  NexthopList m_nexthopList;
};

std::ostream&
operator<<(std::ostream& os, const NamePrefixTableEntry& entry);

} // namespace nlsr

#endif // NLSR_NAME_PREFIX_TABLE_ENTRY_HPP
