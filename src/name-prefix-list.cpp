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

#include "name-prefix-list.hpp"
#include "common.hpp"
#include "tlv-nlsr.hpp"

namespace nlsr {

NamePrefixList::NamePrefixList() = default;

NamePrefixList::NamePrefixList(std::initializer_list<ndn::Name> names)
{
  for (const auto& name : names) {
    insert(name);
  }
}

bool
NamePrefixList::insert(const ndn::Name& name, const std::string& source, double cost)
{
  auto& soucePrefixInfo = m_namesSources[name];
  soucePrefixInfo.costObj = PrefixInfo(name, cost);
  return soucePrefixInfo.sources.insert(source).second;
}

bool
NamePrefixList::insert(const PrefixInfo& nameCost)
{
  auto& soucePrefixInfo = m_namesSources[nameCost.getName()];
  soucePrefixInfo.costObj = nameCost;
  return soucePrefixInfo.sources.insert("").second;
}

bool
NamePrefixList::erase(const ndn::Name& name, const std::string& source)
{
  auto it = m_namesSources.find(name);
  if (it == m_namesSources.end()) {
    return false;
  }

  bool isRemoved = it->second.sources.erase(source);
  if (it->second.sources.empty()) {
    m_namesSources.erase(it);
  }
  return isRemoved;
}

const PrefixInfo&
NamePrefixList::getPrefixInfoForName(const ndn::Name& name) const
{
  auto it = m_namesSources.find(name);
  BOOST_ASSERT(it != m_namesSources.end());
  return it->second.costObj;
}

std::list<ndn::Name>
NamePrefixList::getNames() const
{
  std::list<ndn::Name> names;
  for (const auto& [name, sources] : m_namesSources) {
    names.emplace_back(name);
  }
  return names;
}

std::list<PrefixInfo>
NamePrefixList::getPrefixInfo() const
{
  std::list<PrefixInfo> nameCosts;
  for (const auto& [name, soucePrefixInfo] : m_namesSources) {
    nameCosts.emplace_back(name, soucePrefixInfo.costObj.getCost());
  }
  return nameCosts;
}

#ifdef WITH_TESTS

std::set<std::string>
NamePrefixList::getSources(const ndn::Name& name) const
{
  if (auto it = m_namesSources.find(name); it != m_namesSources.end()) {
    return it->second.sources;
  }
  return {};
}

#endif

std::ostream&
operator<<(std::ostream& os, const NamePrefixList& list)
{
  os << "Name prefix list: {\n";
  for (const auto& [name, sources] : list.m_namesSources) {
    os << name << "\nSources:\n";
    for (const auto& source : sources.sources) {
      os << "  " << source << "\n";
    }
  }
  os << "}" << std::endl;
  return os;
}

template<ndn::encoding::Tag TAG>
size_t
PrefixInfo::wireEncode(ndn::EncodingImpl<TAG>& encoder) const
{
  size_t totalLength = 0;

  totalLength += prependDoubleBlock(encoder, nlsr::tlv::Cost, m_prefixCost);

  totalLength += m_prefixName.wireEncode(encoder);

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(nlsr::tlv::PrefixInfo);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(PrefixInfo);

const ndn::Block&
PrefixInfo::wireEncode() const
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
PrefixInfo::wireDecode(const ndn::Block& wire)
{
  m_wire = wire;

  if (m_wire.type() != nlsr::tlv::PrefixInfo) {
    NDN_THROW(Error("PrefixInfo", m_wire.type()));
  }

  m_wire.parse();

  auto val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::Name) {
    m_prefixName.wireDecode(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required Name field"));
  }

  if (val != m_wire.elements_end() && val->type() == nlsr::tlv::Cost) {
    m_prefixCost = ndn::encoding::readDouble(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required Cost field"));
  }
}

} // namespace nlsr
