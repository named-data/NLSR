/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  The University of Memphis,
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

#ifndef NLSR_ROUTING_TABLE_ENTRY_HPP
#define NLSR_ROUTING_TABLE_ENTRY_HPP

#include "nexthop-list.hpp"

#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/name.hpp>

namespace nlsr {

/*! \brief Data abstraction for RouteTableInfo
 *
 *   RoutingTableEntry := ROUTINGTABLEENTRY-TYPE TLV-LENGTH
 *                          Name
 *                          NexthopList*
 *
 * \sa https://redmine.named-data.net/projects/nlsr/wiki/Routing_Table_DataSet
 */
class RoutingTableEntry
{
public:
  using Error = ndn::tlv::Error;

  RoutingTableEntry() = default;

  RoutingTableEntry(const ndn::Block& block)
  {
    wireDecode(block);
  }

  RoutingTableEntry(const ndn::Name& dest)
  {
    m_destination = dest;
  }

  const ndn::Name&
  getDestination() const
  {
    return m_destination;
  }

  NexthopList&
  getNexthopList()
  {
    return m_nexthopList;
  }

  const NexthopList&
  getNexthopList() const
  {
    return m_nexthopList;
  }

  inline bool
  operator==(RoutingTableEntry& rhs)
  {
    return m_destination == rhs.getDestination() &&
           m_nexthopList == rhs.getNexthopList();
  }

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  const ndn::Block&
  wireEncode() const;

  void
  wireDecode(const ndn::Block& wire);

protected:
  ndn::Name m_destination;
  NexthopList m_nexthopList;

  mutable ndn::Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(RoutingTableEntry);

std::ostream&
operator<<(std::ostream& os, const RoutingTableEntry& rte);

} // namespace nlsr

#endif // NLSR_ROUTING_TABLE_ENTRY_HPP
