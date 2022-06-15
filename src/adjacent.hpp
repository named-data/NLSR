/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022,  The University of Memphis,
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

#ifndef NLSR_ADJACENT_HPP
#define NLSR_ADJACENT_HPP

#include <cmath>
#include <string>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/net/face-uri.hpp>

namespace nlsr {

/*! \brief A neighbor reachable over a Face.
 *
 * Represents another node that we expect to be running NLSR that we
 * should be able to reach over a direct Face connection.
 *
 * Data abstraction for Adjacent
 *  Adjacent := ADJACENCY-TYPE TLV-LENGTH
 *               Name
 *               FaceUri
 *               LinkCost
 *               AdjacencyStatus
 *               AdjacencyInterestTimedOutNo
 */
class Adjacent
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    using ndn::tlv::Error::Error;
  };

  enum Status
  {
    STATUS_UNKNOWN = -1,
    STATUS_INACTIVE = 0,
    STATUS_ACTIVE = 1
  };

  Adjacent();

  Adjacent(const ndn::Block& block);

  Adjacent(const ndn::Name& an);

  Adjacent(const ndn::Name& an, const ndn::FaceUri& faceUri, double lc,
           Status s, uint32_t iton, uint64_t faceId);

  const ndn::Name&
  getName() const
  {
    return m_name;
  }

  void
  setName(const ndn::Name& name)
  {
    m_wire.reset();
    m_name = name;
  }

  const ndn::FaceUri&
  getFaceUri() const
  {
    return m_faceUri;
  }

  void
  setFaceUri(const ndn::FaceUri& faceUri)
  {
    m_wire.reset();
    m_faceUri = faceUri;
  }

  double
  getLinkCost() const
  {
    return ceil(m_linkCost);
  }

  void
  setLinkCost(double lc);

  Status
  getStatus() const
  {
    return m_status;
  }

  void
  setStatus(Status s)
  {
    m_wire.reset();
    m_status = s;
  }

  uint32_t
  getInterestTimedOutNo() const
  {
    return m_interestTimedOutNo;
  }

  void
  setInterestTimedOutNo(uint32_t iton)
  {
    m_wire.reset();
    m_interestTimedOutNo = iton;
  }

  void
  setFaceId(uint64_t faceId)
  {
    m_faceId = faceId;
  }

  uint64_t
  getFaceId() const
  {
    return m_faceId;
  }

  /*! \brief Equality is when name, Face URI, and link cost are all equal. */
  bool
  operator==(const Adjacent& adjacent) const;

  bool
  operator!=(const Adjacent& adjacent) const
  {
    return !(*this == adjacent);
  }

  bool
  operator<(const Adjacent& adjacent) const;

  inline bool
  compare(const ndn::Name& adjacencyName) const
  {
    return m_name == adjacencyName;
  }

  inline bool
  compareFaceId(const uint64_t faceId) const
  {
    return m_faceId == faceId;
  }

  inline bool
  compareFaceUri(const ndn::FaceUri& faceUri) const
  {
    return m_faceUri == faceUri;
  }

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  const ndn::Block&
  wireEncode() const;

  void
  wireDecode(const ndn::Block& wire);

public:
  static constexpr double DEFAULT_LINK_COST = 10.0;
  static constexpr double NON_ADJACENT_COST = -12345.0;

private:
  /*! m_name The NLSR-configured router name of the neighbor */
  ndn::Name m_name;
  /*! m_faceUri The NFD-level specification of the Face*/
  ndn::FaceUri m_faceUri;
  /*! m_linkCost The semi-arbitrary cost to traverse the link. */
  double m_linkCost;
  /*! m_status Whether the neighbor is active or not */
  Status m_status = STATUS_UNKNOWN;
  /*! m_interestTimedOutNo How many failed Hello interests we have sent since the last reply */
  uint32_t m_interestTimedOutNo = 0;
  /*! m_faceId The NFD-assigned ID for the neighbor, used to
   * determine whether a Face is available */
  uint64_t m_faceId;

  mutable ndn::Block m_wire;

  friend std::ostream&
  operator<<(std::ostream& os, const Adjacent& adjacent);
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(Adjacent);

std::ostream&
operator<<(std::ostream& os, const Adjacent& adjacent);

} // namespace nlsr

#endif // NLSR_ADJACENT_HPP
