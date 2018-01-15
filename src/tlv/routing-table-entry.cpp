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

#include "routing-table-entry.hpp"
#include "tlv-nlsr.hpp"

#include <ndn-cxx/util/concepts.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {
namespace tlv {

BOOST_CONCEPT_ASSERT((ndn::WireEncodable<RoutingTable>));
BOOST_CONCEPT_ASSERT((ndn::WireDecodable<RoutingTable>));
static_assert(std::is_base_of<ndn::tlv::Error, RoutingTable::Error>::value,
              "RoutingTable::Error must inherit from tlv::Error");

RoutingTable::RoutingTable()
  : m_hasNexthops(false)
{
}

RoutingTable::RoutingTable(const ndn::Block& block)
{
  wireDecode(block);
}

bool
RoutingTable::hasNexthops() const
{
  return m_hasNexthops;
}

RoutingTable&
RoutingTable::addNexthops(const NextHop& nexthop)
{
  m_nexthops.push_back(nexthop);
  m_wire.reset();
  m_hasNexthops = true;
  return *this;
}

RoutingTable&
RoutingTable::clearNexthops()
{
  m_nexthops.clear();
  m_hasNexthops = false;
  return *this;
}

template<ndn::encoding::Tag TAG>
size_t
RoutingTable::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  for (std::list<NextHop>::const_reverse_iterator it = m_nexthops.rbegin();
       it != m_nexthops.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  totalLength += m_des.wireEncode(block);

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::RouteTableEntry);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(RoutingTable);

const ndn::Block&
RoutingTable::wireEncode() const
{
  if (m_wire.hasWire()) {
    return m_wire;
  }

  ndn::EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  ndn::EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();

  return m_wire;
}

void
RoutingTable::wireDecode(const ndn::Block& wire)
{
  m_hasNexthops = false;
  m_nexthops.clear();

  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::RouteTableEntry) {
    std::stringstream error;
    error << "Expected RoutingTable Block, but Block is of a different type: #"
          << m_wire.type();
    BOOST_THROW_EXCEPTION(Error(error.str()));
  }

  m_wire.parse();

  ndn::Block::element_const_iterator val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::Destination) {
    m_des.wireDecode(*val);
    ++val;
  }
  else {
    BOOST_THROW_EXCEPTION(Error("Missing required destination field"));
  }

  for (; val != m_wire.elements_end(); ++val) {
    if (val->type() == ndn::tlv::nlsr::NextHop) {
      m_nexthops.push_back(NextHop(*val));
      m_hasNexthops = true;
    }
    else {
      std::stringstream error;
      error << "Expected NextHop Block, but Block is of a different type: #"
            << m_wire.type();
      BOOST_THROW_EXCEPTION(Error(error.str()));
    }
  }
}

std::ostream&
operator<<(std::ostream& os, const RoutingTable& routingtable)
{
  os << routingtable.getDestination() << std::endl;
  os << "NexthopList(" << std::endl;

  for (const auto& rtentry : routingtable) {
    os << rtentry;
  }

  os << ")";
  return os;
}

} // namespace tlv
} // namespace nlsr
