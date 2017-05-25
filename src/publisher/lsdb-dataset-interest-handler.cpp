/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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

/*! \file lsdb-dataset-interest-handler.cpp

  This file details a class that is used by NLSRC and other command-line
  tools to examine the state of NLSR. This system is not designed to
  be used by routers to publish data to each other.
 */

#include "lsdb-dataset-interest-handler.hpp"

#include "logger.hpp"
#include "nlsr.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/util/regex.hpp>
#include "tlv/lsdb-status.hpp"

namespace nlsr {

INIT_LOGGER("LsdbDatasetInterestHandler");
const ndn::PartialName ADJACENCIES_DATASET = ndn::PartialName("lsdb/adjacencies");
const ndn::PartialName COORDINATES_DATASET = ndn::PartialName("lsdb/coordinates");
const ndn::PartialName NAMES_DATASET = ndn::PartialName("lsdb/names");
const ndn::PartialName LISTS_DATASET = ndn::PartialName("lsdb/list");
const ndn::PartialName LsdbDatasetInterestHandler::LOCALHOST_COMMAND_PREFIX =
      ndn::Name(Nlsr::LOCALHOST_PREFIX).append(Lsdb::NAME_COMPONENT);

LsdbDatasetInterestHandler::LsdbDatasetInterestHandler(Lsdb& lsdb,
                                                       ndn::mgmt::Dispatcher& localHostDispatcher,
                                                       ndn::mgmt::Dispatcher& routerNameDispatcher,
                                                       ndn::Face& face,
                                                       ndn::KeyChain& keyChain)
  : m_localhostDispatcher(localHostDispatcher)
  , m_routerNameDispatcher(routerNameDispatcher)
  , m_adjacencyLsaPublisher(lsdb, face, keyChain)
  , m_coordinateLsaPublisher(lsdb, face, keyChain)
  , m_nameLsaPublisher(lsdb, face, keyChain)
  , m_adjacencyLsas(lsdb.getAdjLsdb())
  , m_coordinateLsas(lsdb.getCoordinateLsdb())
  , m_nameLsas(lsdb.getNameLsdb())
{
  _LOG_DEBUG("Setting dispatcher for lsdb status dataset:");
  setDispatcher(m_localhostDispatcher);
  setDispatcher(m_routerNameDispatcher);
}

void
LsdbDatasetInterestHandler::setDispatcher(ndn::mgmt::Dispatcher& dispatcher)
{
  dispatcher.addStatusDataset(ADJACENCIES_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&LsdbDatasetInterestHandler::publishAdjStatus, this, _1, _2, _3));
  dispatcher.addStatusDataset(COORDINATES_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&LsdbDatasetInterestHandler::publishCoordinateStatus, this, _1, _2, _3));
  dispatcher.addStatusDataset(NAMES_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&LsdbDatasetInterestHandler::publishNameStatus, this, _1, _2, _3));
  dispatcher.addStatusDataset(LISTS_DATASET,
    ndn::mgmt::makeAcceptAllAuthorization(),
    std::bind(&LsdbDatasetInterestHandler::publishAllStatus, this, _1, _2, _3));
}

void
LsdbDatasetInterestHandler::publishAdjStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                             ndn::mgmt::StatusDatasetContext& context)
{
  _LOG_DEBUG("Received interest:  " << interest);
  for (AdjLsa lsa : m_adjacencyLsas) {
    tlv::AdjacencyLsa tlvLsa;
    std::shared_ptr<tlv::LsaInfo> tlvLsaInfo = tlv::makeLsaInfo(lsa);
    tlvLsa.setLsaInfo(*tlvLsaInfo);

    for (const Adjacent& adj : lsa.getAdl().getAdjList()) {
      tlv::Adjacency tlvAdj;
      tlvAdj.setName(adj.getName());
      tlvAdj.setUri(adj.getFaceUri().toString());
      tlvAdj.setCost(adj.getLinkCost());
      tlvLsa.addAdjacency(tlvAdj);
    }
    const ndn::Block& wire = tlvLsa.wireEncode();
    context.append(wire);
  }
  context.end();
}

void
LsdbDatasetInterestHandler::publishCoordinateStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                                    ndn::mgmt::StatusDatasetContext& context)
{
  _LOG_DEBUG("Received interest:  " << interest);
  for (const CoordinateLsa lsa : m_coordinateLsas) {
    tlv::CoordinateLsa tlvLsa;
    std::shared_ptr<tlv::LsaInfo> tlvLsaInfo = tlv::makeLsaInfo(lsa);
    tlvLsa.setLsaInfo(*tlvLsaInfo);

    tlvLsa.setHyperbolicRadius(lsa.getCorRadius());
    tlvLsa.setHyperbolicAngle(lsa.getCorTheta());

    const ndn::Block& wire = tlvLsa.wireEncode();
    context.append(wire);
  }
  context.end();
}

void
LsdbDatasetInterestHandler::publishNameStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                              ndn::mgmt::StatusDatasetContext& context)
{
  _LOG_DEBUG("Received interest:  " << interest);
  for (NameLsa lsa : m_nameLsas) {
    tlv::NameLsa tlvLsa;

    std::shared_ptr<tlv::LsaInfo> tlvLsaInfo = tlv::makeLsaInfo(lsa);
    tlvLsa.setLsaInfo(*tlvLsaInfo);

    for (const ndn::Name& name : lsa.getNpl().getNameList()) {
      tlvLsa.addName(name);
    }

    const ndn::Block& wire = tlvLsa.wireEncode();
    context.append(wire);
  }
  context.end();
}

void
LsdbDatasetInterestHandler::publishAllStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                                             ndn::mgmt::StatusDatasetContext& context)
{
  _LOG_DEBUG("Received interest:  " << interest);
  tlv::LsdbStatus lsdbStatus;
  for (const tlv::AdjacencyLsa& tlvLsa : m_adjacencyLsaPublisher.getTlvLsas()) {
    lsdbStatus.addAdjacencyLsa(tlvLsa);
  }

  for (const tlv::CoordinateLsa& tlvLsa : m_coordinateLsaPublisher.getTlvLsas()) {
    lsdbStatus.addCoordinateLsa(tlvLsa);
  }

  for (const tlv::NameLsa& tlvLsa : m_nameLsaPublisher.getTlvLsas()) {
    lsdbStatus.addNameLsa(tlvLsa);
  }
  const ndn::Block& wire = lsdbStatus.wireEncode();
  context.append(wire);
  context.end();
}

} // namespace nlsr
