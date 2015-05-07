/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
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

#include <iostream>
#include <cmath>
#include "lsdb.hpp"
#include "routing-table-calculator.hpp"
#include "map.hpp"
#include "lsa.hpp"
#include "nexthop.hpp"
#include "nlsr.hpp"
#include "logger.hpp"

#include <boost/math/constants/constants.hpp>

namespace nlsr {

INIT_LOGGER("RoutingTableCalculator");
using namespace std;

void
RoutingTableCalculator::allocateAdjMatrix()
{
  adjMatrix = new double*[m_nRouters];

  for (size_t i = 0; i < m_nRouters; ++i) {
    adjMatrix[i] = new double[m_nRouters];
  }
}

void
RoutingTableCalculator::initMatrix()
{
  for (size_t i = 0; i < m_nRouters; i++) {
    for (size_t j = 0; j < m_nRouters; j++) {
      adjMatrix[i][j] = 0;
    }
  }
}

void
RoutingTableCalculator::makeAdjMatrix(Nlsr& pnlsr, Map pMap)
{
  std::list<AdjLsa> adjLsdb = pnlsr.getLsdb().getAdjLsdb();
  for (std::list<AdjLsa>::iterator it = adjLsdb.begin(); it != adjLsdb.end() ; it++) {

    int32_t row = pMap.getMappingNoByRouterName((*it).getOrigRouter());

    std::list<Adjacent> adl = (*it).getAdl().getAdjList();
    for (std::list<Adjacent>::iterator itAdl = adl.begin(); itAdl != adl.end() ; itAdl++) {

      int32_t col = pMap.getMappingNoByRouterName((*itAdl).getName());
      double cost = (*itAdl).getLinkCost();

      if ((row >= 0 && row < static_cast<int32_t>(m_nRouters)) &&
          (col >= 0 && col < static_cast<int32_t>(m_nRouters)))
      {
        adjMatrix[row][col] = cost;
      }
    }
  }

  // Links that do not have the same cost for both directions should have their
  // costs corrected:
  //
  //   If the cost of one side of the link is 0, both sides of the link should have their cost
  //   corrected to 0.
  //
  //   Otherwise, both sides of the link should use the larger of the two costs.
  //
  for (size_t row = 0; row < m_nRouters; ++row) {
    for (size_t col = 0; col < m_nRouters; ++col) {
      double toCost = adjMatrix[row][col];
      double fromCost = adjMatrix[col][row];

      if (fromCost != toCost) {
        double correctedCost = 0.0;

        if (toCost != 0 && fromCost != 0) {
          // If both sides of the link are up, use the larger cost
          correctedCost = std::max(toCost, fromCost);
        }

        _LOG_WARN("Cost between [" << row << "][" << col << "] and [" << col << "][" << row <<
                  "] are not the same (" << toCost << " != " << fromCost << "). " <<
                  "Correcting to cost: " << correctedCost);

        adjMatrix[row][col] = correctedCost;
        adjMatrix[col][row] = correctedCost;
      }
    }
  }
}

void
RoutingTableCalculator::writeAdjMatrixLog()
{
  for (size_t i = 0; i < m_nRouters; i++) {
    string line="";
    for (size_t j = 0; j < m_nRouters; j++) {
      line += boost::lexical_cast<std::string>(adjMatrix[i][j]);
      line += " ";
    }
    _LOG_DEBUG(line);
  }
}

void
RoutingTableCalculator::adjustAdMatrix(int source, int link, double linkCost)
{
  for (int i = 0; i < static_cast<int>(m_nRouters); i++) {
    if (i == link) {
      adjMatrix[source][i] = linkCost;
    }
    else {
      adjMatrix[source][i] = 0;
    }
  }
}

int
RoutingTableCalculator::getNumOfLinkfromAdjMatrix(int sRouter)
{
  int noLink = 0;

  for (size_t i = 0; i < m_nRouters; i++) {
    if (adjMatrix[sRouter][i] > 0) {
      noLink++;
    }
  }
  return noLink;
}

void
RoutingTableCalculator::getLinksFromAdjMatrix(int* links,
                                              double* linkCosts, int source)
{
  int j = 0;

  for (size_t i = 0; i < m_nRouters; i++) {
    if (adjMatrix[source][i] > 0) {
      links[j] = i;
      linkCosts[j] = adjMatrix[source][i];
      j++;
    }
  }
}

void
RoutingTableCalculator::freeAdjMatrix()
{
  for (size_t i = 0; i < m_nRouters; ++i) {
    delete [] adjMatrix[i];
  }
  delete [] adjMatrix;
}


void
RoutingTableCalculator::allocateLinks()
{
  links = new int[vNoLink];
}

void
RoutingTableCalculator::allocateLinkCosts()
{
  linkCosts = new double[vNoLink];
}


void
RoutingTableCalculator::freeLinks()
{
  delete [] links;
}
void
RoutingTableCalculator::freeLinksCosts()
{
  delete [] linkCosts;
}

void
LinkStateRoutingTableCalculator::calculatePath(Map& pMap,
                                               RoutingTable& rt, Nlsr& pnlsr)
{
  _LOG_DEBUG("LinkStateRoutingTableCalculator::calculatePath Called");
  allocateAdjMatrix();
  initMatrix();
  makeAdjMatrix(pnlsr, pMap);
  writeAdjMatrixLog();
  int sourceRouter = pMap.getMappingNoByRouterName(pnlsr.getConfParameter().getRouterPrefix());
  allocateParent();
  allocateDistance();
  if (pnlsr.getConfParameter().getMaxFacesPerPrefix() == 1) {
    // Single Path
    doDijkstraPathCalculation(sourceRouter);
    // update routing table
    addAllLsNextHopsToRoutingTable(pnlsr, rt, pMap, sourceRouter);
  }
  else {
    // Multi Path
    setNoLink(getNumOfLinkfromAdjMatrix(sourceRouter));
    allocateLinks();
    allocateLinkCosts();
    getLinksFromAdjMatrix(links, linkCosts, sourceRouter);
    for (int i = 0 ; i < vNoLink; i++) {
      adjustAdMatrix(sourceRouter, links[i], linkCosts[i]);
      writeAdjMatrixLog();
      doDijkstraPathCalculation(sourceRouter);
      //update routing table
      addAllLsNextHopsToRoutingTable(pnlsr, rt, pMap, sourceRouter);
    }
    freeLinks();
    freeLinksCosts();
  }
  freeParent();
  freeDistance();
  freeAdjMatrix();
}

void
LinkStateRoutingTableCalculator::doDijkstraPathCalculation(int sourceRouter)
{
  int i;
  int v, u;
  int* Q = new int[m_nRouters];
  int head = 0;
  /* Initiate the Parent */
  for (i = 0 ; i < static_cast<int>(m_nRouters); i++) {
    m_parent[i] = EMPTY_PARENT;
    m_distance[i] = INF_DISTANCE;
    Q[i] = i;
  }
  if (sourceRouter != NO_MAPPING_NUM) {
    m_distance[sourceRouter] = 0;
    sortQueueByDistance(Q, m_distance, head, m_nRouters);
    while (head < static_cast<int>(m_nRouters)) {
      u = Q[head];
      if (m_distance[u] == INF_DISTANCE) {
        break;
      }
      for (v = 0 ; v < static_cast<int>(m_nRouters); v++) {
        if (adjMatrix[u][v] > 0) {
          if (isNotExplored(Q, v, head + 1, m_nRouters)) {
            if (m_distance[u] + adjMatrix[u][v] <  m_distance[v]) {
              m_distance[v] = m_distance[u] + adjMatrix[u][v] ;
              m_parent[v] = u;
            }
          }
        }
      }
      head++;
      sortQueueByDistance(Q, m_distance, head, m_nRouters);
    }
  }
  delete [] Q;
}

void
LinkStateRoutingTableCalculator::addAllLsNextHopsToRoutingTable(Nlsr& pnlsr, RoutingTable& rt,
                                                                Map& pMap, uint32_t sourceRouter)
{
  _LOG_DEBUG("LinkStateRoutingTableCalculator::addAllNextHopsToRoutingTable Called");

  int nextHopRouter = 0;

  for (size_t i = 0; i < m_nRouters ; i++) {
    if (i != sourceRouter) {

      nextHopRouter = getLsNextHop(i, sourceRouter);

      if (nextHopRouter != NO_NEXT_HOP) {

        double routeCost = m_distance[i];
        ndn::Name nextHopRouterName = pMap.getRouterNameByMappingNo(nextHopRouter);
        std::string nextHopFace =
          pnlsr.getAdjacencyList().getAdjacent(nextHopRouterName).getConnectingFaceUri();
        // Add next hop to routing table
        NextHop nh(nextHopFace, routeCost);
        rt.addNextHop(pMap.getRouterNameByMappingNo(i), nh);
      }
    }
  }
}

int
LinkStateRoutingTableCalculator::getLsNextHop(int dest, int source)
{
  int nextHop = NO_NEXT_HOP;
  while (m_parent[dest] != EMPTY_PARENT) {
    nextHop = dest;
    dest = m_parent[dest];
  }
  if (dest != source) {
    nextHop = NO_NEXT_HOP;
  }
  return nextHop;
}

void
LinkStateRoutingTableCalculator::sortQueueByDistance(int* Q,
                                                     double* dist,
                                                     int start, int element)
{
  for (int i = start ; i < element ; i++) {
    for (int j = i + 1; j < element; j++) {
      if (dist[Q[j]] < dist[Q[i]]) {
        int tempU = Q[j];
        Q[j] = Q[i];
        Q[i] = tempU;
      }
    }
  }
}

int
LinkStateRoutingTableCalculator::isNotExplored(int* Q,
                                               int u, int start, int element)
{
  int ret = 0;
  for (int i = start; i < element; i++) {
    if (Q[i] == u) {
      ret = 1;
      break;
    }
  }
  return ret;
}

void
LinkStateRoutingTableCalculator::allocateParent()
{
  m_parent = new int[m_nRouters];
}

void
LinkStateRoutingTableCalculator::allocateDistance()
{
  m_distance = new double[m_nRouters];
}

void
LinkStateRoutingTableCalculator::freeParent()
{
  delete [] m_parent;
}

void LinkStateRoutingTableCalculator::freeDistance()
{
  delete [] m_distance;
}

const double HyperbolicRoutingCalculator::MATH_PI = boost::math::constants::pi<double>();

const double HyperbolicRoutingCalculator::UNKNOWN_DISTANCE = -1.0;
const double HyperbolicRoutingCalculator::UNKNOWN_RADIUS   = -1.0;

const int32_t HyperbolicRoutingCalculator::ROUTER_NOT_FOUND = -1.0;

void
HyperbolicRoutingCalculator::calculatePaths(Map& map, RoutingTable& rt,
                                            Lsdb& lsdb, AdjacencyList& adjacencies)
{
  _LOG_TRACE("Calculating hyperbolic paths");

  int thisRouter = map.getMappingNoByRouterName(m_thisRouterName);

  // Iterate over directly connected neighbors
  std::list<Adjacent> neighbors = adjacencies.getAdjList();
  for (std::list<Adjacent>::iterator adj = neighbors.begin(); adj != neighbors.end(); ++adj) {

    // Don't calculate nexthops using an inactive router
    if (adj->getStatus() == Adjacent::STATUS_INACTIVE) {
      _LOG_TRACE(adj->getName() << " is inactive; not using it as a nexthop");
      continue;
    }

    ndn::Name srcRouterName = adj->getName();

    // Don't calculate nexthops for this router to other routers
    if (srcRouterName == m_thisRouterName) {
      continue;
    }

    std::string srcFaceUri = adj->getConnectingFaceUri();

    // Install nexthops for this router to the neighbor; direct neighbors have a 0 cost link
    addNextHop(srcRouterName, srcFaceUri, 0, rt);

    int src = map.getMappingNoByRouterName(srcRouterName);

    if (src == ROUTER_NOT_FOUND) {
      _LOG_WARN(adj->getName() << " does not exist in the router map!");
      continue;
    }

    // Get hyperbolic distance from direct neighbor to every other router
    for (int dest = 0; dest < static_cast<int>(m_nRouters); ++dest) {
      // Don't calculate nexthops to this router or from a router to itself
      if (dest != thisRouter && dest != src) {

        ndn::Name destRouterName = map.getRouterNameByMappingNo(dest);

        double distance = getHyperbolicDistance(map, lsdb, srcRouterName, destRouterName);

        // Could not compute distance
        if (distance == UNKNOWN_DISTANCE) {
          _LOG_WARN("Could not calculate hyperbolic distance from " << srcRouterName << " to " <<
                    destRouterName);
          continue;
        }

        addNextHop(destRouterName, srcFaceUri, distance, rt);
      }
    }
  }
}

double
HyperbolicRoutingCalculator::getHyperbolicDistance(Map& map, Lsdb& lsdb,
                                                   ndn::Name src, ndn::Name dest)
{
  _LOG_TRACE("Calculating hyperbolic distance from " << src << " to " << dest);

  double distance = UNKNOWN_DISTANCE;

  ndn::Name srcLsaKey = src;
  srcLsaKey.append("coordinate");

  CoordinateLsa* srcLsa = lsdb.findCoordinateLsa(srcLsaKey);

  ndn::Name destLsaKey = dest;
  destLsaKey.append("coordinate");

  CoordinateLsa* destLsa = lsdb.findCoordinateLsa(destLsaKey);

  // Coordinate LSAs do not exist for these routers
  if (srcLsa == NULL || destLsa == NULL) {
    return UNKNOWN_DISTANCE;
  }

  double srcTheta = srcLsa->getCorTheta();
  double destTheta = destLsa->getCorTheta();

  double diffTheta = fabs(srcTheta - destTheta);

  if (diffTheta > MATH_PI) {
    diffTheta = 2 * MATH_PI - diffTheta;
  }

  double srcRadius = srcLsa->getCorRadius();
  double destRadius = destLsa->getCorRadius();

  if (srcRadius == UNKNOWN_RADIUS && destRadius == UNKNOWN_RADIUS) {
    return UNKNOWN_DISTANCE;
  }

  if (diffTheta == 0) {
    distance = fabs(srcRadius - destRadius);
  }
  else {
    distance = acosh((cosh(srcRadius) * cosh(destRadius)) -
                     (sinh(srcRadius) * sinh(destRadius) * cos(diffTheta)));
  }

  _LOG_TRACE("Distance from " << src << " to " << dest << " is " << distance);

  return distance;
}

void HyperbolicRoutingCalculator::addNextHop(ndn::Name dest, std::string faceUri,
                                             double cost, RoutingTable& rt)
{
  NextHop hop(faceUri, cost);
  hop.setHyperbolic(true);

  _LOG_TRACE("Calculated " << hop << " for destination: " << dest);

  if (m_isDryRun) {
    rt.addNextHopToDryTable(dest, hop);
  }
  else {
    rt.addNextHop(dest, hop);
  }
}

}//namespace nlsr
