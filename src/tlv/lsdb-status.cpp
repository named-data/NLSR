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

#include "lsdb-status.hpp"
#include "tlv-nlsr.hpp"

#include <ndn-cxx/util/concepts.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {
namespace tlv  {

BOOST_CONCEPT_ASSERT((ndn::WireEncodable<LsdbStatus>));
BOOST_CONCEPT_ASSERT((ndn::WireDecodable<LsdbStatus>));
static_assert(std::is_base_of<ndn::tlv::Error, LsdbStatus::Error>::value,
              "LsdbStatus::Error must inherit from tlv::Error");

LsdbStatus::LsdbStatus()
  : m_hasAdjacencyLsas(false)
  , m_hasCoordinateLsas(false)
  , m_hasNameLsas(false)
{
}

LsdbStatus::LsdbStatus(const ndn::Block& block)
{
  wireDecode(block);
}

template<ndn::encoding::Tag TAG>
size_t
LsdbStatus::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  for (std::list<NameLsa>::const_reverse_iterator it = m_nameLsas.rbegin();
       it != m_nameLsas.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  for (std::list<CoordinateLsa>::const_reverse_iterator it = m_coordinateLsas.rbegin();
       it != m_coordinateLsas.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  for (std::list<AdjacencyLsa>::const_reverse_iterator it = m_adjacencyLsas.rbegin();
       it != m_adjacencyLsas.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::LsdbStatus);

  return totalLength;
}

template size_t
LsdbStatus::wireEncode<ndn::encoding::EncoderTag>(ndn::EncodingImpl<ndn::encoding::EncoderTag>& block) const;

template size_t
LsdbStatus::wireEncode<ndn::encoding::EstimatorTag>(ndn::EncodingImpl<ndn::encoding::EstimatorTag>& block) const;

const ndn::Block&
LsdbStatus::wireEncode() const
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
LsdbStatus::wireDecode(const ndn::Block& wire)
{
  m_adjacencyLsas.clear();
  m_coordinateLsas.clear();
  m_nameLsas.clear();

  m_hasAdjacencyLsas = false;
  m_hasCoordinateLsas = false;
  m_hasNameLsas = false;

  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::LsdbStatus) {
    std::stringstream error;
    error << "Expected LsdbStatus Block, but Block is of a different type: #"
          << m_wire.type();
    throw Error(error.str());
  }

  m_wire.parse();

  ndn::Block::element_const_iterator val = m_wire.elements_begin();

  for (; val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::AdjacencyLsa; ++val) {
    m_adjacencyLsas.push_back(AdjacencyLsa(*val));
    m_hasAdjacencyLsas = true;
  }

  for (; val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::CoordinateLsa; ++val) {
    m_coordinateLsas.push_back(CoordinateLsa(*val));
    m_hasCoordinateLsas = true;
  }

  for (; val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::NameLsa; ++val) {
    m_nameLsas.push_back(NameLsa(*val));
    m_hasNameLsas = true;
  }

  if (val != m_wire.elements_end()) {
    std::stringstream error;
    error << "Expected the end of elements, but Block is of a different type: #"
          << val->type();
    throw Error(error.str());
  }
}

std::ostream&
operator<<(std::ostream& os, const LsdbStatus& lsdbStatus)
{
  os << "LsdbStatus(";

  bool isFirst = true;

  for (const auto& adjacencyLsa : lsdbStatus.getAdjacencyLsas()) {
    if (isFirst) {
      isFirst = false;
    }
    else {
      os << ", ";
    }

    os << adjacencyLsa;
  }

  for (const auto& coordinateLsa : lsdbStatus.getCoordinateLsas()) {
    if (isFirst) {
      isFirst = false;
    }
    else {
      os << ", ";
    }

    os << coordinateLsa;
  }

  for (const auto& nameLsa : lsdbStatus.getNameLsas()) {
    if (isFirst) {
      isFirst = false;
    }
    else {
      os << ", ";
    }

    os << nameLsa;
  }

  os << ")";

  return os;
}

} // namespace tlv
} // namespace nlsr
