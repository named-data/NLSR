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

#ifndef NLSR_LSA_ADJ_LSA_HPP
#define NLSR_LSA_ADJ_LSA_HPP

#include "lsa.hpp"
#include "test-access-control.hpp"

namespace nlsr {

/*!
   \brief Data abstraction for AdjLsa
   AdjacencyLsa := ADJACENCY-LSA-TYPE TLV-LENGTH
                     Lsa
                     Adjacency*

 */
class AdjLsa : public Lsa
{
public:
  typedef AdjacencyList::const_iterator const_iterator;

  AdjLsa() = default;

  AdjLsa(const ndn::Name& originR, uint64_t seqNo,
         const ndn::time::system_clock::TimePoint& timepoint,
         uint32_t noLink, AdjacencyList& adl);

  AdjLsa(const ndn::Block& block);

  Lsa::Type
  getType() const override
  {
    return type();
  }

  static constexpr Lsa::Type
  type()
  {
    return Lsa::Type::ADJACENCY;
  }

  const AdjacencyList&
  getAdl() const
  {
    return m_adl;
  }

  void
  resetAdl()
  {
    m_wire.reset();
    m_adl.reset();
  }

  void
  addAdjacent(Adjacent adj)
  {
    m_wire.reset();
    m_adl.insert(adj);
  }

  uint32_t
  getNoLink()
  {
    return m_noLink;
  }

  bool
  isEqualContent(const AdjLsa& alsa) const;

  const_iterator
  begin() const
  {
    return m_adl.begin();
  }

  const_iterator
  end() const
  {
    return m_adl.end();
  }

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
  uint32_t m_noLink;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  AdjacencyList m_adl;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(AdjLsa);

std::ostream&
operator<<(std::ostream& os, const AdjLsa& lsa);

} // namespace nlsr

#endif // NLSR_LSA_ADJ_LSA_HPP
