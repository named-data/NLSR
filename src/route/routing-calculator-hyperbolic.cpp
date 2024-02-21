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

#include <cmath>

namespace nlsr {

INIT_LOGGER(route.RoutingCalculatorHyperbolic);

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
  calculatePath(NameMap& map, RoutingTable& rt, Lsdb& lsdb, AdjacencyList& adjacencies);

private:
  double
  getHyperbolicDistance(Lsdb& lsdb, ndn::Name src, ndn::Name dest);

  void
  addNextHop(const ndn::Name& destinationRouter, const ndn::FaceUri& faceUri, double cost, RoutingTable& rt);

  double
  calculateHyperbolicDistance(double rI, double rJ, double deltaTheta);

  double
  calculateAngularDistance(std::vector<double> angleVectorI,
                           std::vector<double> angleVectorJ);

private:
  const size_t m_nRouters;
  const bool m_isDryRun;
  const ndn::Name m_thisRouterName;
};

constexpr double UNKNOWN_DISTANCE = -1.0;
constexpr double UNKNOWN_RADIUS   = -1.0;

void
HyperbolicRoutingCalculator::calculatePath(NameMap& map, RoutingTable& rt,
                                           Lsdb& lsdb, AdjacencyList& adjacencies)
{
  NLSR_LOG_TRACE("Calculating hyperbolic paths");

  auto thisRouter = map.getMappingNoByRouterName(m_thisRouterName);

  // Iterate over directly connected neighbors
  std::list<Adjacent> neighbors = adjacencies.getAdjList();
  for (auto adj = neighbors.begin(); adj != neighbors.end(); ++adj) {

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

    // Install nexthops for this router to the neighbor; direct neighbors have a 0 cost link
    addNextHop(srcRouterName, adj->getFaceUri(), 0, rt);

    auto src = map.getMappingNoByRouterName(srcRouterName);
    if (!src) {
      NLSR_LOG_WARN(adj->getName() << " does not exist in the router map!");
      continue;
    }

    // Get hyperbolic distance from direct neighbor to every other router
    for (int dest = 0; dest < static_cast<int>(m_nRouters); ++dest) {
      // Don't calculate nexthops to this router or from a router to itself
      if (thisRouter && dest != *thisRouter && dest != *src) {

        auto destRouterName = map.getRouterNameByMappingNo(dest);
        if (destRouterName) {
          double distance = getHyperbolicDistance(lsdb, srcRouterName, *destRouterName);

          // Could not compute distance
          if (distance == UNKNOWN_DISTANCE) {
            NLSR_LOG_WARN("Could not calculate hyperbolic distance from " << srcRouterName
                           << " to " << *destRouterName);
            continue;
          }
          addNextHop(*destRouterName, adj->getFaceUri(), distance, rt);
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

  std::vector<double> srcTheta = srcLsa->getTheta();
  std::vector<double> destTheta = destLsa->getTheta();

  double srcRadius = srcLsa->getRadius();
  double destRadius = destLsa->getRadius();

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
HyperbolicRoutingCalculator::addNextHop(const ndn::Name& dest, const ndn::FaceUri& faceUri,
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

void
calculateHyperbolicRoutingPath(NameMap& map, RoutingTable& rt, Lsdb& lsdb,
                               AdjacencyList& adjacencies, ndn::Name thisRouterName,
                               bool isDryRun)
{
  HyperbolicRoutingCalculator calculator(map.size(), isDryRun, thisRouterName);
  calculator.calculatePath(map, rt, lsdb, adjacencies);
}

} // namespace nlsr
