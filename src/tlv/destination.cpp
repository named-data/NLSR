/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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

#include "destination.hpp"
#include "tlv-nlsr.hpp"

#include <ndn-cxx/util/concepts.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {
namespace tlv {

BOOST_CONCEPT_ASSERT((ndn::WireEncodable<Destination>));
BOOST_CONCEPT_ASSERT((ndn::WireDecodable<Destination>));
static_assert(std::is_base_of<ndn::tlv::Error, Destination::Error>::value,
              "Destination::Error must inherit from tlv::Error");

Destination::Destination() = default;

Destination::Destination(const ndn::Block& block)
{
  wireDecode(block);
}

template<ndn::encoding::Tag TAG>
size_t
Destination::wireEncode(ndn::EncodingImpl<TAG>& encoder) const
{
  size_t totalLength = 0;

  totalLength += m_name.wireEncode(encoder);

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(ndn::tlv::nlsr::Destination);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(Destination);

const ndn::Block&
Destination::wireEncode() const
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
Destination::wireDecode(const ndn::Block& wire)
{
  m_name.clear();

  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::Destination) {
    std::stringstream error;
    error << "Expected Destination Block, but Block is of a different type: #"
          << m_wire.type();
    BOOST_THROW_EXCEPTION(Error(error.str()));
  }

  m_wire.parse();

  ndn::Block::element_const_iterator val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::Name) {
    m_name.wireDecode(*val);
    ++val;
  }
  else {
    BOOST_THROW_EXCEPTION(Error("Missing required Name field"));
  }
}

std::ostream&
operator<<(std::ostream& os, const Destination& Destination)
{
  os << "Destination: " << Destination.getName();
  return os;
}

std::shared_ptr<Destination>
makeDes(const RoutingTableEntry& rte)
{
  std::shared_ptr<Destination> desInfo = std::make_shared<Destination>();

  desInfo->setName(rte.getDestination());

  return desInfo;
}

} // namespace tlv
} // namespace nlsr
