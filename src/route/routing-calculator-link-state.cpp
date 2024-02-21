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
 */

#include "routing-calculator.hpp"
#include "name-map.hpp"
#include "nexthop.hpp"

#include "adjacent.hpp"
#include "logger.hpp"
#include "nlsr.hpp"

#include <boost/lexical_cast.hpp>

namespace nlsr {

INIT_LOGGER(route.RoutingCalculatorLinkState);

class RoutingTableCalculator
{
public:
  RoutingTableCalculator(size_t nRouters)
  {
    m_nRouters = nRouters;
  }

protected:
  /*! \brief Allocate the space needed for the adj. matrix. */
  void
  allocateAdjMatrix();

  /*! \brief set NON_ADJACENT_COST i.e. -12345 to every cell of the matrix to
    ensure that the memory is safe. This is also to incorporate zero cost links */
  void
  initMatrix();

  /*! \brief Constructs an adj. matrix to calculate with.
    \param lsdb Reference to the Lsdb
    \param pMap The map to populate with the adj. data.
  */
  void
  makeAdjMatrix(const Lsdb& lsdb, NameMap& pMap);

  /*! \brief Writes a formated adjacent matrix to DEBUG log
    \param map The map containing adjacent matrix data
  */
  void
  writeAdjMatrixLog(const NameMap& map) const;

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
    temporary variables will not contain a NON_ADJACENT_COST (-12345) at i,
    but rather will contain the values for the next valid adjacency.
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
  {
  }

  void
  calculatePath(NameMap& pMap, RoutingTable& rt, ConfParameter& confParam,
                const Lsdb& lsdb);

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

    The cost between two nodes can be zero or greater than zero.

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
  addAllLsNextHopsToRoutingTable(AdjacencyList& adjacencies, RoutingTable& rt,
                                 NameMap& pMap, uint32_t sourceRouter);

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
};

constexpr int EMPTY_PARENT = -12345;
constexpr double INF_DISTANCE = 2147483647;
constexpr int NO_MAPPING_NUM = -1;
constexpr int NO_NEXT_HOP = -12345;

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
RoutingTableCalculator::makeAdjMatrix(const Lsdb& lsdb, NameMap& pMap)
{
  // For each LSA represented in the map
  auto lsaRange = lsdb.getLsdbIterator<AdjLsa>();
  for (auto lsaIt = lsaRange.first; lsaIt != lsaRange.second; ++lsaIt) {
    auto adjLsa = std::static_pointer_cast<AdjLsa>(*lsaIt);
    auto row = pMap.getMappingNoByRouterName(adjLsa->getOriginRouter());

    std::list<Adjacent> adl = adjLsa->getAdl().getAdjList();
    // For each adjacency represented in the LSA
    for (const auto& adjacent : adl) {
      auto col = pMap.getMappingNoByRouterName(adjacent.getName());
      double cost = adjacent.getLinkCost();

      if (row && col && *row < static_cast<int32_t>(m_nRouters)
          && *col < static_cast<int32_t>(m_nRouters)) {
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
RoutingTableCalculator::writeAdjMatrixLog(const NameMap& map) const
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
      if (adjMatrix[i][j] == NO_NEXT_HOP) {
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
LinkStateRoutingTableCalculator::calculatePath(NameMap& pMap, RoutingTable& rt,
                                               ConfParameter& confParam,
                                               const Lsdb& lsdb)
{
  NLSR_LOG_DEBUG("LinkStateRoutingTableCalculator::calculatePath Called");
  allocateAdjMatrix();
  initMatrix();
  makeAdjMatrix(lsdb, pMap);
  writeAdjMatrixLog(pMap);
  auto sourceRouter = pMap.getMappingNoByRouterName(confParam.getRouterPrefix());
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
                                                                RoutingTable& rt, NameMap& pMap,
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
        auto nextHopRouterName = pMap.getRouterNameByMappingNo(nextHopRouter);
        if (nextHopRouterName) {
          auto nextHopFace = adjacencies.getAdjacent(*nextHopRouterName).getFaceUri();
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

void
calculateLinkStateRoutingPath(NameMap& map, RoutingTable& rt, ConfParameter& confParam,
                              const Lsdb& lsdb)
{
  LinkStateRoutingTableCalculator calculator(map.size());
  calculator.calculatePath(map, rt, confParam, lsdb);
}

} // namespace nlsr
