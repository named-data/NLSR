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

#ifndef NLSR_TESTS_TEST_COMMON_HPP
#define NLSR_TESTS_TEST_COMMON_HPP

#include "conf-parameter.hpp"

#include "tests/boost-test.hpp"

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr::tests {

/**
 * \brief Create a Data with a null (i.e., empty) signature
 *
 * If a "real" signature is desired, use KeyChainFixture and sign again with `m_keyChain`.
 */
std::shared_ptr<ndn::Data>
makeData(const ndn::Name& name);

/**
 * \brief Add a null signature to \p data
 */
ndn::Data&
signData(ndn::Data& data);

/**
 * \brief Add a null signature to \p data
 */
inline std::shared_ptr<ndn::Data>
signData(std::shared_ptr<ndn::Data> data)
{
  signData(*data);
  return data;
}

void
checkPrefixRegistered(const ndn::DummyClientFace& face, const ndn::Name& prefix);

class DummyConfFileProcessor
{
public:
  DummyConfFileProcessor(ConfParameter& conf,
                         SyncProtocol protocol = SyncProtocol::PSYNC,
                         HyperbolicState hyperbolicState = HYPERBOLIC_STATE_OFF,
                         const ndn::Name& networkName = "/ndn",
                         const ndn::Name& siteName = "/site",
                         const ndn::Name& routerName = "/%C1.Router/this-router")
  {
    conf.setNetwork(networkName);
    conf.setSiteName(siteName);
    conf.setRouterName(routerName);
    conf.buildRouterAndSyncUserPrefix();
    conf.setSyncProtocol(protocol);
    conf.setHyperbolicState(hyperbolicState);
  }
};

} // namespace nlsr::tests

#endif // NLSR_TESTS_TEST_COMMON_HPP
