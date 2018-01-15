/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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

#ifndef NLSR_TLV_ROUTING_TABLE_ENTRY_HPP
#define NLSR_TLV_ROUTING_TABLE_ENTRY_HPP

#include "destination.hpp"
#include "nexthop.hpp"

#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/name.hpp>

#include <list>

namespace nlsr {
namespace tlv {

/*! \brief Data abstraction for RouteTableInfo
 *
 *   RouteTableInfo := ROUTINGTABLE-TYPE TLV-LENGTH
 *                       Destination
 *                       NexthopList*
 *
 * \sa https://redmine.named-data.net/projects/nlsr/wiki/Routing_Table_DataSet
 */
class RoutingTable
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : ndn::tlv::Error(what)
    {
    }
  };

  typedef std::list<NextHop> HopList;
  typedef HopList::const_iterator const_iterator;

  RoutingTable();

  explicit
  RoutingTable(const ndn::Block& block);

  const Destination&
  getDestination() const
  {
    return m_des;
  }

  RoutingTable&
  setDestination(const Destination& des)
  {
    m_des = des;
    m_wire.reset();
    return *this;
  }

  uint64_t
  getRtSize() const
  {
    return m_size;
  }

  RoutingTable&
  setRtSize(uint64_t size)
  {
    m_size = size;
    m_wire.reset();
    return *this;
  }

  bool
  hasNexthops() const;

  const std::list<NextHop>&
  getNextHops() const
  {
    return m_nexthops;
  }

  RoutingTable&
  addNexthops(const NextHop& nexthop);

  RoutingTable&
  clearNexthops();

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  const ndn::Block&
  wireEncode() const;

  void
  wireDecode(const ndn::Block& wire);

  const_iterator
  begin() const;

  const_iterator
  end() const;

private:
  Destination m_des;
  uint64_t m_size;
  bool m_hasNexthops;
  HopList m_nexthops;

  mutable ndn::Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(RoutingTable);

inline RoutingTable::const_iterator
RoutingTable::begin() const
{
  return m_nexthops.begin();
}

inline RoutingTable::const_iterator
RoutingTable::end() const
{
  return m_nexthops.end();
}

std::ostream&
operator<<(std::ostream& os, const RoutingTable& routetable);

} // namespace tlv
} // namespace nlsr

#endif // NLSR_TLV_ROUTING_TABLE_ENTRY_HPP
