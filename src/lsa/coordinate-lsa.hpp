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

#ifndef NLSR_LSA_COORDINATE_LSA_HPP
#define NLSR_LSA_COORDINATE_LSA_HPP

#include "lsa.hpp"
#include "utility/numeric.hpp"

#include <boost/operators.hpp>

namespace nlsr {

/**
 * @brief Represents an LSA of hyperbolic coordinates of the origin router.
 *
 * CoordinateLsa is encoded as:
 * @code{.abnf}
 * CoordinateLsa = COORDINATE-LSA-TYPE TLV-LENGTH
 *                   Lsa
 *                   HyperbolicRadius
 *                   1*HyperbolicAngle ; theta
 *
 * HyperbolicRadius = HYPERBOLIC-RADIUS-TYPE TLV-LENGTH
 *                      Double ; IEEE754 double precision
 *
 * HyperbolicAngle = HYPERBOLIC-ANGLE-TYPE TLV-LENGTH
 *                     Double ; IEEE754 double precision
 * @endcode
 */
class CoordinateLsa : public Lsa, private boost::equality_comparable<CoordinateLsa>
{
public:
  CoordinateLsa() = default;

  CoordinateLsa(const ndn::Name& originRouter, uint64_t seqNo,
                const ndn::time::system_clock::time_point& timepoint,
                double radius, std::vector<double> angles);

  explicit
  CoordinateLsa(const ndn::Block& block);

  Lsa::Type
  getType() const override
  {
    return type();
  }

  static constexpr Lsa::Type
  type()
  {
    return Lsa::Type::COORDINATE;
  }

  double
  getRadius() const
  {
    return m_hyperbolicRadius;
  }

  void
  setRadius(double cr)
  {
    m_wire.reset();
    m_hyperbolicRadius = cr;
  }

  const std::vector<double>&
  getTheta() const
  {
    return m_hyperbolicAngles;
  }

  void
  setTheta(std::vector<double> ct)
  {
    m_wire.reset();
    m_hyperbolicAngles = std::move(ct);
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
  operator==(const CoordinateLsa& lhs, const CoordinateLsa& rhs)
  {
    return util::diffInEpsilon(lhs.m_hyperbolicRadius, rhs.m_hyperbolicRadius) &&
           std::equal(lhs.m_hyperbolicAngles.begin(), lhs.m_hyperbolicAngles.end(),
                      rhs.m_hyperbolicAngles.begin(), rhs.m_hyperbolicAngles.end(),
                      util::diffInEpsilon);
  }

private:
  double m_hyperbolicRadius = 0.0;
  std::vector<double> m_hyperbolicAngles;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(CoordinateLsa);

} // namespace nlsr

#endif // NLSR_LSA_COORDINATE_LSA_HPP
