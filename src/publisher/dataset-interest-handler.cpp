/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
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
 **/

#include "dataset-interest-handler.hpp"
#include "nlsr.hpp"
#include "logger.hpp"

#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/util/regex.hpp>

namespace nlsr {

INIT_LOGGER(DatasetInterestHandler);

const ndn::PartialName ADJACENCIES_DATASET = ndn::PartialName("lsdb/adjacencies");
const ndn::PartialName COORDINATES_DATASET = ndn::PartialName("lsdb/coordinates");
const ndn::PartialName NAMES_DATASET = ndn::PartialName("lsdb/names");
const ndn::PartialName RT_DATASET = ndn::PartialName("routing-table");

DatasetInterestHandler::DatasetInterestHandler(ndn::mgmt::Dispatcher& dispatcher,
                                               const Lsdb& lsdb,
                                               const RoutingTable& rt)
  : m_dispatcher(dispatcher)
  , m_lsdb(lsdb)
  , m_routingTableEntries(rt.getRoutingTableEntry())
  , m_dryRoutingTableEntries(rt.getDryRoutingTableEntry())
{
  setDispatcher(m_dispatcher);
}

void
DatasetInterestHandler::setDispatcher(ndn::mgmt::Dispatcher& dispatcher)
{
  dispatcher.addStatusDataset(ADJACENCIES_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&DatasetInterestHandler::publishAdjStatus, this, _1, _2, _3));
  dispatcher.addStatusDataset(COORDINATES_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&DatasetInterestHandler::publishCoordinateStatus, this, _1, _2, _3));
  dispatcher.addStatusDataset(NAMES_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&DatasetInterestHandler::publishNameStatus, this, _1, _2, _3));
  dispatcher.addStatusDataset(RT_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&DatasetInterestHandler::publishRtStatus, this, _1, _2, _3));
}

void
DatasetInterestHandler::publishAdjStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                         ndn::mgmt::StatusDatasetContext& context)
{
  for (const auto& adjLsa : m_lsdb.getAdjLsdb()) {
    context.append(adjLsa.wireEncode());
  }
  context.end();
}

void
DatasetInterestHandler::publishCoordinateStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                                ndn::mgmt::StatusDatasetContext& context)
{
  for (const auto& coordinateLsa : m_lsdb.getCoordinateLsdb()) {
    context.append(coordinateLsa.wireEncode());
  }
  context.end();
}

void
DatasetInterestHandler::publishNameStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                          ndn::mgmt::StatusDatasetContext& context)
{
  for (const auto& nameLsa : m_lsdb.getNameLsdb()) {
    context.append(nameLsa.wireEncode());
  }
  context.end();
}


std::vector<tlv::RoutingTable>
DatasetInterestHandler::getTlvRTEntries()
{
  std::vector<tlv::RoutingTable> rtable;
  for (const auto& rte : m_routingTableEntries) {
    tlv::RoutingTable tlvRoutingTable;
    std::shared_ptr<tlv::Destination> tlvDes = tlv::makeDes(rte);
    tlvRoutingTable.setDestination(*tlvDes);
    for (const auto& nh : rte.getNexthopList().getNextHops()) {
      tlv::NextHop tlvNexthop;
      tlvNexthop.setUri(nh.getConnectingFaceUri());
      tlvNexthop.setCost(nh.getRouteCost());
      tlvRoutingTable.addNexthops(tlvNexthop);
    }
    rtable.push_back(tlvRoutingTable);
  }
  if (!m_dryRoutingTableEntries.empty()) {
    for (const auto& dryRte : m_dryRoutingTableEntries) {
      tlv::RoutingTable tlvRoutingTable;
      std::shared_ptr<tlv::Destination> tlvDes = tlv::makeDes(dryRte);
      tlvRoutingTable.setDestination(*tlvDes);
      for (const auto& nh : dryRte.getNexthopList().getNextHops()) {
        tlv::NextHop tlvNexthop;
        tlvNexthop.setUri(nh.getConnectingFaceUri());
        tlvNexthop.setCost(nh.getRouteCost());
        tlvRoutingTable.addNexthops(tlvNexthop);
      }
      rtable.push_back(tlvRoutingTable);
    }
  }
  return rtable;
}

void
DatasetInterestHandler::publishRtStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                        ndn::mgmt::StatusDatasetContext& context)
{
  NLSR_LOG_DEBUG("Received interest:  " << interest);
  tlv::RoutingTableStatus rtStatus;
  for (const tlv::RoutingTable& rt : getTlvRTEntries()) {
    rtStatus.addRoutingTable(rt);
  }
  const ndn::Block& wire = rtStatus.wireEncode();
  context.append(wire);
  context.end();
}

} // namespace nlsr
