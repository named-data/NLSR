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

#ifndef NLSR_TLV_LSA_INFO_HPP
#define NLSR_TLV_LSA_INFO_HPP

#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/name.hpp>

#include "lsa.hpp"

namespace nlsr {
namespace tlv  {

/**
 * @brief Data abstraction for LsaInfo
 *
 * LsaInfo := LSA-TYPE TLV-LENGTH
 *              OriginRouter
 *              SequenceNumber
 *              ExpirationPeriod?
 *
 * @sa http://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class LsaInfo
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : ndn::tlv::Error(what)
    {
    }
  };

  LsaInfo();

  explicit
  LsaInfo(const ndn::Block& block);

  const ndn::Name&
  getOriginRouter() const
  {
    return m_originRouter;
  }

  LsaInfo&
  setOriginRouter(const ndn::Name& name)
  {
    m_originRouter = name;
    m_wire.reset();
    return *this;
  }

  uint64_t
  getSequenceNumber() const
  {
    return m_sequenceNumber;
  }

  LsaInfo&
  setSequenceNumber(uint64_t sequenceNumber)
  {
    m_sequenceNumber = sequenceNumber;
    m_wire.reset();
    return *this;
  }

  static const ndn::time::milliseconds INFINITE_EXPIRATION_PERIOD;

  const ndn::time::milliseconds&
  getExpirationPeriod() const
  {
    return m_expirationPeriod;
  }

  LsaInfo&
  setExpirationPeriod(const ndn::time::milliseconds& expirationPeriod)
  {
    m_expirationPeriod = expirationPeriod;

    m_hasInfiniteExpirationPeriod = (m_expirationPeriod == INFINITE_EXPIRATION_PERIOD);

    m_wire.reset();
    return *this;
  }

  bool
  hasInfiniteExpirationPeriod() const
  {
    return m_hasInfiniteExpirationPeriod;
  }

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  const ndn::Block&
  wireEncode() const;

  void
  wireDecode(const ndn::Block& wire);

private:
  ndn::Name m_originRouter;
  uint64_t m_sequenceNumber;
  ndn::time::milliseconds m_expirationPeriod;
  bool m_hasInfiniteExpirationPeriod;

  mutable ndn::Block m_wire;
};

std::ostream&
operator<<(std::ostream& os, const LsaInfo& lsaInfo);

std::shared_ptr<LsaInfo>
makeLsaInfo(const Lsa& lsa);

} // namespace tlv
} // namespace nlsr

#endif // NLSR_TLV_LSA_INFO_HPP
