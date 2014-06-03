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
#ifndef NLSR_ROUTING_TABLE_CALCULATOR_HPP
#define NLSR_ROUTING_TABLE_CALCULATOR_HPP

#include <list>
#include <iostream>
#include <boost/cstdint.hpp>

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
  RoutingTableCalculator(int rn)
  {
    numOfRouter = rn;
  }
protected:
  void
  allocateAdjMatrix();

  void
  initMatrix();

  void
  makeAdjMatrix(Nlsr& pnlsr, Map pMap);
/*
  void
  printAdjMatrix();
*/
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
  int numOfRouter;

  int vNoLink;
  int* links;
  double* linkCosts;
};

class LinkStateRoutingTableCalculator: public RoutingTableCalculator
{
public:
  LinkStateRoutingTableCalculator(int rn)
    : EMPTY_PARENT(-12345)
    , INF_DISTANCE(2147483647)
    , NO_MAPPING_NUM(-1)
    , NO_NEXT_HOP(-12345)
  {
    numOfRouter = rn;
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
                                 Map& pMap, int sourceRouter);

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

class HypRoutingTableCalculator: public RoutingTableCalculator
{
public:
  HypRoutingTableCalculator(int rn)
    :  MATH_PI(3.141592654)
  {
    numOfRouter = rn;
    m_isDryRun = 0;
  }

  HypRoutingTableCalculator(int rn, int idr)
    :  MATH_PI(3.141592654)
  {
    numOfRouter = rn;
    m_isDryRun = idr;
  }

  void
  calculatePath(Map& pMap, RoutingTable& rt, Nlsr& pnlsr);

private:
  void
  allocateLinkFaces();

  void
  allocateDistanceToNeighbor();

  void
  allocateDistFromNbrToDest();

  void
  freeLinkFaces();

  void
  freeDistanceToNeighbor();

  void
  freeDistFromNbrToDest();

  double
  getHyperbolicDistance(Nlsr& pnlsr, Map& pMap, int src, int dest);

  void
  addHypNextHopsToRoutingTable(Nlsr& pnlsr, Map& pMap,
                               RoutingTable& rt, int noFaces, int dest);

private:
  bool m_isDryRun;

  std::vector<std::string> m_linkFaceUris;
  double* m_distanceToNeighbor;
  double* m_distFromNbrToDest;

  const double MATH_PI;

};

}//namespace nlsr

#endif //NLSR_ROUTING_TABLE_CALCULATOR_HPP
