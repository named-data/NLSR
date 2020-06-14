/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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

#include "name-lsa.hpp"
#include "tlv-nlsr.hpp"

namespace nlsr {

NameLsa::NameLsa(const ndn::Name& originRouter, uint64_t seqNo,
                 const ndn::time::system_clock::TimePoint& timepoint,
                 const NamePrefixList& npl)
  : Lsa(originRouter, seqNo, timepoint)
{
  for (const auto& name : npl.getNames()) {
    addName(name);
  }
}

NameLsa::NameLsa(const ndn::Block& block)
{
  wireDecode(block);
}

template<ndn::encoding::Tag TAG>
size_t
NameLsa::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  auto names = m_npl.getNames();

  for (auto it = names.rbegin();  it != names.rend(); ++it) {
    totalLength += it->wireEncode(block);
  }

  totalLength += Lsa::wireEncode(block);

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::NameLsa);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(NameLsa);

const ndn::Block&
NameLsa::wireEncode() const
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
NameLsa::wireDecode(const ndn::Block& wire)
{
  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::NameLsa) {
    NDN_THROW(Error("NameLsa", m_wire.type()));
  }

  m_wire.parse();

  auto val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::Lsa) {
    Lsa::wireDecode(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required Lsa field"));
  }

  NamePrefixList npl;
  for (; val != m_wire.elements_end(); ++val) {
    if (val->type() == ndn::tlv::Name) {
      npl.insert(ndn::Name(*val));
    }
    else {
      NDN_THROW(Error("Name", val->type()));
    }
  }
  m_npl = npl;
}

bool
NameLsa::isEqualContent(const NameLsa& other) const
{
  return m_npl == other.getNpl();
}

std::string
NameLsa::toString() const
{
  std::ostringstream os;
  os << getString();
  os << "      Names:\n";
  int i = 0;
  for (const auto& name : m_npl.getNames()) {
    os << "        Name " << i++ << ": " << name << "\n";
  }

  return os.str();
}

std::tuple<bool, std::list<ndn::Name>, std::list<ndn::Name>>
NameLsa::update(const std::shared_ptr<Lsa>& lsa)
{
  auto nlsa = std::static_pointer_cast<NameLsa>(lsa);
  bool updated = false;

  // Obtain the set difference of the current and the incoming
  // name prefix sets, and add those.
  std::list<ndn::Name> newNames = nlsa->getNpl().getNames();
  std::list<ndn::Name> oldNames = m_npl.getNames();
  std::list<ndn::Name> namesToAdd;
  std::set_difference(newNames.begin(), newNames.end(), oldNames.begin(), oldNames.end(),
                      std::inserter(namesToAdd, namesToAdd.begin()));
  for (const auto& name : namesToAdd) {
    addName(name);
    updated = true;
  }

  m_npl.sort();

  // Also remove any names that are no longer being advertised.
  std::list<ndn::Name> namesToRemove;
  std::set_difference(oldNames.begin(), oldNames.end(), newNames.begin(), newNames.end(),
                      std::inserter(namesToRemove, namesToRemove.begin()));
  for (const auto& name : namesToRemove) {
    removeName(name);
    updated = true;
  }

  return std::make_tuple(updated, namesToAdd, namesToRemove);
}

std::ostream&
operator<<(std::ostream& os, const NameLsa& lsa)
{
  return os << lsa.toString();
}

} // namespace nlsr
