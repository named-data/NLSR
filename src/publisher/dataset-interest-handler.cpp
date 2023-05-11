/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023,  The University of Memphis,
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

#include "dataset-interest-handler.hpp"
#include "nlsr.hpp"
#include "logger.hpp"

#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/util/regex.hpp>

namespace nlsr {

INIT_LOGGER(DatasetInterestHandler);

const ndn::PartialName ADJACENCIES_DATASET{"lsdb/adjacencies"};
const ndn::PartialName COORDINATES_DATASET{"lsdb/coordinates"};
const ndn::PartialName NAMES_DATASET{"lsdb/names"};
const ndn::PartialName RT_DATASET{"routing-table"};

DatasetInterestHandler::DatasetInterestHandler(ndn::mgmt::Dispatcher& dispatcher,
                                               const Lsdb& lsdb,
                                               const RoutingTable& rt)
  : m_lsdb(lsdb)
  , m_routingTable(rt)
{
  dispatcher.addStatusDataset(ADJACENCIES_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&DatasetInterestHandler::publishLsaStatus<AdjLsa>, this, _1, _2, _3));
  dispatcher.addStatusDataset(COORDINATES_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&DatasetInterestHandler::publishLsaStatus<CoordinateLsa>, this, _1, _2, _3));
  dispatcher.addStatusDataset(NAMES_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&DatasetInterestHandler::publishLsaStatus<NameLsa>, this, _1, _2, _3));
  dispatcher.addStatusDataset(RT_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&DatasetInterestHandler::publishRtStatus, this, _1, _2, _3));
}

template <typename T>
void
DatasetInterestHandler::publishLsaStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                         ndn::mgmt::StatusDatasetContext& context)
{
  NLSR_LOG_TRACE("Received interest: " << interest);
  auto lsaRange = m_lsdb.getLsdbIterator<T>();
  for (auto lsaIt = lsaRange.first; lsaIt != lsaRange.second; ++lsaIt) {
    context.append((*lsaIt)->wireEncode());
  }
  context.end();
}

void
DatasetInterestHandler::publishRtStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                        ndn::mgmt::StatusDatasetContext& context)
{
  NLSR_LOG_TRACE("Received interest: " << interest);
  context.append(m_routingTable.wireEncode());
  context.end();
}

} // namespace nlsr
