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
#ifndef NLSR_ADJACENCY_LIST_HPP
#define NLSR_ADJACENCY_LIST_HPP

#include <list>
#include <boost/cstdint.hpp>
#include <ndn-cxx/common.hpp>

#include "adjacent.hpp"

namespace nlsr {
class Nlsr;

class AdjacencyList
{

public:
  AdjacencyList();
  ~AdjacencyList();

  int32_t
  insert(Adjacent& adjacent);

  int32_t
  updateAdjacentStatus(const ndn::Name& adjName, int32_t s);

  int32_t
  updateAdjacentLinkCost(const ndn::Name& adjName, double lc);

  std::list<Adjacent>&
  getAdjList();

  bool
  isNeighbor(const ndn::Name& adjName);

  void
  incrementTimedOutInterestCount(const ndn::Name& neighbor);

  int32_t
  getTimedOutInterestCount(const ndn::Name& neighbor);

  uint32_t
  getStatusOfNeighbor(const ndn::Name& neighbor);

  void
  setStatusOfNeighbor(const ndn::Name& neighbor, int32_t status);

  void
  setTimedOutInterestCount(const ndn::Name& neighbor, uint32_t count);

  void
  addAdjacents(AdjacencyList& adl);

  bool
  isAdjLsaBuildable(Nlsr& pnlsr);

  int32_t
  getNumOfActiveNeighbor();

  Adjacent
  getAdjacent(const ndn::Name& adjName);

  bool
  operator==(AdjacencyList& adl);

  size_t
  getSize()
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

  void
  print();

  void
  writeLog();

private:
  std::list<Adjacent>::iterator
  find(const ndn::Name& adjName);

private:
  std::list<Adjacent> m_adjList;
};

} //namespace nlsr
#endif //NLSR_ADJACENCY_LIST_HPP
