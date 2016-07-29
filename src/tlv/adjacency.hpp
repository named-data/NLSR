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

#ifndef NLSR_TLV_ADJACENCY_HPP
#define NLSR_TLV_ADJACENCY_HPP

#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/name.hpp>

namespace nlsr {
namespace tlv  {

/**
 * @brief Data abstraction for Adjacency
 *
 * Adjacency := ADJACENCY-TYPE TLV-LENGTH
 *                Name
 *                Uri
 *                Cost
 *
 * @sa http://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class Adjacency
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

  Adjacency();

  explicit
  Adjacency(const ndn::Block& block);

  const ndn::Name&
  getName() const
  {
    return m_name;
  }

  Adjacency&
  setName(const ndn::Name& name)
  {
    m_name = name;
    m_wire.reset();
    return *this;
  }

  const std::string&
  getUri() const
  {
    return m_uri;
  }

  Adjacency&
  setUri(const std::string& uri)
  {
    m_uri = uri;
    m_wire.reset();
    return *this;
  }

  uint64_t
  getCost() const
  {
    return m_cost;
  }

  Adjacency&
  setCost(uint64_t cost)
  {
    m_cost = cost;
    m_wire.reset();
    return *this;
  }

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  const ndn::Block&
  wireEncode() const;

  void
  wireDecode(const ndn::Block& wire);

private:
  ndn::Name m_name;
  std::string m_uri;
  uint64_t m_cost;

  mutable ndn::Block m_wire;
};

std::ostream&
operator<<(std::ostream& os, const Adjacency& adjacency);

} // namespace tlv
} // namespace nlsr

#endif // NLSR_TLV_ADJACENCY_HPP
