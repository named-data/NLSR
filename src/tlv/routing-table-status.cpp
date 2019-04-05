/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2019,  The University of Memphis,
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

#include "routing-table-status.hpp"
#include "tlv-nlsr.hpp"

#include <ndn-cxx/util/concepts.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {
namespace tlv {

BOOST_CONCEPT_ASSERT((ndn::WireEncodable<RoutingTableStatus>));
BOOST_CONCEPT_ASSERT((ndn::WireDecodable<RoutingTableStatus>));
static_assert(std::is_base_of<ndn::tlv::Error, RoutingTableStatus::Error>::value,
              "RoutingTableStatus::Error must inherit from tlv::Error");

RoutingTableStatus::RoutingTableStatus()
  : m_hasRoutingtable(false)
{
}

RoutingTableStatus::RoutingTableStatus(const ndn::Block& block)
{
  wireDecode(block);
}

RoutingTableStatus&
RoutingTableStatus::addRoutingTable(const RoutingTable& routetable)
{
  m_routingtables.push_back(routetable);
  m_wire.reset();
  m_hasRoutingtable = true;
  return *this;
}

RoutingTableStatus&
RoutingTableStatus::clearRoutingTable()
{
  m_routingtables.clear();
  m_hasRoutingtable = false;
  return *this;
}

bool
RoutingTableStatus::hasRoutingTable()
{
  return m_hasRoutingtable;
}

template<ndn::encoding::Tag TAG>
size_t
RoutingTableStatus::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  for (std::list<RoutingTable>::const_reverse_iterator it = m_routingtables.rbegin();
       it != m_routingtables.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::RoutingTable);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(RoutingTableStatus);

const ndn::Block&
RoutingTableStatus::wireEncode() const
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
RoutingTableStatus::wireDecode(const ndn::Block& wire)
{
  m_routingtables.clear();

  m_hasRoutingtable = false;

  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::RoutingTable) {
    std::stringstream error;
    error << "Expected RoutingTableStatus Block, but Block is of a different type: #"
          << m_wire.type();
    BOOST_THROW_EXCEPTION(Error(error.str()));
  }

  m_wire.parse();

  ndn::Block::element_const_iterator val = m_wire.elements_begin();

  for (; val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::RouteTableEntry; ++val) {
    m_routingtables.push_back(RoutingTable(*val));
    m_hasRoutingtable = true;
  }

  if (val != m_wire.elements_end()) {
    std::stringstream error;
    error << "Expected the end of elements, but Block is of a different type: #"
          << val->type();
    BOOST_THROW_EXCEPTION(Error(error.str()));
  }
}

std::ostream&
operator<<(std::ostream& os, const RoutingTableStatus& rtStatus)
{
  os << "Routing Table Status: " << std::endl;

  bool isFirst = true;

  for (const auto& routingtable : rtStatus.getRoutingtable()) {
    if (isFirst) {
      isFirst = false;
    }
    else {
      os << ", ";
    }

    os << routingtable;
  }

  return os;
}

} // namespace tlv
} // namespace nlsr
