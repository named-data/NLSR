/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#include <string>
#include <cmath>
#include <boost/cstdint.hpp>
#include <ndn-cxx/face.hpp>

#ifndef NLSR_ADJACENT_HPP
#define NLSR_ADJACENT_HPP

namespace nlsr {

enum {
  ADJACENT_STATUS_INACTIVE = 0,
  ADJACENT_STATUS_ACTIVE = 1
};

class Adjacent
{

public:
  Adjacent();

  Adjacent(const ndn::Name& an);

  Adjacent(const ndn::Name& an, const std::string& cfu,  double lc,
          uint32_t s, uint32_t iton, uint64_t faceId);

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

  uint32_t
  getStatus() const
  {
    return m_status;
  }

  void
  setStatus(uint32_t s)
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
  getFaceId()
  {
    return m_faceId;
  }

  bool
  operator==(const Adjacent& adjacent) const;

  inline bool
  compare(const ndn::Name& adjacencyName)
  {
    return m_name == adjacencyName;
  }

  inline bool
  compareFaceId(uint64_t faceId)
  {
    return m_faceId == faceId;
  }

  inline bool
  compareFaceUri(std::string& faceUri)
  {
    return m_connectingFaceUri == faceUri;
  }

  void
  writeLog();

public:
  static const float DEFAULT_LINK_COST;

private:
  ndn::Name m_name;
  std::string m_connectingFaceUri;
  double m_linkCost;
  uint32_t m_status;
  uint32_t m_interestTimedOutNo;
  uint64_t m_faceId;
};

} // namespace nlsr

#endif //NLSR_ADJACENT_HPP
