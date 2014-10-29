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
#include <algorithm>
#include <ndn-cxx/common.hpp>

#include "adjacency-list.hpp"
#include "adjacent.hpp"
#include "common.hpp"
#include "nlsr.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("AdjacencyList");

using namespace std;

AdjacencyList::AdjacencyList()
{
}

AdjacencyList::~AdjacencyList()
{
}

int32_t
AdjacencyList::insert(Adjacent& adjacent)
{
  std::list<Adjacent>::iterator it = find(adjacent.getName());
  if (it != m_adjList.end()) {
    return -1;
  }
  m_adjList.push_back(adjacent);
  return 0;
}

void
AdjacencyList::addAdjacents(AdjacencyList& adl)
{
  for (std::list<Adjacent>::iterator it = adl.getAdjList().begin();
       it != adl.getAdjList().end(); ++it) {
    insert((*it));
  }
}

bool
AdjacencyList::updateAdjacentStatus(const ndn::Name& adjName, Adjacent::Status s)
{
  std::list<Adjacent>::iterator it = find(adjName);

  if (it == m_adjList.end()) {
    return false;
  }
  else {
    it->setStatus(s);
    return true;
  }
}

Adjacent
AdjacencyList::getAdjacent(const ndn::Name& adjName)
{
  Adjacent adj(adjName);
  std::list<Adjacent>::iterator it = find(adjName);
  if (it != m_adjList.end()) {
    return (*it);
  }
  return adj;
}

static bool
compareAdjacent(const Adjacent& adjacent1, const Adjacent& adjacent2)
{
  return adjacent1.getName() < adjacent2.getName();
}

bool
AdjacencyList::operator==(AdjacencyList& adl)
{
  if (getSize() != adl.getSize()) {
    return false;
  }
  m_adjList.sort(compareAdjacent);
  adl.getAdjList().sort(compareAdjacent);
  uint32_t equalAdjCount = 0;
  std::list<Adjacent>& adjList2 = adl.getAdjList();
  std::list<Adjacent>::iterator it1;
  std::list<Adjacent>::iterator it2;
  for (it1 = m_adjList.begin(), it2 = adjList2.begin();
       it1 != m_adjList.end(); it1++, it2++) {
    if (!((*it1) == (*it2))) {
      break;
    }
    equalAdjCount++;
  }
  return equalAdjCount == getSize();
}

int32_t
AdjacencyList::updateAdjacentLinkCost(const ndn::Name& adjName, double lc)
{
  std::list<Adjacent>::iterator it = find(adjName);
  if (it == m_adjList.end()) {
    return -1;
  }
  (*it).setLinkCost(lc);
  return 0;
}

bool
AdjacencyList::isNeighbor(const ndn::Name& adjName)
{
  std::list<Adjacent>::iterator it = find(adjName);
  if (it == m_adjList.end())
  {
    return false;
  }
  return true;
}

void
AdjacencyList::incrementTimedOutInterestCount(const ndn::Name& neighbor)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it == m_adjList.end()) {
    return ;
  }
  (*it).setInterestTimedOutNo((*it).getInterestTimedOutNo() + 1);
}

void
AdjacencyList::setTimedOutInterestCount(const ndn::Name& neighbor,
                                        uint32_t count)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it != m_adjList.end()) {
    (*it).setInterestTimedOutNo(count);
  }
}

int32_t
AdjacencyList::getTimedOutInterestCount(const ndn::Name& neighbor)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it == m_adjList.end()) {
    return -1;
  }
  return (*it).getInterestTimedOutNo();
}

Adjacent::Status
AdjacencyList::getStatusOfNeighbor(const ndn::Name& neighbor)
{
  std::list<Adjacent>::iterator it = find(neighbor);

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
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it != m_adjList.end()) {
    it->setStatus(status);
  }
}

std::list<Adjacent>&
AdjacencyList::getAdjList()
{
  return m_adjList;
}

bool
AdjacencyList::isAdjLsaBuildable(Nlsr& pnlsr)
{
  uint32_t nbrCount = 0;
  for (std::list<Adjacent>::iterator it = m_adjList.begin(); it != m_adjList.end() ; it++) {

    if (it->getStatus() == Adjacent::STATUS_ACTIVE) {
      nbrCount++;
    }
    else {
      if ((*it).getInterestTimedOutNo() >=
          pnlsr.getConfParameter().getInterestRetryNumber()) {
        nbrCount++;
      }
    }
  }
  if (nbrCount == m_adjList.size()) {
    return true;
  }
  return false;
}

int32_t
AdjacencyList::getNumOfActiveNeighbor()
{
  int32_t actNbrCount = 0;
  for (std::list<Adjacent>::iterator it = m_adjList.begin(); it != m_adjList.end(); it++) {

    if (it->getStatus() == Adjacent::STATUS_ACTIVE) {
      actNbrCount++;
    }
  }
  return actNbrCount;
}

std::list<Adjacent>::iterator
AdjacencyList::find(const ndn::Name& adjName)
{
  std::list<Adjacent>::iterator it = std::find_if(m_adjList.begin(),
                                                  m_adjList.end(),
                                                  ndn::bind(&Adjacent::compare,
                                                            _1, ndn::cref(adjName)));
  return it;
}

Adjacent *
AdjacencyList::findAdjacent(const ndn::Name& adjName)
{
  std::list<Adjacent>::iterator it = std::find_if(m_adjList.begin(),
                                                  m_adjList.end(),
                                                  ndn::bind(&Adjacent::compare,
                                                            _1, ndn::cref(adjName)));
  if (it != m_adjList.end()) {
    return &(*it);
  }

  return 0;
}

Adjacent *
AdjacencyList::findAdjacent(uint64_t faceId)
{
  std::list<Adjacent>::iterator it = std::find_if(m_adjList.begin(),
                                                  m_adjList.end(),
                                                  ndn::bind(&Adjacent::compareFaceId,
                                                            _1, faceId));
  if (it != m_adjList.end()) {
    return &(*it);
  }

  return 0;
}

uint64_t
AdjacencyList::getFaceId(const std::string& faceUri)
{
  std::list<Adjacent>::iterator it = std::find_if(m_adjList.begin(),
                                                  m_adjList.end(),
                                                  ndn::bind(&Adjacent::compareFaceUri,
                                                            _1, faceUri));
  if (it != m_adjList.end()) {
    return it->getFaceId();
  }

  return 0;
}

void
AdjacencyList::writeLog()
{
  _LOG_DEBUG("-------Adjacency List--------");
  for (std::list<Adjacent>::iterator it = m_adjList.begin();
       it != m_adjList.end(); it++) {
    (*it).writeLog();
  }
}

} //namespace nlsr
