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

#ifndef NLSR_LSA_LSA_HPP
#define NLSR_LSA_LSA_HPP

#include "common.hpp"
#include "name-prefix-list.hpp"
#include "test-access-control.hpp"

#include <ndn-cxx/util/scheduler.hpp>

#include <list>


namespace nlsr {

/**
 * @brief Represents a Link State Announcement (LSA).
 *
 * The base level LSA is encoded as:
 * @code{.abnf}
 * Lsa = LSA-TYPE TLV-LENGTH
 *         Name ; origin router
 *         SequenceNumber
 *         ExpirationTime
 * @endcode
 */
class Lsa
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    using ndn::tlv::Error::Error;
  };

  enum class Type {
    ADJACENCY,
    COORDINATE,
    NAME,
    BASE
  };

protected:
  Lsa() = default;

  Lsa(const ndn::Name& originRouter, uint64_t seqNo,
      ndn::time::system_clock::time_point expirationTimePoint);

  explicit
  Lsa(const Lsa& lsa);

public:
  virtual
  ~Lsa() = default;

  virtual Type
  getType() const = 0;

  void
  setSeqNo(uint64_t seqNo)
  {
    m_seqNo = seqNo;
    m_wire.reset();
  }

  uint64_t
  getSeqNo() const
  {
    return m_seqNo;
  }

  const ndn::Name&
  getOriginRouter() const
  {
    return m_originRouter;
  }

  const ndn::time::system_clock::time_point&
  getExpirationTimePoint() const
  {
    return m_expirationTimePoint;
  }

  void
  setExpirationTimePoint(const ndn::time::system_clock::time_point& lt)
  {
    m_expirationTimePoint = lt;
    m_wire.reset();
  }

  void
  setExpiringEventId(ndn::scheduler::EventId eid)
  {
    m_expiringEventId = eid;
  }

  virtual std::tuple<bool, std::list<PrefixInfo>, std::list<PrefixInfo>>
  update(const std::shared_ptr<Lsa>& lsa) = 0;

  virtual const ndn::Block&
  wireEncode() const = 0;

protected:
  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  void
  wireDecode(const ndn::Block& wire);

private:
  virtual void
  print(std::ostream& os) const = 0;

  friend std::ostream&
  operator<<(std::ostream& os, const Lsa& lsa);

PUBLIC_WITH_TESTS_ELSE_PROTECTED:
  ndn::Name m_originRouter;
  uint64_t m_seqNo = 0;
  ndn::time::system_clock::time_point m_expirationTimePoint;
  ndn::scheduler::ScopedEventId m_expiringEventId;

  mutable ndn::Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(Lsa);

std::ostream&
operator<<(std::ostream& os, const Lsa::Type& type);

std::istream&
operator>>(std::istream& is, Lsa::Type& type);

} // namespace nlsr

#endif // NLSR_LSA_LSA_HPP
