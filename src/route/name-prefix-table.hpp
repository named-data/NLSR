/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#ifndef NLSR_NAME_PREFIX_TABLE_HPP
#define NLSR_NAME_PREFIX_TABLE_HPP

#include <list>
#include <boost/cstdint.hpp>

#include "name-prefix-table-entry.hpp"
#include "routing-table-entry.hpp"

namespace nlsr {
class Nlsr;

class NamePrefixTable
{
public:
  NamePrefixTable(Nlsr& nlsr)
    : m_nlsr(nlsr)
  {
  }

  void
  addEntry(const ndn::Name& name, const ndn::Name& destRouter);

  void
  removeEntry(const ndn::Name& name, const ndn::Name& destRouter);

  void
  updateWithNewRoute();

  void
  writeLog();

private:
  void
  addEntry(const ndn::Name& name, RoutingTableEntry& rte);

  void
  removeEntry(const ndn::Name& name, RoutingTableEntry& rte);

private:
  Nlsr& m_nlsr;
  std::list<NamePrefixTableEntry> m_table;
};

}//namespace nlsr

#endif //NLSR_NAME_PREFIX_TABLE_HPP
