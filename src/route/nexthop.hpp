/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
 *                           Regents of the University of California
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

#ifndef NLSR_ROUTE_NEXTHOP_HPP
#define NLSR_ROUTE_NEXTHOP_HPP

#include "test-access-control.hpp"

#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv.hpp>

#include <cmath>
#include <ostream>

namespace nlsr {

/*! \brief Data abstraction for Nexthop
 *
 *   NextHop := NEXTHOP-TYPE TLV-LENGTH
 *                Uri
 *                Cost
 *
 * \sa https://redmine.named-data.net/projects/nlsr/wiki/Routing_Table_Dataset
 */
class NextHop
{
public:
  using Error = ndn::tlv::Error;

  NextHop()
    : m_connectingFaceUri()
    , m_routeCost(0)
    , m_isHyperbolic(false)
  {
  }

  NextHop(const std::string& cfu, double rc)
    : m_connectingFaceUri(cfu)
    , m_routeCost(rc)
    , m_isHyperbolic(false)
  {
  }

  NextHop(const ndn::Block& block)
  {
    wireDecode(block);
  }

  const std::string&
  getConnectingFaceUri() const
  {
    return m_connectingFaceUri;
  }

  void
  setConnectingFaceUri(const std::string& cfu)
  {
    m_connectingFaceUri = cfu;
  }

  uint64_t
  getRouteCostAsAdjustedInteger() const
  {
    if (m_isHyperbolic) {
      // Round the cost to better preserve decimal cost differences
      // e.g. Without rounding: 12.3456 > 12.3454 -> 12345 = 12345
      //      With rounding:    12.3456 > 12.3454 -> 12346 > 12345
      return static_cast<uint64_t>(round(m_routeCost*HYPERBOLIC_COST_ADJUSTMENT_FACTOR));
    }
    else {
      return static_cast<uint64_t>(m_routeCost);
    }
  }

  double
  getRouteCost() const
  {
    return m_routeCost;
  }

  void
  setRouteCost(const double rc)
  {
    m_routeCost = rc;
  }

  void
  setHyperbolic(bool b)
  {
    m_isHyperbolic = b;
  }

  bool
  isHyperbolic() const
  {
    return m_isHyperbolic;
  }

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  const ndn::Block&
  wireEncode() const;

  void
  wireDecode(const ndn::Block& wire);

private:
  std::string m_connectingFaceUri;
  double m_routeCost;
  bool m_isHyperbolic;

  mutable ndn::Block m_wire;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  /*! \brief Used to adjust floating point route costs to integers
      Since NFD uses integer route costs in the FIB, hyperbolic paths with similar route costs
      will be rounded to integers and installed with identical nexthop costs.

      e.g. costs 12.34 and 12.35 will be equal in NFD's FIB

      This multiplier is used to differentiate similar route costs in the FIB.

      e.g  costs 12.34 and 12.35 will be installed into NFD's FIB as 12340 and 12350
  */
  static const uint64_t HYPERBOLIC_COST_ADJUSTMENT_FACTOR = 1000;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(NextHop);

bool
operator==(const NextHop& lhs, const NextHop& rhs);

std::ostream&
operator<<(std::ostream& os, const NextHop& hop);

} // namespace nlsr

#endif // NLSR_ROUTE_NEXTHOP_HPP
