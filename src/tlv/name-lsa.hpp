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

#ifndef NLSR_TLV_NAME_LSA_HPP
#define NLSR_TLV_NAME_LSA_HPP

#include "lsa-info.hpp"

#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/name.hpp>

#include <list>

namespace nlsr {
namespace tlv  {

/**
 * @brief Data abstraction for NameLsa
 *
 * NameLsa := NAME-LSA-TYPE TLV-LENGTH
 *              LsaInfo
 *              Name+
 *
 * @sa http://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class NameLsa
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

  typedef std::list<ndn::Name> NameList;
  typedef NameList::const_iterator iterator;

  NameLsa();

  explicit
  NameLsa(const ndn::Block& block);

  const LsaInfo&
  getLsaInfo() const
  {
    return m_lsaInfo;
  }

  NameLsa&
  setLsaInfo(const LsaInfo& lsaInfo)
  {
    m_lsaInfo = lsaInfo;
    m_wire.reset();
    return *this;
  }

  bool
  hasNames() const
  {
    return m_hasNames;
  }

  const std::list<ndn::Name>&
  getNames() const
  {
    return m_names;
  }

  NameLsa&
  addName(const ndn::Name& name)
  {
    m_names.push_back(name);
    m_wire.reset();
    m_hasNames = true;
    return *this;
  }

  NameLsa&
  clearNames()
  {
    m_names.clear();
    m_hasNames = false;
    return *this;
  }

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  const ndn::Block&
  wireEncode() const;

  void
  wireDecode(const ndn::Block& wire);

  iterator
  begin() const;

  iterator
  end() const;

private:
  LsaInfo m_lsaInfo;
  bool m_hasNames;
  NameList m_names;

  mutable ndn::Block m_wire;
};

inline NameLsa::iterator
NameLsa::begin() const
{
  return m_names.begin();
}

inline NameLsa::iterator
NameLsa::end() const
{
  return m_names.end();
}

std::ostream&
operator<<(std::ostream& os, const NameLsa& nameLsa);

} // namespace tlv
} // namespace nlsr

#endif // NLSR_TLV_NAME_LSA_HPP
