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

#include "adjacent.hpp"
#include "logger.hpp"
#include "tlv-nlsr.hpp"
#include "utility/numeric.hpp"

namespace nlsr {

INIT_LOGGER(Adjacent);

Adjacent::Adjacent()
  : m_name()
  , m_faceUri()
  , m_linkCost(DEFAULT_LINK_COST)
  , m_status(STATUS_INACTIVE)
  , m_interestTimedOutNo(0)
  , m_faceId(0)
{
}

Adjacent::Adjacent(const ndn::Block& block)
{
  wireDecode(block);
}

Adjacent::Adjacent(const ndn::Name& an)
  : m_name(an)
  , m_faceUri()
  , m_linkCost(DEFAULT_LINK_COST)
  , m_status(STATUS_INACTIVE)
  , m_interestTimedOutNo(0)
  , m_faceId(0)
{
}

Adjacent::Adjacent(const ndn::Name& an, const ndn::FaceUri& faceUri, double lc,
                   Status s, uint32_t iton, uint64_t faceId)
  : m_name(an)
  , m_faceUri(faceUri)
  , m_status(s)
  , m_interestTimedOutNo(iton)
  , m_faceId(faceId)
{
  this->setLinkCost(lc);
}

void
Adjacent::setLinkCost(double lc)
{
  // NON_ADJACENT_COST is a negative value and is used for nodes that aren't direct neighbors.
  // But for direct/active neighbors, the cost cannot be negative.
  if (lc < 0 && lc != NON_ADJACENT_COST) {
    NLSR_LOG_ERROR(" Neighbor's link-cost cannot be negative");
    NDN_THROW(ndn::tlv::Error("Neighbor's link-cost cannot be negative"));
  }

  m_linkCost = lc;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(Adjacent);

template<ndn::encoding::Tag TAG>
size_t
Adjacent::wireEncode(ndn::EncodingImpl<TAG>& encoder) const
{
  size_t totalLength = 0;

  totalLength += prependDoubleBlock(encoder, nlsr::tlv::Cost, m_linkCost);

  totalLength += prependStringBlock(encoder, nlsr::tlv::Uri, m_faceUri.toString());

  totalLength += m_name.wireEncode(encoder);

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(nlsr::tlv::Adjacency);

  return totalLength;
}

const ndn::Block&
Adjacent::wireEncode() const
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
Adjacent::wireDecode(const ndn::Block& wire)
{
  m_name.clear();
  m_faceUri = ndn::FaceUri();
  m_linkCost = 0;

  m_wire = wire;

  if (m_wire.type() != nlsr::tlv::Adjacency) {
    NDN_THROW(Error("Adjacency", m_wire.type()));
  }

  m_wire.parse();

  auto val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::Name) {
    m_name.wireDecode(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required Name field"));
  }

  if (val != m_wire.elements_end() && val->type() == nlsr::tlv::Uri) {
    m_faceUri = ndn::FaceUri(readString(*val));
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required Uri field"));
  }

  if (val != m_wire.elements_end() && val->type() == nlsr::tlv::Cost) {
    m_linkCost = ndn::encoding::readDouble(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required Cost field"));
  }
}

bool
Adjacent::operator==(const Adjacent& adjacent) const
{
  return m_name == adjacent.getName() &&
         m_faceUri == adjacent.getFaceUri() &&
         util::diffInEpsilon(m_linkCost, adjacent.getLinkCost());
}

bool
Adjacent::operator<(const Adjacent& adjacent) const
{
  auto linkCost = adjacent.getLinkCost();
  return std::tie(m_name, m_linkCost) <
         std::tie(adjacent.getName(), linkCost);
}

std::ostream&
operator<<(std::ostream& os, const Adjacent& adjacent)
{
  os << "Adjacent: " << adjacent.m_name
     << "\n\t\tConnecting FaceUri: " << adjacent.m_faceUri
     << "\n\t\tLink cost: " << adjacent.m_linkCost
     << "\n\t\tStatus: " << adjacent.m_status
     << "\n\t\tInterest Timed Out: " << adjacent.m_interestTimedOutNo << std::endl;
  return os;
}

} // namespace nlsr
