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
  /*! \brief Allocate the space needed for the adj. matrix. */
  void
  allocateAdjMatrix();

  /*! \brief Zero every cell of the matrix to ensure that the memory is safe. */
  void
  initMatrix();

  /*! \brief Constructs an adj. matrix to calculate with.
    \param pnlsr The NLSR object that contains the LSAs that we need to iterate
    over.
    \param pMap The map to populate with the adj. data.
  */
  void
  makeAdjMatrix(Nlsr& pnlsr, Map pMap);

  void
  writeAdjMatrixLog();

  /*! \brief Returns how many links a router in the matrix has.
    \param sRouter The router to count the links of.
  */
  int
  getNumOfLinkfromAdjMatrix(int sRouter);

  void
  freeAdjMatrix();
  /*! \brief Adjust a link cost in the adj. matrix
    \param source The source router whose adjacency to adjust.
    \param link The adjacency of the source to adjust.
    \param linkCost The cost to change to.
  */
  void
  adjustAdMatrix(int source, int link, double linkCost);

  /*! \brief Populates temp. variables with the link costs for some router.
    \param source The router whose values are to be adjusted.
    \param links An integer pointer array for the link mappingNos.
    \param linkCosts A double pointer array that stores the link costs.

    Obtains a sparse list of adjacencies and link costs for some
    router. Since this is sparse, that means while generating these
    arrays, if there is no adjacency at i in the matrix, these
    temporary variables will not contain a 0 at i, but rather will
    contain the values for the next valid adjacency.
  */
  void
  getLinksFromAdjMatrix(int* links, double* linkCosts, int source);

  /*! Allocates an array large enough to hold multipath calculation temps. */
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
  /*! \brief Performs a Dijkstra's calculation over the adjacency matrix.
    \param sourceRouter The origin router to compute paths from.
  */
  void
  doDijkstraPathCalculation(int sourceRouter);

  /*! \brief Sort the elements of a list.
    \param Q The array that contains the elements to sort.
    \param dist The array that contains the distances.
    \param start The first element in the list to sort.
    \param element The last element in the list to sort through.

    Sorts the list based on distance. The distances are indexed by
    their mappingNo in dist. Currently uses an insertion sort.
  */
  void
  sortQueueByDistance(int* Q, double* dist, int start, int element);

  /*! \brief Returns whether an element has been visited yet.
    \param Q The list of elements to look through.
    \param u The element to check.
    \param start The start of list to look through.
    \param element The end of the list to look through.
  */
  int
  isNotExplored(int* Q, int u, int start, int element);

  void
  addAllLsNextHopsToRoutingTable(Nlsr& pnlsr, RoutingTable& rt,
                                 Map& pMap, uint32_t sourceRouter);

  /*! \brief Determines a destination's next hop.
    \param dest The router whose next hop we want to determine.
    \param source The router to determine a next path to.
  */
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
