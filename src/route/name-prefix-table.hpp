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

#ifndef NLSR_NAME_PREFIX_TABLE_HPP
#define NLSR_NAME_PREFIX_TABLE_HPP

#include "name-prefix-table-entry.hpp"
#include "routing-table-entry.hpp"

#include <list>

namespace nlsr {
class Nlsr;

class NamePrefixTable
{
public:

  typedef std::list<NamePrefixTableEntry> NptEntryList;
  typedef NptEntryList::const_iterator const_iterator;

  NamePrefixTable(Nlsr& nlsr)
    : m_nlsr(nlsr)
  {
  }

  /*! \brief Adds an entry to the NPT.
    \param name The name prefix that is being advertised.
    \param destRouter The destination router that is advertising the prefix.

    Adds a destination router to the name prefix. If there isn't
    currently an entry present for the prefix, one is made and
    added. If there is no routing table entry available for the
    destination router, a placeholder routing table entry without next
    hops will be made. Since this function can be called when a
    routing table entry has been updated to have no hops (i.e. is
    unreachable), this function will check for the number of next hops
    an entry has. If it is found to have no next hops, the NPT will
    inform the FIB to remove that prefix.
  */
  void
  addEntry(const ndn::Name& name, const ndn::Name& destRouter);

  void
  removeEntry(const ndn::Name& name, const ndn::Name& destRouter);

  void
  updateWithNewRoute();

  void
  writeLog();

  const_iterator
  begin() const;

  const_iterator
  end() const;

private:
  /*! \brief Adds an entry to the NPT table.
    \param name The name prefix to add to the table.
    \param rte The routing table entry with the next hop information to
    reach the prefix.
  */
  void
  addEntry(const ndn::Name& name, RoutingTableEntry& rte);

  void
  removeEntry(const ndn::Name& name, RoutingTableEntry& rte);

private:
  Nlsr& m_nlsr;
  std::list<NamePrefixTableEntry> m_table;
};

inline NamePrefixTable::const_iterator
NamePrefixTable::begin() const
{
  return m_table.begin();
}

inline NamePrefixTable::const_iterator
NamePrefixTable::end() const
{
  return m_table.end();
}

std::ostream&
operator<<(std::ostream& os, const NamePrefixTable& table);

} // namespace nlsr

#endif // NLSR_NAME_PREFIX_TABLE_HPP
