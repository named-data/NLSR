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

#ifndef NLSR_LSA_NAME_LSA_HPP
#define NLSR_LSA_NAME_LSA_HPP

#include "lsa.hpp"
#include "name-prefix-list.hpp"

#include <boost/operators.hpp>

namespace nlsr {

/**
 * @brief Represents an LSA of name prefixes announced by the origin router.
 *
 * NameLsa is encoded as:
 * @code{.abnf}
 * NameLsa = NAME-LSA-TYPE TLV-LENGTH
 *             Lsa
 *             1*Name
 * @endcode
 */
class NameLsa : public Lsa, private boost::equality_comparable<NameLsa>
{
public:
  NameLsa() = default;

  NameLsa(const ndn::Name& originRouter, uint64_t seqNo,
          const ndn::time::system_clock::time_point& timepoint,
          const NamePrefixList& npl);

  explicit
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
  addName(const PrefixInfo& name)
  {
    m_wire.reset();
    m_npl.insert(name);
  }

  void
  removeName(const PrefixInfo& name)
  {
    m_wire.reset();
    m_npl.erase(name.getName());
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
  operator==(const NameLsa& lhs, const NameLsa& rhs)
  {
    return lhs.m_npl == rhs.m_npl;
  }

private:
  NamePrefixList m_npl;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(NameLsa);

} // namespace nlsr

#endif // NLSR_LSA_NAME_LSA_HPP
