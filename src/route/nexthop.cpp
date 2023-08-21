/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023,  The University of Memphis,
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

#include "nexthop.hpp"
#include "tlv-nlsr.hpp"

#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {

template<ndn::encoding::Tag TAG>
size_t
NextHop::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  totalLength += ndn::encoding::prependDoubleBlock(block, nlsr::tlv::CostDouble, m_routeCost);
  totalLength += ndn::encoding::prependStringBlock(block, nlsr::tlv::Uri, m_connectingFaceUri.toString());

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(nlsr::tlv::NextHop);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(NextHop);

const ndn::Block&
NextHop::wireEncode() const
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
NextHop::wireDecode(const ndn::Block& wire)
{
  m_connectingFaceUri = {};
  m_routeCost = 0;

  m_wire = wire;

  if (m_wire.type() != nlsr::tlv::NextHop) {
    NDN_THROW(Error("NextHop", m_wire.type()));
  }

  m_wire.parse();

  auto val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == nlsr::tlv::Uri) {
    try {
      m_connectingFaceUri = ndn::FaceUri(ndn::encoding::readString(*val));
    }
    catch (const ndn::FaceUri::Error& e) {
      NDN_THROW_NESTED(Error("Invalid Uri"));
    }
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required Uri field"));
  }

  if (val != m_wire.elements_end() && val->type() == nlsr::tlv::CostDouble) {
    m_routeCost = ndn::encoding::readDouble(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required CostDouble field"));
  }
}

std::ostream&
operator<<(std::ostream& os, const NextHop& hop)
{
  os << "NextHop(Uri: " << hop.getConnectingFaceUri() << ", Cost: " << hop.getRouteCost() << ")";
  return os;
}

} // namespace nlsr
