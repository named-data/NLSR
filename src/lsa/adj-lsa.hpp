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

#ifndef NLSR_LSA_ADJ_LSA_HPP
#define NLSR_LSA_ADJ_LSA_HPP

#include "lsa.hpp"
#include "adjacent.hpp"
#include "adjacency-list.hpp"

#include <boost/operators.hpp>

namespace nlsr {

/**
 * @brief Represents an LSA of adjacencies of the origin router in link-state mode.
 *
 * AdjLsa is encoded as:
 * @code{.abnf}
 * AdjLsa = ADJACENCY-LSA-TYPE TLV-LENGTH
 *            Lsa
 *            *Adjacency
 * @endcode
 */
class AdjLsa : public Lsa, private boost::equality_comparable<AdjLsa>
{
public:
  using const_iterator = AdjacencyList::const_iterator;

  AdjLsa() = default;

  AdjLsa(const ndn::Name& originR, uint64_t seqNo,
         const ndn::time::system_clock::time_point& timepoint, AdjacencyList& adl);

  explicit
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
  addAdjacent(const Adjacent& adj)
  {
    m_wire.reset();
    m_adl.insert(adj);
  }

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

  std::tuple<bool, std::list<PrefixInfo>, std::list<PrefixInfo>>
  update(const std::shared_ptr<Lsa>& lsa) override;

private:
  void
  print(std::ostream& os) const override;

private: // non-member operators
  // NOTE: the following "hidden friend" operators are available via
  //       argument-dependent lookup only and must be defined inline.
  // boost::equality_comparable provides != operator.

  friend bool
  operator==(const AdjLsa& lhs, const AdjLsa& rhs)
  {
    return lhs.m_adl == rhs.m_adl;
  }

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  AdjacencyList m_adl;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(AdjLsa);

} // namespace nlsr

#endif // NLSR_LSA_ADJ_LSA_HPP
