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

#include "lsa.hpp"
#include "nlsr.hpp"
#include "name-prefix-list.hpp"
#include "adjacent.hpp"
#include "tlv-nlsr.hpp"

namespace nlsr {

Lsa::Lsa(const ndn::Name& originRouter, uint64_t seqNo,
         ndn::time::system_clock::TimePoint expirationTimePoint)
  : m_originRouter(originRouter)
  , m_seqNo(seqNo)
  , m_expirationTimePoint(expirationTimePoint)
{
}

Lsa::Lsa(const Lsa& lsa)
  : m_originRouter(lsa.getOriginRouter())
  , m_seqNo(lsa.getSeqNo())
  , m_expirationTimePoint(lsa.getExpirationTimePoint())
{
}

template<ndn::encoding::Tag TAG>
size_t
Lsa::wireEncode(ndn::EncodingImpl<TAG>& encoder) const
{
  size_t totalLength = 0;

  totalLength += prependStringBlock(encoder,
                                    ndn::tlv::nlsr::ExpirationTime,
                                    ndn::time::toString(m_expirationTimePoint));

  totalLength += prependNonNegativeIntegerBlock(encoder, ndn::tlv::nlsr::SequenceNumber,
                                                m_seqNo);

  totalLength += m_originRouter.wireEncode(encoder);

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(ndn::tlv::nlsr::Lsa);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(Lsa);

void
Lsa::wireDecode(const ndn::Block& wire)
{
  m_originRouter.clear();
  m_seqNo = 0;

  ndn::Block baseWire = wire;
  baseWire.parse();

  auto val = baseWire.elements_begin();

  if (val != baseWire.elements_end() && val->type() == ndn::tlv::Name) {
    m_originRouter.wireDecode(*val);
  }
  else {
    NDN_THROW(Error("OriginRouter: Missing required Name field"));
  }

  ++val;

  if (val != baseWire.elements_end() && val->type() == ndn::tlv::nlsr::SequenceNumber) {
    m_seqNo = ndn::readNonNegativeInteger(*val);
    ++val;
  }
  else {
    NDN_THROW(Error("Missing required SequenceNumber field"));
  }

  if (val != baseWire.elements_end() && val->type() == ndn::tlv::nlsr::ExpirationTime) {
    m_expirationTimePoint = ndn::time::fromString(readString(*val));
  }
  else {
    NDN_THROW(Error("Missing required ExpirationTime field"));
  }
}

std::ostream&
operator<<(std::ostream& os, const Lsa::Type& type)
{
  switch (type) {
  case nlsr::Lsa::Type::ADJACENCY:
    os << "ADJACENCY";
    break;

  case nlsr::Lsa::Type::COORDINATE:
    os << "COORDINATE";
    break;

  case nlsr::Lsa::Type::NAME:
    os << "NAME";
    break;

  default:
    os << "BASE";
    break;
  }
  return os;
}

std::istream&
operator>>(std::istream& is, Lsa::Type& type)
{
  std::string typeString;
  is >> typeString;
  if (typeString == "ADJACENCY") {
    type = Lsa::Type::ADJACENCY;
  }
  else if (typeString == "COORDINATE") {
    type = Lsa::Type::COORDINATE;
  }
  else if (typeString == "NAME") {
    type = Lsa::Type::NAME;
  }
  else {
    type = Lsa::Type::BASE;
  }
  return is;
}

std::string
Lsa::getString() const
{
  std::ostringstream os;
  auto duration = m_expirationTimePoint - ndn::time::system_clock::now();
  os << "    " << getType() << " LSA:\n"
     << "      Origin Router      : " << m_originRouter << "\n"
     << "      Sequence Number    : " << m_seqNo << "\n"
     << "      Expires in         : " << ndn::time::duration_cast<ndn::time::milliseconds>(duration)
     << "\n";
  return os.str();
}

} // namespace nlsr
