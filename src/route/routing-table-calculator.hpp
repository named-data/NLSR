/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#ifndef NLSR_ROUTING_TABLE_CALCULATOR_HPP
#define NLSR_ROUTING_TABLE_CALCULATOR_HPP

#include <list>
#include <iostream>
#include <boost/cstdint.hpp>

#include <ndn-cxx/name.hpp>

namespace nlsr {

class Map;
class RoutingTable;
class Nlsr;

class RoutingTableCalculator
{
public:
  RoutingTableCalculator()
  {
  }
  RoutingTableCalculator(size_t nRouters)
  {
    m_nRouters = nRouters;
  }
protected:
  void
  allocateAdjMatrix();

  void
  initMatrix();

  void
  makeAdjMatrix(Nlsr& pnlsr, Map pMap);

  void
  writeAdjMatrixLog();

  int
  getNumOfLinkfromAdjMatrix(int sRouter);

  void
  freeAdjMatrix();

  void
  adjustAdMatrix(int source, int link, double linkCost);

  void
  getLinksFromAdjMatrix(int* links, double* linkCosts, int source);

  void
  allocateLinks();

  void
  allocateLinkCosts();

  void
  freeLinks();

  void
  freeLinksCosts();

  void
  setNoLink(int nl)
  {
    vNoLink = nl;
  }

protected:
  double** adjMatrix;
  size_t m_nRouters;

  int vNoLink;
  int* links;
  double* linkCosts;
};

class LinkStateRoutingTableCalculator: public RoutingTableCalculator
{
public:
  LinkStateRoutingTableCalculator(size_t nRouters)
    : RoutingTableCalculator(nRouters)
    , EMPTY_PARENT(-12345)
    , INF_DISTANCE(2147483647)
    , NO_MAPPING_NUM(-1)
    , NO_NEXT_HOP(-12345)
  {
  }

  void
  calculatePath(Map& pMap, RoutingTable& rt, Nlsr& pnlsr);

private:
  void
  doDijkstraPathCalculation(int sourceRouter);

  void
  sortQueueByDistance(int* Q, double* dist, int start, int element);

  int
  isNotExplored(int* Q, int u, int start, int element);

  void
  addAllLsNextHopsToRoutingTable(Nlsr& pnlsr, RoutingTable& rt,
                                 Map& pMap, uint32_t sourceRouter);

  int
  getLsNextHop(int dest, int source);

  void
  allocateParent();

  void
  allocateDistance();

  void
  freeParent();

  void
  freeDistance();

private:
  int* m_parent;
  double* m_distance;

  const int EMPTY_PARENT;
  const double INF_DISTANCE;
  const int NO_MAPPING_NUM;
  const int NO_NEXT_HOP;

};

class AdjacencyList;
class Lsdb;

class HyperbolicRoutingCalculator
{
public:
  HyperbolicRoutingCalculator(size_t nRouters, bool isDryRun, ndn::Name thisRouterName)
    : m_nRouters(nRouters)
    , m_isDryRun(isDryRun)
    , m_thisRouterName(thisRouterName)
  {
  }

  void
  calculatePaths(Map& map, RoutingTable& rt, Lsdb& lsdb, AdjacencyList& adjacencies);

private:
  double
  getHyperbolicDistance(Map& map, Lsdb& lsdb, ndn::Name src, ndn::Name dest);

  void
  addNextHop(ndn::Name destinationRouter, std::string faceUri, double cost, RoutingTable& rt);

private:
  const size_t m_nRouters;
  const bool m_isDryRun;
  const ndn::Name m_thisRouterName;

  static const double MATH_PI;
  static const double UNKNOWN_DISTANCE;
  static const double UNKNOWN_RADIUS;
  static const int32_t ROUTER_NOT_FOUND;
};

} // namespace nlsr

#endif //NLSR_ROUTING_TABLE_CALCULATOR_HPP
