/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
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
 */

#include "adj-lsa.hpp"
#include "tlv-nlsr.hpp"

namespace nlsr {

AdjLsa::AdjLsa(const ndn::Name& originRouter, uint64_t seqNo,
               const ndn::time::system_clock::time_point& timepoint, AdjacencyList& adl)
  : Lsa(originRouter, seqNo, timepoint)
{
  for (const auto& adjacent : adl.getAdjList()) {
    if (adjacent.getStatus() == Adjacent::STATUS_ACTIVE) {
      addAdjacent(adjacent);
    }
  }
}

AdjLsa::AdjLsa(const ndn::Block& block)
{
  wireDecode(block);
}

template<ndn::encoding::Tag TAG>
size_t
AdjLsa::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  auto list = m_adl.getAdjList();
  for (auto it = list.rbegin(); it != list.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  totalLength += Lsa::wireEncode(block);

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(nlsr::tlv::AdjacencyLsa);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(AdjLsa);

const ndn::Block&
AdjLsa::wireEncode() const
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
AdjLsa::wireDecode(const ndn::Block& wire)
{
  m_wire = wire;

  if (m_wire.type() != nlsr::tlv::AdjacencyLsa) {
    NDN_THROW(Error("AdjacencyLsa", m_wire.type()));
  }

  m_wire.parse();

  auto val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == nlsr::tlv::Lsa) {
    Lsa::wireDecode(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required Lsa field"));
  }

  AdjacencyList adl;
  for (; val != m_wire.elements_end(); ++val) {
    if (val->type() == nlsr::tlv::Adjacency) {
      adl.insert(Adjacent(*val));
    }
    else {
      NDN_THROW(Error("Adjacency", val->type()));
    }
  }
  m_adl = adl;
}

void
AdjLsa::print(std::ostream& os) const
{
  os << "      Adjacent(s):\n";

  int adjacencyIndex = 0;
  for (const auto& adjacency : m_adl) {
    os << "        Adjacent " << adjacencyIndex++
       << ": (name=" << adjacency.getName()
       << ", uri="   << adjacency.getFaceUri()
       << ", cost="  << adjacency.getLinkCost() << ")\n";
  }
}

std::tuple<bool, std::list<PrefixInfo>, std::list<PrefixInfo>>
AdjLsa::update(const std::shared_ptr<Lsa>& lsa)
{
  auto alsa = std::static_pointer_cast<AdjLsa>(lsa);
  if (*this != *alsa) {
    resetAdl();
    for (const auto& adjacent : alsa->getAdl()) {
      addAdjacent(adjacent);
    }
    return {true, std::list<PrefixInfo>{}, std::list<PrefixInfo>{}};
  }
  return {false, std::list<PrefixInfo>{}, std::list<PrefixInfo>{}};
}

} // namespace nlsr
