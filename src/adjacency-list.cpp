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

#include "adjacency-list.hpp"
#include "logger.hpp"

#include <algorithm>

namespace nlsr {

INIT_LOGGER(AdjacencyList);

bool
AdjacencyList::insert(const Adjacent& adjacent)
{
  auto it = find(adjacent.getName());
  if (it != m_adjList.end()) {
    return false;
  }
  m_adjList.push_back(adjacent);
  return true;
}

Adjacent
AdjacencyList::getAdjacent(const ndn::Name& adjName) const
{
  Adjacent adj(adjName);
  auto it = find(adjName);
  if (it != m_adjList.end()) {
    return *it;
  }
  return adj;
}

bool
AdjacencyList::operator==(const AdjacencyList& adl) const
{
  auto theirList = adl.getAdjList();
  if (m_adjList.size() != theirList.size()) {
    return false;
  }

  std::set<Adjacent> ourSet(m_adjList.cbegin(), m_adjList.cend());
  std::set<Adjacent> theirSet(theirList.cbegin(), theirList.cend());
  return ourSet == theirSet;
}

bool
AdjacencyList::isNeighbor(const ndn::Name& adjName) const
{
  return find(adjName) != m_adjList.end();
}

void
AdjacencyList::incrementTimedOutInterestCount(const ndn::Name& neighbor)
{
  auto it = find(neighbor);
  if (it != m_adjList.end()) {
    it->setInterestTimedOutNo(it->getInterestTimedOutNo() + 1);
  }
}

void
AdjacencyList::setTimedOutInterestCount(const ndn::Name& neighbor, uint32_t count)
{
  auto it = find(neighbor);
  if (it != m_adjList.end()) {
    it->setInterestTimedOutNo(count);
  }
}

int32_t
AdjacencyList::getTimedOutInterestCount(const ndn::Name& neighbor) const
{
  auto it = find(neighbor);
  if (it == m_adjList.end()) {
    return -1;
  }
  return it->getInterestTimedOutNo();
}

Adjacent::Status
AdjacencyList::getStatusOfNeighbor(const ndn::Name& neighbor) const
{
  auto it = find(neighbor);
  if (it == m_adjList.end()) {
    return Adjacent::STATUS_UNKNOWN;
  }
  else {
    return it->getStatus();
  }
}

void
AdjacencyList::setStatusOfNeighbor(const ndn::Name& neighbor, Adjacent::Status status)
{
  auto it = find(neighbor);
  if (it != m_adjList.end()) {
    it->setStatus(status);
  }
}

std::list<Adjacent>&
AdjacencyList::getAdjList()
{
  return m_adjList;
}

const std::list<Adjacent>&
AdjacencyList::getAdjList() const
{
  return m_adjList;
}

bool
AdjacencyList::isAdjLsaBuildable(const uint32_t interestRetryNo) const
{
  uint32_t nTimedOutNeighbors = 0;

  for (const auto& adjacency : m_adjList) {
    if (adjacency.getStatus() == Adjacent::STATUS_ACTIVE) {
      return true;
    }
    else if (adjacency.getInterestTimedOutNo() >= interestRetryNo) {
      nTimedOutNeighbors++;
    }
  }

  return nTimedOutNeighbors == m_adjList.size();
}

int32_t
AdjacencyList::getNumOfActiveNeighbor() const
{
  int32_t actNbrCount = 0;
  for (const auto& adjacent: m_adjList) {
    if (adjacent.getStatus() == Adjacent::STATUS_ACTIVE) {
      actNbrCount++;
    }
  }
  return actNbrCount;
}

std::list<Adjacent>::iterator
AdjacencyList::find(const ndn::Name& adjName)
{
  return std::find_if(m_adjList.begin(), m_adjList.end(),
                      std::bind(&Adjacent::compare, _1, std::cref(adjName)));
}

std::list<Adjacent>::const_iterator
AdjacencyList::find(const ndn::Name& adjName) const
{
  return std::find_if(m_adjList.cbegin(), m_adjList.cend(),
                      std::bind(&Adjacent::compare, _1, std::cref(adjName)));
}

AdjacencyList::iterator
AdjacencyList::findAdjacent(const ndn::Name& adjName)
{
  return std::find_if(m_adjList.begin(), m_adjList.end(),
                      std::bind(&Adjacent::compare, _1, std::cref(adjName)));
}

AdjacencyList::iterator
AdjacencyList::findAdjacent(uint64_t faceId)
{
  return std::find_if(m_adjList.begin(), m_adjList.end(),
                      std::bind(&Adjacent::compareFaceId, _1, faceId));
}

AdjacencyList::iterator
AdjacencyList::findAdjacent(const ndn::FaceUri& faceUri)
{
  return std::find_if(m_adjList.begin(), m_adjList.end(),
                      std::bind(&Adjacent::compareFaceUri, _1, faceUri));
}

uint64_t
AdjacencyList::getFaceId(const ndn::FaceUri& faceUri)
{
  auto it = std::find_if(m_adjList.begin(), m_adjList.end(),
                         std::bind(&Adjacent::compareFaceUri, _1, faceUri));
  return it != m_adjList.end() ? it->getFaceId() : 0;
}

void
AdjacencyList::writeLog()
{
  NLSR_LOG_DEBUG("-------Adjacency List--------");
  for (const auto& adjacent : m_adjList) {
    NLSR_LOG_DEBUG(adjacent);
  }
}

} // namespace nlsr
