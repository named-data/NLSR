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

#ifndef NLSR_LSA_COORDINATE_LSA_HPP
#define NLSR_LSA_COORDINATE_LSA_HPP

#include "lsa.hpp"

namespace nlsr {

/*!
   \brief Data abstraction for CoordinateLsa
   CoordinateLsa := COORDINATE-LSA-TYPE TLV-LENGTH
                      Lsa
                      HyperbolicRadius
                      HyperbolicAngle+
 */
class CoordinateLsa : public Lsa
{
public:
  CoordinateLsa() = default;

  CoordinateLsa(const ndn::Name& originRouter, uint64_t seqNo,
                const ndn::time::system_clock::TimePoint& timepoint,
                double radius, std::vector<double> angles);

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
  getCorRadius() const
  {
    return m_hyperbolicRadius;
  }

  void
  setCorRadius(double cr)
  {
    m_wire.reset();
    m_hyperbolicRadius = cr;
  }

  const std::vector<double>
  getCorTheta() const
  {
    return m_hyperbolicAngles;
  }

  void
  setCorTheta(std::vector<double> ct)
  {
    m_wire.reset();
    m_hyperbolicAngles = ct;
  }

  bool
  isEqualContent(const CoordinateLsa& clsa) const;

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
  double m_hyperbolicRadius = 0.0;
  std::vector<double> m_hyperbolicAngles;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(CoordinateLsa);

std::ostream&
operator<<(std::ostream& os, const CoordinateLsa& lsa);

} // namespace nlsr

#endif // NLSR_LSA_COORDINATE_LSA_HPP
