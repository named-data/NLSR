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

#include "lsdb-dataset-interest-handler.hpp"

#include "logger.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/management/nfd-control-response.hpp>
#include <ndn-cxx/util/regex.hpp>

namespace nlsr {

INIT_LOGGER("LsdbDatasetInterestHandler");

LsdbDatasetInterestHandler::LsdbDatasetInterestHandler(Lsdb& lsdb,
                                                       ndn::Face& face,
                                                       const ndn::Name& routerName,
                                                       ndn::KeyChain& keyChain)
  : COMMAND_PREFIX(ndn::Name(routerName).append(Lsdb::NAME_COMPONENT))
  , m_face(face)
  , m_keyChain(keyChain)
  , m_adjacencyLsaPublisher(lsdb, face, COMMAND_PREFIX, keyChain)
  , m_coordinateLsaPublisher(lsdb, face, COMMAND_PREFIX, keyChain)
  , m_nameLsaPublisher(lsdb, face, COMMAND_PREFIX, keyChain)
  , m_lsdbStatusPublisher(lsdb, face, COMMAND_PREFIX, keyChain,
                          m_adjacencyLsaPublisher,
                          m_coordinateLsaPublisher,
                          m_nameLsaPublisher)

{
  _LOG_DEBUG("Setting interest filter for: " << COMMAND_PREFIX);
  m_face.setInterestFilter(COMMAND_PREFIX,
                           std::bind(&LsdbDatasetInterestHandler::onInterest, this, _2));
}

void
LsdbDatasetInterestHandler::onInterest(const ndn::Interest& interest)
{
  // Does interest match command prefix with one additional component?
  if (interest.getName().size() != COMMAND_PREFIX.size() + 1 ||
      !COMMAND_PREFIX.isPrefixOf(interest.getName()))
  {
    _LOG_DEBUG("Received malformed interest: " << interest.getName());

    sendErrorResponse(interest.getName(), 400, "Malformed command");
    return;
  }

  ndn::Name::Component command = interest.getName().get(COMMAND_PREFIX.size());
  _LOG_TRACE("Received interest with command: " << command);

  if (command.equals(AdjacencyLsaPublisher::DATASET_COMPONENT)) {
    m_adjacencyLsaPublisher.publish();
  }
  else if (command.equals(CoordinateLsaPublisher::DATASET_COMPONENT)) {
    m_coordinateLsaPublisher.publish();
  }
  else if (command.equals(NameLsaPublisher::DATASET_COMPONENT)) {
    m_nameLsaPublisher.publish();
  }
  else if (command.equals(LsdbStatusPublisher::DATASET_COMPONENT)) {
    m_lsdbStatusPublisher.publish();
  }
  else {
    _LOG_DEBUG("Unsupported command: " << command);
    sendErrorResponse(interest.getName(), 501, "Unsupported command");
  }
}

void
LsdbDatasetInterestHandler::sendErrorResponse(const ndn::Name& name,
                                              uint32_t code,
                                              const std::string& error)
{
  ndn::nfd::ControlResponse response(code, error);

  std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>(name);
  data->setContent(response.wireEncode());

  m_keyChain.sign(*data);
  m_face.put(*data);
}

} // namespace nlsr
