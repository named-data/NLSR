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

#include "lsa-info.hpp"
#include "tlv-nlsr.hpp"

#include <ndn-cxx/util/concepts.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {
namespace tlv  {

BOOST_CONCEPT_ASSERT((ndn::WireEncodable<LsaInfo>));
BOOST_CONCEPT_ASSERT((ndn::WireDecodable<LsaInfo>));
static_assert(std::is_base_of<ndn::tlv::Error, LsaInfo::Error>::value,
              "LsaInfo::Error must inherit from tlv::Error");

const ndn::time::milliseconds LsaInfo::INFINITE_EXPIRATION_PERIOD(ndn::time::milliseconds::max());

LsaInfo::LsaInfo()
  : m_sequenceNumber(0)
  , m_expirationPeriod(INFINITE_EXPIRATION_PERIOD)
  , m_hasInfiniteExpirationPeriod(true)
{
}

LsaInfo::LsaInfo(const ndn::Block& block)
{
  wireDecode(block);
}

template<ndn::encoding::Tag TAG>
size_t
LsaInfo::wireEncode(ndn::EncodingImpl<TAG>& encoder) const
{
  size_t totalLength = 0;

  // Absence of an ExpirationPeriod signifies non-expiration
  if (!m_hasInfiniteExpirationPeriod) {
    totalLength += prependNonNegativeIntegerBlock(encoder,
                                                  ndn::tlv::nlsr::ExpirationPeriod,
                                                  m_expirationPeriod.count());
  }

  totalLength += prependNonNegativeIntegerBlock(encoder,
                                                ndn::tlv::nlsr::SequenceNumber,
                                                m_sequenceNumber);

  totalLength += prependNestedBlock(encoder, ndn::tlv::nlsr::OriginRouter, m_originRouter);

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(ndn::tlv::nlsr::LsaInfo);

  return totalLength;
}

template size_t
LsaInfo::wireEncode<ndn::encoding::EncoderTag>(ndn::EncodingImpl<ndn::encoding::EncoderTag>& block) const;

template size_t
LsaInfo::wireEncode<ndn::encoding::EstimatorTag>(ndn::EncodingImpl<ndn::encoding::EstimatorTag>& block) const;

const ndn::Block&
LsaInfo::wireEncode() const
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
LsaInfo::wireDecode(const ndn::Block& wire)
{
  m_originRouter.clear();
  m_sequenceNumber = 0;
  m_expirationPeriod = ndn::time::milliseconds::min();

  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::LsaInfo) {
    std::stringstream error;
    error << "Expected LsaInfo Block, but Block is of a different type: #"
          << m_wire.type();
    throw Error(error.str());
  }

  m_wire.parse();

  ndn::Block::element_const_iterator val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::OriginRouter) {
    val->parse();
    ndn::Block::element_const_iterator it = val->elements_begin();

    if (it != val->elements_end() && it->type() == ndn::tlv::Name) {
      m_originRouter.wireDecode(*it);
    }
    else {
      throw Error("OriginRouter: Missing required Name field");
    }

    ++val;
  }
  else {
    throw Error("Missing required OriginRouter field");
  }

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::SequenceNumber) {
    m_sequenceNumber = ndn::readNonNegativeInteger(*val);
    ++val;
  }
  else {
    throw Error("Missing required SequenceNumber field");
  }

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::ExpirationPeriod) {
    m_expirationPeriod = ndn::time::milliseconds(ndn::readNonNegativeInteger(*val));
    m_hasInfiniteExpirationPeriod = false;
  }
  else {
    m_expirationPeriod = INFINITE_EXPIRATION_PERIOD;
    m_hasInfiniteExpirationPeriod = true;
  }
}

std::ostream&
operator<<(std::ostream& os, const LsaInfo& lsaInfo)
{
  os << "LsaInfo("
     << "OriginRouter: " << lsaInfo.getOriginRouter() << ", "
     << "SequenceNumber: " << lsaInfo.getSequenceNumber() << ", ";

  if (!lsaInfo.hasInfiniteExpirationPeriod()) {
    os << "ExpirationPeriod: " << lsaInfo.getExpirationPeriod();
  }
  else {
    os << "ExpirationPeriod: Infinity";
  }

  os << ")";

  return os;
}

std::shared_ptr<LsaInfo>
makeLsaInfo(const Lsa& lsa)
{
  std::shared_ptr<LsaInfo> lsaInfo = std::make_shared<LsaInfo>();

  lsaInfo->setOriginRouter(lsa.getOrigRouter());
  lsaInfo->setSequenceNumber(lsa.getLsSeqNo());

  ndn::time::system_clock::duration duration
    = lsa.getExpirationTimePoint() - ndn::time::system_clock::now();

  lsaInfo->setExpirationPeriod(ndn::time::duration_cast<ndn::time::milliseconds>(duration));

  return lsaInfo;
}

} // namespace tlv
} // namespace nlsr
