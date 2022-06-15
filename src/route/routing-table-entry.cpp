/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022,  The University of Memphis,
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

#include "routing-table-entry.hpp"
#include "nexthop-list.hpp"
#include "tlv-nlsr.hpp"

namespace nlsr {

template<ndn::encoding::Tag TAG>
size_t
RoutingTableEntry::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  for (auto it = m_nexthopList.rbegin(); it != m_nexthopList.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  totalLength += m_destination.wireEncode(block);

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(nlsr::tlv::RoutingTableEntry);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(RoutingTableEntry);

const ndn::Block&
RoutingTableEntry::wireEncode() const
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
RoutingTableEntry::wireDecode(const ndn::Block& wire)
{
  m_nexthopList.clear();

  m_wire = wire;

  if (m_wire.type() != nlsr::tlv::RoutingTableEntry) {
    NDN_THROW(Error("RoutingTableEntry", m_wire.type()));
  }

  m_wire.parse();
  auto val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::Name) {
    m_destination.wireDecode(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required Name field"));
  }

  for (; val != m_wire.elements_end(); ++val) {
    if (val->type() == nlsr::tlv::NextHop) {
      m_nexthopList.addNextHop(NextHop(*val));
    }
    else {
      NDN_THROW(Error("NextHop", val->type()));
    }
  }
}

std::ostream&
operator<<(std::ostream& os, const RoutingTableEntry& rte)
{
  return os << "  Destination: " << rte.getDestination() << "\n"
            << rte.getNexthopList() << "\n";
}

} // namespace nlsr
