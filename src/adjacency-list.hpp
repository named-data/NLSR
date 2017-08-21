/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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
#include <boost/cstdint.hpp>

namespace nlsr {

class AdjacencyList
{
public:
  typedef std::list<Adjacent>::const_iterator const_iterator;
  typedef std::list<Adjacent>::iterator iterator;

  AdjacencyList();
  ~AdjacencyList();

  /*! \brief Inserts an adjacency into the list.

    \param adjacent The adjacency that we want to add to this list.

    \retval 0 Indicates success.
    \retval 1 Indicates failure.

    This function attempts to insert the supplied adjacency into this
    object, which is an adjacency list.
   */
  int32_t
  insert(Adjacent& adjacent);

  /*! \brief Sets the status of an adjacency.

    \param adjName The adjacency in this list you want to change the status of.

    \param s The status to change to.

    \return A boolean indicating whether an adjacency was
    updated. This is false if s is not in the list.
   */
  bool
  updateAdjacentStatus(const ndn::Name& adjName, Adjacent::Status s);

  int32_t
  updateAdjacentLinkCost(const ndn::Name& adjName, double lc);

  std::list<Adjacent>&
  getAdjList();

  const std::list<Adjacent>&
  getAdjList() const;

  bool
  isNeighbor(const ndn::Name& adjName);

  void
  incrementTimedOutInterestCount(const ndn::Name& neighbor);

  int32_t
  getTimedOutInterestCount(const ndn::Name& neighbor);

  Adjacent::Status
  getStatusOfNeighbor(const ndn::Name& neighbor);

  void
  setStatusOfNeighbor(const ndn::Name& neighbor, Adjacent::Status status);

  void
  setTimedOutInterestCount(const ndn::Name& neighbor, uint32_t count);

  /*! \brief Copies the adjacencies in a list to this one.

    \param adl The adjacency list, the entries of which we want to
    copy into this object.

    Copies the entries contained in one list into this object.
   */
  void
  addAdjacents(AdjacencyList& adl);

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
  getNumOfActiveNeighbor();

  Adjacent
  getAdjacent(const ndn::Name& adjName);

  bool
  operator==(AdjacencyList& adl) const;

  size_t
  size() const
  {
    return m_adjList.size();
  }

  void
  reset()
  {
    if (m_adjList.size() > 0) {
      m_adjList.clear();
    }
  }

  AdjacencyList::iterator
  findAdjacent(const ndn::Name& adjName);

  AdjacencyList::iterator
  findAdjacent(uint64_t faceId);

  AdjacencyList::iterator
  findAdjacent(const ndn::FaceUri& faceUri);

  /*! \brief Hack to stop developers from using this function

    It is here so that faceUri cannot be passed in as string,
    converted to Name and findAdjacent(Name) be used.
    So when faceUri is passed as string this will cause a compile error
   */
  template <typename T = float> void
  findAdjacent(const std::string& faceUri)
  {
    BOOST_STATIC_ASSERT_MSG(std::is_integral<T>::value,
      "Don't use std::string with findAdjacent!");
  }

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

private:
  std::list<Adjacent> m_adjList;
};

} // namespace nlsr
#endif // NLSR_ADJACENCY_LIST_HPP
