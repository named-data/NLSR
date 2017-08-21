/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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
 **/

#include <string>
#include <cmath>
#include <boost/cstdint.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/net/face-uri.hpp>

#ifndef NLSR_ADJACENT_HPP
#define NLSR_ADJACENT_HPP

namespace nlsr {

/*! \brief A neighbor reachable over a Face.
 *
 * Represents another node that we expect to be running NLSR that we
 * should be able to reach over a direct Face connection.
 */
class Adjacent
{

public:
  enum Status
  {
    STATUS_UNKNOWN = -1,
    STATUS_INACTIVE = 0,
    STATUS_ACTIVE = 1
  };

  Adjacent();

  Adjacent(const ndn::Name& an);

  Adjacent(const ndn::Name& an, const ndn::FaceUri& faceUri, double lc,
           Status s, uint32_t iton, uint64_t faceId);

  const ndn::Name&
  getName() const
  {
    return m_name;
  }

  void
  setName(const ndn::Name& an)
  {
    m_name = an;
  }

  const ndn::FaceUri&
  getFaceUri() const
  {
    return m_faceUri;
  }

  void
  setFaceUri(const ndn::FaceUri& faceUri)
  {
    m_faceUri = faceUri;
  }

  uint64_t
  getLinkCost() const
  {
    uint64_t linkCost = static_cast<uint64_t>(ceil(m_linkCost));
    return linkCost;
  }

  void
  setLinkCost(double lc)
  {
    m_linkCost = lc;
  }

  Status
  getStatus() const
  {
    return m_status;
  }

  void
  setStatus(Status s)
  {
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
  compare(const ndn::Name& adjacencyName)
  {
    return m_name == adjacencyName;
  }

  inline bool
  compareFaceId(const uint64_t faceId)
  {
    return m_faceId == faceId;
  }

  inline bool
  compareFaceUri(const ndn::FaceUri& faceUri)
  {
    return m_faceUri == faceUri;
  }

  void
  writeLog();

public:
  static const float DEFAULT_LINK_COST;

private:
  /*! m_name The NLSR-configured router name of the neighbor */
  ndn::Name m_name;
  /*! m_faceUri The NFD-level specification of the Face*/
  ndn::FaceUri m_faceUri;
  /*! m_linkCost The semi-arbitrary cost to traverse the link. */
  double m_linkCost;
  /*! m_status Whether the neighbor is active or not */
  Status m_status;
  /*! m_interestTimedOutNo How many failed Hello interests we have sent since the last reply */
  uint32_t m_interestTimedOutNo;
  /*! m_faceId The NFD-assigned ID for the neighbor, used to
   * determine whether a Face is available */
  uint64_t m_faceId;

  friend std::ostream&
  operator<<(std::ostream& os, const Adjacent& adjacent);
};

std::ostream&
operator<<(std::ostream& os, const Adjacent& adjacent);

} // namespace nlsr

#endif // NLSR_ADJACENT_HPP
