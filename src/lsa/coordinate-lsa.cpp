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

#include "coordinate-lsa.hpp"
#include "tlv-nlsr.hpp"

#include <boost/lexical_cast.hpp>

namespace nlsr {

CoordinateLsa::CoordinateLsa(const ndn::Name& originRouter, uint64_t seqNo,
                             const ndn::time::system_clock::TimePoint& timepoint,
                             double radius, std::vector<double> angles)
  : Lsa(originRouter, seqNo, timepoint)
  , m_hyperbolicRadius(radius)
  , m_hyperbolicAngles(angles)
{
}

CoordinateLsa::CoordinateLsa(const ndn::Block& block)
{
  wireDecode(block);
}

bool
CoordinateLsa::isEqualContent(const CoordinateLsa& clsa) const
{
  if (clsa.getCorTheta().size() != m_hyperbolicAngles.size()) {
    return false;
  }

  std::vector<double> m_angles2 = clsa.getCorTheta();
  for (unsigned int i = 0; i < clsa.getCorTheta().size(); i++) {
    if (std::abs(m_hyperbolicAngles[i] - m_angles2[i]) > std::numeric_limits<double>::epsilon()) {
      return false;
    }
  }

  return (std::abs(m_hyperbolicRadius - clsa.getCorRadius()) <
          std::numeric_limits<double>::epsilon());
}

template<ndn::encoding::Tag TAG>
size_t
CoordinateLsa::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  for (auto it = m_hyperbolicAngles.rbegin(); it != m_hyperbolicAngles.rend(); ++it) {
    totalLength += ndn::encoding::prependDoubleBlock(block, ndn::tlv::nlsr::HyperbolicAngle, *it);
  }

  totalLength += ndn::encoding::prependDoubleBlock(block, ndn::tlv::nlsr::HyperbolicRadius,
                                                   m_hyperbolicRadius);

  totalLength += Lsa::wireEncode(block);

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::CoordinateLsa);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(CoordinateLsa);

const ndn::Block&
CoordinateLsa::wireEncode() const
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
CoordinateLsa::wireDecode(const ndn::Block& wire)
{
  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::CoordinateLsa) {
    NDN_THROW(Error("CoordinateLsa", m_wire.type()));
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

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::HyperbolicRadius) {
    m_hyperbolicRadius = ndn::encoding::readDouble(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required HyperbolicRadius field"));
  }

  std::vector<double> angles;
  for (; val != m_wire.elements_end(); ++val) {
    if (val->type() == ndn::tlv::nlsr::HyperbolicAngle) {
      angles.push_back(ndn::encoding::readDouble(*val));
    }
    else {
      NDN_THROW(Error("Missing required HyperbolicAngle field"));
    }
  }
  m_hyperbolicAngles = angles;
}

std::string
CoordinateLsa::toString() const
{
  std::ostringstream os;
  os << getString();
  os << "      Hyperbolic Radius  : " << m_hyperbolicRadius << "\n";
  int i = 0;
  for (const auto& value : m_hyperbolicAngles) {
    os << "      Hyperbolic Theta " << i++ << " : " << value << "\n";
  }

  return os.str();
}

std::tuple<bool, std::list<ndn::Name>, std::list<ndn::Name>>
CoordinateLsa::update(const std::shared_ptr<Lsa>& lsa)
{
  auto clsa = std::static_pointer_cast<CoordinateLsa>(lsa);
  if (!isEqualContent(*clsa)) {
    m_hyperbolicRadius = clsa->getCorRadius();
    m_hyperbolicAngles.clear();
    for (const auto& angle : clsa->getCorTheta()) {
      m_hyperbolicAngles.push_back(angle);
    }
    return std::make_tuple(true, std::list<ndn::Name>{}, std::list<ndn::Name>{});
  }
  return std::make_tuple(false, std::list<ndn::Name>{}, std::list<ndn::Name>{});
}

std::ostream&
operator<<(std::ostream& os, const CoordinateLsa& lsa)
{
  return os << lsa.toString();
}

} // namespace nlsr
