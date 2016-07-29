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

#include "adjacency-lsa.hpp"
#include "tlv-nlsr.hpp"

#include <ndn-cxx/util/concepts.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {
namespace tlv  {

BOOST_CONCEPT_ASSERT((ndn::WireEncodable<AdjacencyLsa>));
BOOST_CONCEPT_ASSERT((ndn::WireDecodable<AdjacencyLsa>));
static_assert(std::is_base_of<ndn::tlv::Error, AdjacencyLsa::Error>::value,
              "AdjacencyLsa::Error must inherit from tlv::Error");

AdjacencyLsa::AdjacencyLsa()
  : m_hasAdjacencies(false)
{
}

AdjacencyLsa::AdjacencyLsa(const ndn::Block& block)
{
  wireDecode(block);
}

template<ndn::encoding::Tag TAG>
size_t
AdjacencyLsa::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  for (std::list<Adjacency>::const_reverse_iterator it = m_adjacencies.rbegin();
       it != m_adjacencies.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  totalLength += m_lsaInfo.wireEncode(block);

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::AdjacencyLsa);

  return totalLength;
}

template size_t
AdjacencyLsa::wireEncode<ndn::encoding::EncoderTag>(ndn::EncodingImpl<ndn::encoding::EncoderTag>& encoder) const;

template size_t
AdjacencyLsa::wireEncode<ndn::encoding::EstimatorTag>(ndn::EncodingImpl<ndn::encoding::EstimatorTag>& encoder) const;

const ndn::Block&
AdjacencyLsa::wireEncode() const
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
AdjacencyLsa::wireDecode(const ndn::Block& wire)
{
  m_hasAdjacencies = false;
  m_adjacencies.clear();

  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::AdjacencyLsa) {
    std::stringstream error;
    error << "Expected AdjacencyLsa Block, but Block is of a different type: #"
          << m_wire.type();
    throw Error(error.str());
  }

  m_wire.parse();

  ndn::Block::element_const_iterator val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::LsaInfo) {
    m_lsaInfo.wireDecode(*val);
    ++val;
  }
  else {
    throw Error("Missing required LsaInfo field");
  }

  for (; val != m_wire.elements_end(); ++val) {
    if (val->type() == ndn::tlv::nlsr::Adjacency) {
      m_adjacencies.push_back(Adjacency(*val));
      m_hasAdjacencies = true;
    }
    else {
      std::stringstream error;
      error << "Expected Adjacency Block, but Block is of a different type: #"
            << m_wire.type();
      throw Error(error.str());
    }
  }
}

std::ostream&
operator<<(std::ostream& os, const AdjacencyLsa& adjacencyLsa)
{
  os << "AdjacencyLsa("
     << adjacencyLsa.getLsaInfo();

  for (const auto& adjacency : adjacencyLsa) {
    os << ", " << adjacency;
  }

  os << ")";

  return os;
}

} // namespace tlv
} // namespace nlsr
