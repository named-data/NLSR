/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  The University of Memphis,
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
 */

#include "routing-table-calculator.hpp"
#include "lsdb.hpp"
#include "map.hpp"
#include "nexthop.hpp"
#include "nlsr.hpp"
#include "logger.hpp"
#include "adjacent.hpp"

#include <boost/math/constants/constants.hpp>
#include <ndn-cxx/util/logger.hpp>
#include <cmath>

namespace nlsr {

INIT_LOGGER(route.RoutingTableCalculator);

const int LinkStateRoutingTableCalculator::EMPTY_PARENT = -12345;
const double LinkStateRoutingTableCalculator::INF_DISTANCE = 2147483647;
const int LinkStateRoutingTableCalculator::NO_MAPPING_NUM = -1;
const int LinkStateRoutingTableCalculator::NO_NEXT_HOP = -12345;

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
      adjMatrix[i][j] = Adjacent::NON_ADJACENT_COST;
    }
  }
}

void
RoutingTableCalculator::makeAdjMatrix(const Lsdb& lsdb, Map& pMap)
{
  // For each LSA represented in the map
  auto lsaRange = lsdb.getLsdbIterator<AdjLsa>();
  for (auto lsaIt = lsaRange.first; lsaIt != lsaRange.second; ++lsaIt) {
    auto adjLsa = std::static_pointer_cast<AdjLsa>(*lsaIt);
    ndn::optional<int32_t> row = pMap.getMappingNoByRouterName(adjLsa->getOriginRouter());

    std::list<Adjacent> adl = adjLsa->getAdl().getAdjList();
    // For each adjacency represented in the LSA
    for (const auto& adjacent : adl) {
      ndn::optional<int32_t> col = pMap.getMappingNoByRouterName(adjacent.getName());
      double cost = adjacent.getLinkCost();

      if (row && col && *row < static_cast<int32_t>(m_nRouters)
          && *col < static_cast<int32_t>(m_nRouters))
      {
        adjMatrix[*row][*col] = cost;
      }
    }
  }

  // Links that do not have the same cost for both directions should
  // have their costs corrected:
  //
  //   If the cost of one side of the link is NON_ADJACENT_COST (i.e. broken) or negative,
  //   both direction of the link should have their cost corrected to NON_ADJACENT_COST.
  //
  //   Otherwise, both sides of the link should use the larger of the two costs.
  //
  // Additionally, this means that we can halve the amount of space
  // that the matrix uses by only maintaining a triangle.
  // - But that is not yet implemented.
  for (size_t row = 0; row < m_nRouters; ++row) {
    for (size_t col = 0; col < m_nRouters; ++col) {
      double toCost = adjMatrix[row][col];
      double fromCost = adjMatrix[col][row];

      if (fromCost != toCost) {
        double correctedCost = Adjacent::NON_ADJACENT_COST;

        if (toCost >= 0 && fromCost >= 0) {
          // If both sides of the link are up, use the larger cost else break the link
          correctedCost = std::max(toCost, fromCost);
        }

        NLSR_LOG_WARN("Cost between [" << row << "][" << col << "] and [" << col << "][" << row <<
                  "] are not the same (" << toCost << " != " << fromCost << "). " <<
                  "Correcting to cost: " << correctedCost);

        adjMatrix[row][col] = correctedCost;
        adjMatrix[col][row] = correctedCost;
      }
    }
  }
}

void
RoutingTableCalculator::writeAdjMatrixLog(const Map& map) const
{
  if (!ndn_cxx_getLogger().isLevelEnabled(ndn::util::LogLevel::DEBUG)) {
    return;
  }

  NLSR_LOG_DEBUG("-----------Legend (routerName -> index)------");
  std::string routerIndex;
  std::string indexToNameMapping;
  std::string lengthOfDash = "--";

  for (size_t i = 0; i < m_nRouters; i++) {
      routerIndex += boost::lexical_cast<std::string>(i);
      routerIndex += " ";
      lengthOfDash += "--";
      NLSR_LOG_DEBUG("Router:" + map.getRouterNameByMappingNo(i)->toUri() +
                     " Index:" + boost::lexical_cast<std::string>(i));
  }
  NLSR_LOG_DEBUG(" |" + routerIndex);
  NLSR_LOG_DEBUG(lengthOfDash);

  for (size_t i = 0; i < m_nRouters; i++) {
    std::string line;
    for (size_t j = 0; j < m_nRouters; j++) {
      if (adjMatrix[i][j] == LinkStateRoutingTableCalculator::NO_NEXT_HOP) {
        line += "0 ";
      }
      else {
        line += boost::lexical_cast<std::string>(adjMatrix[i][j]);
        line += " ";
      }
    }
    line = boost::lexical_cast<std::string>(i) + "|" + line;
    NLSR_LOG_DEBUG(line);
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
      // if "i" is not a link to the source, set it's cost to a non adjacent value.
      adjMatrix[source][i] = Adjacent::NON_ADJACENT_COST;
    }
  }
}

int
RoutingTableCalculator::getNumOfLinkfromAdjMatrix(int sRouter)
{
  int noLink = 0;

  for (size_t i = 0; i < m_nRouters; i++) {
    if (adjMatrix[sRouter][i] >= 0 && i != static_cast<size_t>(sRouter)) { // make sure "i" is not self
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
    if (adjMatrix[source][i] >= 0 && i != static_cast<size_t>(source)) {// make sure "i" is not self
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
LinkStateRoutingTableCalculator::calculatePath(Map& pMap, RoutingTable& rt,
                                               ConfParameter& confParam,
                                               const Lsdb& lsdb)
{
  NLSR_LOG_DEBUG("LinkStateRoutingTableCalculator::calculatePath Called");
  allocateAdjMatrix();
  initMatrix();
  makeAdjMatrix(lsdb, pMap);
  writeAdjMatrixLog(pMap);
  ndn::optional<int32_t> sourceRouter =
    pMap.getMappingNoByRouterName(confParam.getRouterPrefix());
  allocateParent(); // These two matrices are used in Dijkstra's algorithm.
  allocateDistance(); //
  // We only bother to do the calculation if we have a router by that name.
  if (sourceRouter && confParam.getMaxFacesPerPrefix() == 1) {
    // In the single path case we can simply run Dijkstra's algorithm.
    doDijkstraPathCalculation(*sourceRouter);
    // Inform the routing table of the new next hops.
    addAllLsNextHopsToRoutingTable(confParam.getAdjacencyList(), rt, pMap, *sourceRouter);
  }
  else {
    // Multi Path
    setNoLink(getNumOfLinkfromAdjMatrix(*sourceRouter));
    allocateLinks();
    allocateLinkCosts();
    // Gets a sparse listing of adjacencies for path calculation
    getLinksFromAdjMatrix(links, linkCosts, *sourceRouter);
    for (int i = 0 ; i < vNoLink; i++) {
      // Simulate that only the current neighbor is accessible
      adjustAdMatrix(*sourceRouter, links[i], linkCosts[i]);
      writeAdjMatrixLog(pMap);
      // Do Dijkstra's algorithm using the current neighbor as your start.
      doDijkstraPathCalculation(*sourceRouter);
      // Update the routing table with the calculations.
      addAllLsNextHopsToRoutingTable(confParam.getAdjacencyList(), rt, pMap, *sourceRouter);
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
  int* Q = new int[m_nRouters]; // Each cell represents the router with that mapping no.
  int head = 0;
  // Initiate the parent
  for (i = 0 ; i < static_cast<int>(m_nRouters); i++) {
    m_parent[i] = EMPTY_PARENT;
    // Array where the ith element is the distance to the router with mapping no i.
    m_distance[i] = INF_DISTANCE;
    Q[i] = i;
  }
  if (sourceRouter != NO_MAPPING_NUM) {
    // Distance to source from source is always 0.
    m_distance[sourceRouter] = 0;
    sortQueueByDistance(Q, m_distance, head, m_nRouters);
    // While we haven't visited every node.
    while (head < static_cast<int>(m_nRouters)) {
      u = Q[head]; // Set u to be the current node pointed to by head.
      if (m_distance[u] == INF_DISTANCE) {
        break; // This can only happen when there are no accessible nodes.
      }
      // Iterate over the adjacent nodes to u.
      for (v = 0 ; v < static_cast<int>(m_nRouters); v++) {
        // If the current node is accessible.
        if (adjMatrix[u][v] >= 0) {
          // And we haven't visited it yet.
          if (isNotExplored(Q, v, head + 1, m_nRouters)) {
            // And if the distance to this node + from this node to v
            // is less than the distance from our source node to v
            // that we got when we built the adj LSAs
            if (m_distance[u] + adjMatrix[u][v] <  m_distance[v]) {
              // Set the new distance
              m_distance[v] = m_distance[u] + adjMatrix[u][v] ;
              // Set how we get there.
              m_parent[v] = u;
            }
          }
        }
      }
      // Increment the head position, resort the list by distance from where we are.
      head++;
      sortQueueByDistance(Q, m_distance, head, m_nRouters);
    }
  }
  delete [] Q;
}

void
LinkStateRoutingTableCalculator::addAllLsNextHopsToRoutingTable(AdjacencyList& adjacencies,
                                                                RoutingTable& rt, Map& pMap,
                                                                uint32_t sourceRouter)
{
  NLSR_LOG_DEBUG("LinkStateRoutingTableCalculator::addAllNextHopsToRoutingTable Called");

  int nextHopRouter = 0;

  // For each router we have
  for (size_t i = 0; i < m_nRouters ; i++) {
    if (i != sourceRouter) {

      // Obtain the next hop that was determined by the algorithm
      nextHopRouter = getLsNextHop(i, sourceRouter);
      // If this router is accessible at all
      if (nextHopRouter != NO_NEXT_HOP) {

        // Fetch its distance
        double routeCost = m_distance[i];
        // Fetch its actual name
        ndn::optional<ndn::Name> nextHopRouterName= pMap.getRouterNameByMappingNo(nextHopRouter);
        if (nextHopRouterName) {
          std::string nextHopFace =
            adjacencies.getAdjacent(*nextHopRouterName).getFaceUri().toString();
          // Add next hop to routing table
          NextHop nh(nextHopFace, routeCost);
          rt.addNextHop(*(pMap.getRouterNameByMappingNo(i)), nh);

        }
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

void
HyperbolicRoutingCalculator::calculatePath(Map& map, RoutingTable& rt,
                                           Lsdb& lsdb, AdjacencyList& adjacencies)
{
  NLSR_LOG_TRACE("Calculating hyperbolic paths");

  ndn::optional<int32_t> thisRouter = map.getMappingNoByRouterName(m_thisRouterName);

  // Iterate over directly connected neighbors
  std::list<Adjacent> neighbors = adjacencies.getAdjList();
  for (std::list<Adjacent>::iterator adj = neighbors.begin(); adj != neighbors.end(); ++adj) {

    // Don't calculate nexthops using an inactive router
    if (adj->getStatus() == Adjacent::STATUS_INACTIVE) {
      NLSR_LOG_TRACE(adj->getName() << " is inactive; not using it as a nexthop");
      continue;
    }

    ndn::Name srcRouterName = adj->getName();

    // Don't calculate nexthops for this router to other routers
    if (srcRouterName == m_thisRouterName) {
      continue;
    }

    std::string srcFaceUri = adj->getFaceUri().toString();

    // Install nexthops for this router to the neighbor; direct neighbors have a 0 cost link
    addNextHop(srcRouterName, srcFaceUri, 0, rt);

    ndn::optional<int32_t> src = map.getMappingNoByRouterName(srcRouterName);

    if (!src) {
      NLSR_LOG_WARN(adj->getName() << " does not exist in the router map!");
      continue;
    }

    // Get hyperbolic distance from direct neighbor to every other router
    for (int dest = 0; dest < static_cast<int>(m_nRouters); ++dest) {
      // Don't calculate nexthops to this router or from a router to itself
      if (thisRouter && dest != *thisRouter && dest != *src) {

        ndn::optional<ndn::Name> destRouterName = map.getRouterNameByMappingNo(dest);
        if (destRouterName) {
          double distance = getHyperbolicDistance(lsdb, srcRouterName, *destRouterName);

          // Could not compute distance
          if (distance == UNKNOWN_DISTANCE) {
            NLSR_LOG_WARN("Could not calculate hyperbolic distance from " << srcRouterName
                           << " to " << *destRouterName);
            continue;
          }
          addNextHop(*destRouterName, srcFaceUri, distance, rt);
        }
      }
    }
  }
}

double
HyperbolicRoutingCalculator::getHyperbolicDistance(Lsdb& lsdb, ndn::Name src, ndn::Name dest)
{
  NLSR_LOG_TRACE("Calculating hyperbolic distance from " << src << " to " << dest);

  double distance = UNKNOWN_DISTANCE;

  auto srcLsa  = lsdb.findLsa<CoordinateLsa>(src);
  auto destLsa = lsdb.findLsa<CoordinateLsa>(dest);

  // Coordinate LSAs do not exist for these routers
  if (srcLsa == nullptr || destLsa == nullptr) {
    return UNKNOWN_DISTANCE;
  }

  std::vector<double> srcTheta = srcLsa->getCorTheta();
  std::vector<double> destTheta = destLsa->getCorTheta();

  double srcRadius = srcLsa->getCorRadius();
  double destRadius = destLsa->getCorRadius();

  double diffTheta = calculateAngularDistance(srcTheta, destTheta);

  if (srcRadius == UNKNOWN_RADIUS || destRadius == UNKNOWN_RADIUS ||
      diffTheta == UNKNOWN_DISTANCE) {
    return UNKNOWN_DISTANCE;
  }

  // double r_i, double r_j, double delta_theta, double zeta = 1 (default)
  distance = calculateHyperbolicDistance(srcRadius, destRadius, diffTheta);

  NLSR_LOG_TRACE("Distance from " << src << " to " << dest << " is " << distance);

  return distance;
}

double
HyperbolicRoutingCalculator::calculateAngularDistance(std::vector<double> angleVectorI,
                                                      std::vector<double> angleVectorJ)
{
  // It is not possible for angle vector size to be zero as ensured by conf-file-processor

  // https://en.wikipedia.org/wiki/N-sphere#Spherical_coordinates

  // Check if two vector lengths are the same
  if (angleVectorI.size() != angleVectorJ.size()) {
    NLSR_LOG_ERROR("Angle vector sizes do not match");
    return UNKNOWN_DISTANCE;
  }

  // Check if all angles are within the [0, PI] and [0, 2PI] ranges
  if (angleVectorI.size() > 1) {
    for (unsigned int k = 0; k < angleVectorI.size() - 1; k++) {
      if ((angleVectorI[k] > M_PI && angleVectorI[k] < 0.0) ||
          (angleVectorJ[k] > M_PI && angleVectorJ[k] < 0.0)) {
        NLSR_LOG_ERROR("Angle outside [0, PI]");
        return UNKNOWN_DISTANCE;
      }
    }
  }
  if (angleVectorI[angleVectorI.size()-1] > 2.*M_PI ||
      angleVectorI[angleVectorI.size()-1] < 0.0) {
    NLSR_LOG_ERROR("Angle not within [0, 2PI]");
    return UNKNOWN_DISTANCE;
  }

  if (angleVectorI[angleVectorI.size()-1] > 2.*M_PI ||
      angleVectorI[angleVectorI.size()-1] < 0.0) {
    NLSR_LOG_ERROR("Angle not within [0, 2PI]");
    return UNKNOWN_DISTANCE;
  }

  // deltaTheta = arccos(vectorI . vectorJ) -> do the inner product
  double innerProduct = 0.0;

  // Calculate x0 of the vectors
  double x0i = std::cos(angleVectorI[0]);
  double x0j = std::cos(angleVectorJ[0]);

  // Calculate xn of the vectors
  double xni = std::sin(angleVectorI[angleVectorI.size() - 1]);
  double xnj = std::sin(angleVectorJ[angleVectorJ.size() - 1]);

  // Do the aggregation of the (n-1) coordinates (if there is more than one angle)
  // i.e contraction of all (n-1)-dimensional angular coordinates to one variable
  for (unsigned int k = 0; k < angleVectorI.size() - 1; k++) {
    xni *= std::sin(angleVectorI[k]);
    xnj *= std::sin(angleVectorJ[k]);
  }
  innerProduct += (x0i * x0j) + (xni * xnj);

  // If d > 1
  if (angleVectorI.size() > 1) {
    for (unsigned int m = 1; m < angleVectorI.size(); m++) {
      // calculate euclidean coordinates given the angles and assuming R_sphere = 1
      double xmi = std::cos(angleVectorI[m]);
      double xmj = std::cos(angleVectorJ[m]);
      for (unsigned int l = 0; l < m; l++) {
        xmi *= std::sin(angleVectorI[l]);
        xmj *= std::sin(angleVectorJ[l]);
      }
      innerProduct += xmi * xmj;
    }
  }

  // ArcCos of the inner product gives the angular distance
  // between two points on a d-dimensional sphere
  return std::acos(innerProduct);
}

double
HyperbolicRoutingCalculator::calculateHyperbolicDistance(double rI, double rJ,
                                                         double deltaTheta)
{
  if (deltaTheta == UNKNOWN_DISTANCE) {
    return UNKNOWN_DISTANCE;
  }

  // Usually, we set zeta = 1 in all experiments
  double zeta = 1;

  if (deltaTheta <= 0.0 || rI <= 0.0 || rJ <= 0.0) {
    NLSR_LOG_ERROR("Delta theta or rI or rJ is <= 0");
    NLSR_LOG_ERROR("Please make sure that no two nodes have the exact same HR coordinates");
    return UNKNOWN_DISTANCE;
  }

  double xij = (1. / zeta) * std::acosh(std::cosh(zeta*rI) * std::cosh(zeta*rJ) -
               std::sinh(zeta*rI)*std::sinh(zeta*rJ)*std::cos(deltaTheta));
  return xij;
}

void
HyperbolicRoutingCalculator::addNextHop(ndn::Name dest, std::string faceUri,
                                        double cost, RoutingTable& rt)
{
  NextHop hop(faceUri, cost);
  hop.setHyperbolic(true);

  NLSR_LOG_TRACE("Calculated " << hop << " for destination: " << dest);

  if (m_isDryRun) {
    rt.addNextHopToDryTable(dest, hop);
  }
  else {
    rt.addNextHop(dest, hop);
  }
}

} // namespace nlsr
