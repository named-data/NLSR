/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2018,  The University of Memphis,
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

#ifndef NLSR_TLV_COORDINATE_LSA_HPP
#define NLSR_TLV_COORDINATE_LSA_HPP

#include "lsa-info.hpp"

#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/name.hpp>

namespace nlsr {
namespace tlv {

/*!
   \brief Data abstraction for CoordinateLsa

   CoordinateLsa := COORDINATE-LSA-TYPE TLV-LENGTH
                      LsaInfo
                      HyperbolicRadius
                      HyperbolicAngle+

   \sa https://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class CoordinateLsa
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

  CoordinateLsa();

  explicit
  CoordinateLsa(const ndn::Block& block);

  const LsaInfo&
  getLsaInfo() const
  {
    return m_lsaInfo;
  }

  CoordinateLsa&
  setLsaInfo(const LsaInfo& lsaInfo)
  {
    m_lsaInfo = lsaInfo;
    m_wire.reset();
    return *this;
  }

  double
  getHyperbolicRadius() const
  {
    return m_hyperbolicRadius;
  }

  CoordinateLsa&
  setHyperbolicRadius(double hyperbolicRadius)
  {
    m_hyperbolicRadius = hyperbolicRadius;
    m_wire.reset();
    return *this;
  }

  const std::vector<double>
  getHyperbolicAngle() const
  {
    return m_hyperbolicAngle;
  }

  CoordinateLsa&
  setHyperbolicAngle(const std::vector<double>& hyperbolicAngle)
  {
    m_hyperbolicAngle = hyperbolicAngle;
    m_wire.reset();
    return *this;
  }

  /*! \brief Encodes the hyperbolic coordinates and some info using the method in TAG.
   *
   * This function will TLV-format the hyperbolic coordinates objects and some LSA
   * info using the implementation speciifed by TAG. Usually this is
   * called with an estimator first to guess how long the buffer needs
   * to be, then with an encoder to do the real work. This process is
   * automated by the other wireEncode.
   * \sa CoordinateLsa::wireEncode()
   */
  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  /*! \brief Create a TLV encoding of this object.
   *
   * Create a block containing the TLV encoding of this object. That
   * involves two steps: estimating the size that the information will
   * take up, and then creating a buffer of that size and encoding the
   * information into it. Both steps are accomplished by
   * CoordinateLsa::wireEncode(ndn::EncodingImpl<TAG>&)
   */
  const ndn::Block&
  wireEncode() const;

  /*! \brief Populate this object by decoding the one contained in the
   * given block.
   */
  void
  wireDecode(const ndn::Block& wire);

private:
  LsaInfo m_lsaInfo;
  double m_hyperbolicRadius;
  std::vector<double> m_hyperbolicAngle;

  mutable ndn::Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(CoordinateLsa);

std::ostream&
operator<<(std::ostream& os, const CoordinateLsa& coordinateLsa);

} // namespace tlv
} // namespace nlsr

#endif // NLSR_TLV_COORDINATE_LSA_HPP
