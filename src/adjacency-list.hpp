/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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
 **/

#ifndef NLSR_ADJACENCY_LIST_HPP
#define NLSR_ADJACENCY_LIST_HPP

#include "adjacent.hpp"
#include "common.hpp"

#include <list>

namespace nlsr {

class AdjacencyList
{
public:
  using const_iterator = std::list<Adjacent>::const_iterator;
  using iterator = std::list<Adjacent>::iterator;

  bool
  insert(const Adjacent& adjacent);

  std::list<Adjacent>&
  getAdjList();

  const std::list<Adjacent>&
  getAdjList() const;

  bool
  isNeighbor(const ndn::Name& adjName) const;

  void
  incrementTimedOutInterestCount(const ndn::Name& neighbor);

  int32_t
  getTimedOutInterestCount(const ndn::Name& neighbor) const;

  Adjacent::Status
  getStatusOfNeighbor(const ndn::Name& neighbor) const;

  void
  setStatusOfNeighbor(const ndn::Name& neighbor, Adjacent::Status status);

  void
  setTimedOutInterestCount(const ndn::Name& neighbor, uint32_t count);

  /*! \brief Determines whether this list can be used to build an adj. LSA.
    \param interestRetryNo The maximum number of hello-interest
      retries to contact a neighbor.

    \return Returns a boolean indicating whether this list can be used
    to build an adj. LSA.

    Determines whether this adjacency list object could be used to
    build an adjacency LSA. An LSA is buildable when the status of all
    neighbors is known. A neighbor's status is known when their status
    is ACTIVE, or INACTIVE and some number of hello interests
    (specified by nlsr::ConfParameter::getInterestRetryNumber()) have
    failed. To be explicit, a neighbor's status is unknown if we are
    still sending hello interests.
   */
  bool
  isAdjLsaBuildable(const uint32_t interestRetryNo) const;

  int32_t
  getNumOfActiveNeighbor() const;

  Adjacent
  getAdjacent(const ndn::Name& adjName) const;

  bool
  operator==(const AdjacencyList& adl) const;

  size_t
  size() const
  {
    return m_adjList.size();
  }

  void
  reset()
  {
    m_adjList.clear();
  }

  AdjacencyList::iterator
  findAdjacent(const ndn::Name& adjName);

  AdjacencyList::iterator
  findAdjacent(uint64_t faceId);

  AdjacencyList::iterator
  findAdjacent(const ndn::FaceUri& faceUri);

  uint64_t
  getFaceId(const ndn::FaceUri& faceUri);

  void
  writeLog();

public:
  const_iterator
  begin() const
  {
    return m_adjList.begin();
  }

  const_iterator
  end() const
  {
    return m_adjList.end();
  }

private:
  iterator
  find(const ndn::Name& adjName);

  const_iterator
  find(const ndn::Name& adjName) const;

private:
  std::list<Adjacent> m_adjList;
};

} // namespace nlsr
#endif // NLSR_ADJACENCY_LIST_HPP
