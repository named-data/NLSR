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

#ifndef NLSR_TLV_NLSR_HPP
#define NLSR_TLV_NLSR_HPP

#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace ndn {
namespace tlv {
namespace nlsr {

/*! The TLV block types that NLSR uses to encode/decode LSA types. The
 *  way NLSR encodes LSAs to TLV is by encoding each element of the
 *  LSA as a separate TLV block. So, block types are needed. These are
 *  used in the LSDB Status Dataset.
 */
enum {
  LsaInfo          = 128,
  OriginRouter     = 129,
  SequenceNumber   = 130,
  AdjacencyLsa     = 131,
  Adjacency        = 132,
  CoordinateLsa    = 133,
  Double           = 134,
  HyperbolicRadius = 135,
  HyperbolicAngle  = 136,
  NameLsa          = 137,
  LsdbStatus       = 138,
  ExpirationPeriod = 139,
  Cost             = 140,
  Uri              = 141,
  Destination      = 142,
  NextHop          = 143,
  RoutingTable     = 144,
  RouteTableEntry  = 145
};

/*! \brief Read a double from a TLV element
 *  \param block the TLV element
 *  \throw ndn::tlv::Error block does not contain a double
 *  \sa prependDouble
 */
inline double
readDouble(const ndn::Block& block)
{
  block.parse();
  auto it = block.elements_begin();

  double doubleFromBlock = 0.0;
  if (it == it->elements_end() || it->type() != ndn::tlv::nlsr::Double ||
      it->value_size() != sizeof(doubleFromBlock)) {
    BOOST_THROW_EXCEPTION(ndn::tlv::Error("Block does not contain a double"));
  }
  memcpy(&doubleFromBlock, it->value(), sizeof(doubleFromBlock));
  return doubleFromBlock;
}

/*! \brief Prepend a TLV element containing a double.
 *  \param encoder an EncodingBuffer or EncodingEstimator
 *  \param type TLV-TYPE number
 *  \param value double value
 */
template<ndn::encoding::Tag TAG>
inline size_t
prependDouble(ndn::EncodingImpl<TAG>& encoder, uint32_t type, double value)
{
  size_t totalLength = 0;

  const uint8_t* doubleBytes = reinterpret_cast<const uint8_t*>(&value);
  totalLength = encoder.prependByteArrayBlock(ndn::tlv::nlsr::Double, doubleBytes, 8);
  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(type);

  return totalLength;
}

} // namespace nlsr
} // namespace tlv
} // namespace ndn

#endif // NLSR_TLV_NLSR_HPP
