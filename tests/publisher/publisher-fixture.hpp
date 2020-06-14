/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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

#ifndef NLSR_PUBLISHER_FIXTURE_HPP
#define NLSR_PUBLISHER_FIXTURE_HPP

#include "publisher/dataset-interest-handler.hpp"
#include "nlsr.hpp"

#include "tests/test-common.hpp"

#include <ndn-cxx/util/dummy-client-face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/pib/identity.hpp>

#include <ndn-cxx/util/io.hpp>

#include <boost/filesystem.hpp>

using namespace ndn;

namespace nlsr {
namespace test {

class PublisherFixture : public BaseFixture
{
public:
  PublisherFixture()
    : face(m_ioService, m_keyChain, {true, true})
    , conf(face, m_keyChain)
    , confProcessor(conf)
    , nlsr(face, m_keyChain, conf)
    , lsdb(nlsr.m_lsdb)
    , rt1(nlsr.m_routingTable)
  {
    routerId = addIdentity(conf.getRouterPrefix());
    face.processEvents(ndn::time::milliseconds(100));
  }

  void
  addAdjacency(AdjLsa& lsa, const std::string& name, const std::string& faceUri, double cost)
  {
    Adjacent adjacency(name, ndn::FaceUri(faceUri), cost, Adjacent::STATUS_ACTIVE, 0, 0);
    lsa.addAdjacent(std::move(adjacency));
  }

  NextHop
  createNextHop(const std::string& faceUri, double cost)
  {
    NextHop nexthop(faceUri, cost);
    return nexthop;
  }

  CoordinateLsa
  createCoordinateLsa(const std::string& origin, double radius, std::vector<double> angle)
  {
    CoordinateLsa lsa(origin, 1, ndn::time::system_clock::now(),
                      radius, angle);
    return lsa;
  }

public:
  ndn::util::DummyClientFace face;
  ConfParameter conf;
  DummyConfFileProcessor confProcessor;
  Nlsr nlsr;
  Lsdb& lsdb;

  ndn::security::pib::Identity routerId;
  RoutingTable& rt1;
};

} // namespace test
} // namespace nlsr

#endif // NLSR_PUBLISHER_FIXTURE_HPP
