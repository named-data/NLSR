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

#ifndef NLSR_LSA_NAME_LSA_HPP
#define NLSR_LSA_NAME_LSA_HPP

#include "lsa.hpp"

namespace nlsr {

/*!
   \brief Data abstraction for NameLsa
   NameLsa := NAME-LSA-TYPE TLV-LENGTH
                Lsa
                Name+
 */
class NameLsa : public Lsa
{
public:
  NameLsa() = default;

  NameLsa(const ndn::Name& originRouter, uint64_t seqNo,
          const ndn::time::system_clock::TimePoint& timepoint,
          const NamePrefixList& npl);

  NameLsa(const ndn::Block& block);

  Lsa::Type
  getType() const override
  {
    return type();
  }

  static constexpr Lsa::Type
  type()
  {
    return Lsa::Type::NAME;
  }

  NamePrefixList&
  getNpl()
  {
    return m_npl;
  }

  const NamePrefixList&
  getNpl() const
  {
    return m_npl;
  }

  void
  addName(const ndn::Name& name)
  {
    m_wire.reset();
    m_npl.insert(name);
  }

  void
  removeName(const ndn::Name& name)
  {
    m_wire.reset();
    m_npl.remove(name);
  }

  bool
  isEqualContent(const NameLsa& other) const;

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  const ndn::Block&
  wireEncode() const override;

  void
  wireDecode(const ndn::Block& wire);

  std::string
  toString() const override;

  std::tuple<bool, std::list<ndn::Name>, std::list<ndn::Name>>
  update(const std::shared_ptr<Lsa>& lsa) override;

private:
  NamePrefixList m_npl;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(NameLsa);

std::ostream&
operator<<(std::ostream& os, const NameLsa& lsa);

} // namespace nlsr

#endif // NLSR_LSA_NAME_LSA_HPP
