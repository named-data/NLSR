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

#ifndef NLSR_TLV_LSDB_STATUS_HPP
#define NLSR_TLV_LSDB_STATUS_HPP

#include "adjacency-lsa.hpp"
#include "coordinate-lsa.hpp"
#include "name-lsa.hpp"

#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/name.hpp>

#include <list>

namespace nlsr {
namespace tlv  {

/**
 * @brief Data abstraction for LsdbStatus
 *
 * LsdbStatus := LSDB-STATUS-TYPE TLV-LENGTH
 *                 AdjacencyLsa*
 *                 CoordinateLsa*
 *                 NameLsa*
 *
 * @sa http://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class LsdbStatus
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

  typedef std::list<AdjacencyLsa> AdjacencyLsaList;
  typedef std::list<CoordinateLsa> CoordinateLsaList;
  typedef std::list<NameLsa> NameLsaList;

  LsdbStatus();

  explicit
  LsdbStatus(const ndn::Block& block);

  const std::list<AdjacencyLsa>&
  getAdjacencyLsas() const
  {
    return m_adjacencyLsas;
  }

  LsdbStatus&
  addAdjacencyLsa(const AdjacencyLsa& adjacencyLsa)
  {
    m_adjacencyLsas.push_back(adjacencyLsa);
    m_wire.reset();
    m_hasAdjacencyLsas = true;
    return *this;
  }

  LsdbStatus&
  clearAdjacencyLsas()
  {
    m_adjacencyLsas.clear();
    m_hasAdjacencyLsas = false;
    return *this;
  }

  bool
  hasAdjacencyLsas()
  {
    return m_hasAdjacencyLsas;
  }

  const std::list<CoordinateLsa>&
  getCoordinateLsas() const
  {
    return m_coordinateLsas;
  }

  LsdbStatus&
  addCoordinateLsa(const CoordinateLsa& coordinateLsa)
  {
    m_coordinateLsas.push_back(coordinateLsa);
    m_wire.reset();
    m_hasCoordinateLsas = true;
    return *this;
  }

  LsdbStatus&
  clearCoordinateLsas()
  {
    m_coordinateLsas.clear();
    m_hasCoordinateLsas = false;
    return *this;
  }

  bool
  hasCoordinateLsas()
  {
    return m_hasCoordinateLsas;
  }

  const std::list<NameLsa>&
  getNameLsas() const
  {
    return m_nameLsas;
  }

  LsdbStatus&
  addNameLsa(const NameLsa& nameLsa)
  {
    m_nameLsas.push_back(nameLsa);
    m_wire.reset();
    m_hasNameLsas = true;
    return *this;
  }

  LsdbStatus&
  clearNameLsas()
  {
    m_nameLsas.clear();
    m_hasNameLsas = false;
    return *this;
  }

  bool
  hasNameLsas()
  {
    return m_hasNameLsas;
  }

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  const ndn::Block&
  wireEncode() const;

  void
  wireDecode(const ndn::Block& wire);

private:
  AdjacencyLsaList m_adjacencyLsas;
  CoordinateLsaList m_coordinateLsas;
  NameLsaList m_nameLsas;

  bool m_hasAdjacencyLsas;
  bool m_hasCoordinateLsas;
  bool m_hasNameLsas;

  mutable ndn::Block m_wire;
};

std::ostream&
operator<<(std::ostream& os, const LsdbStatus& lsdbStatus);

} // namespace tlv
} // namespace nlsr

#endif // NLSR_TLV_LSDB_STATUS_HPP
