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

#include "nexthop.hpp"
#include "tlv-nlsr.hpp"
#include "coordinate-lsa.hpp"

#include <ndn-cxx/util/concepts.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {
namespace tlv {

BOOST_CONCEPT_ASSERT((ndn::WireEncodable<NextHop>));
BOOST_CONCEPT_ASSERT((ndn::WireDecodable<NextHop>));
static_assert(std::is_base_of<ndn::tlv::Error, NextHop::Error>::value,
              "NextHop::Error must inherit from tlv::Error");

NextHop::NextHop()
  : m_cost(0)
{
}

NextHop::NextHop(const ndn::Block& block)
{
  wireDecode(block);
}

template<ndn::encoding::Tag TAG>
size_t
NextHop::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  totalLength += prependDouble(block, ndn::tlv::nlsr::Double, m_cost);

  totalLength += block.prependByteArrayBlock(
    ndn::tlv::nlsr::Uri, reinterpret_cast<const uint8_t*>(m_uri.c_str()), m_uri.size());

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::NextHop);

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
  m_uri = "";
  m_cost = 0;

  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::NextHop) {
    std::stringstream error;
    error << "Expected NextHop Block, but Block is of a different type: #"
          << m_wire.type();
    BOOST_THROW_EXCEPTION(Error(error.str()));
  }

  m_wire.parse();

  ndn::Block::element_const_iterator val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::Uri) {
    m_uri.assign(reinterpret_cast<const char*>(val->value()), val->value_size());
    ++val;
  }
  else {
    BOOST_THROW_EXCEPTION(Error("Missing required Uri field"));
  }

  m_cost = ndn::tlv::nlsr::readDouble(*val);
}

std::ostream&
operator<<(std::ostream& os, const NextHop& nexthop)
{
  os << "NextHop("
     << "Uri: " << nexthop.getUri() << ", "<< "Cost: " << nexthop.getCost() << ")" << std::endl;

  return os;
}

} // namespace tlv
} // namespace nlsr
