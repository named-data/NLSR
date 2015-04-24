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
#include "nlsr.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/management/nfd-control-response.hpp>
#include <ndn-cxx/util/regex.hpp>

namespace nlsr {

INIT_LOGGER("LsdbDatasetInterestHandler");

const uint32_t LsdbDatasetInterestHandler::ERROR_CODE_MALFORMED_COMMAND = 400;
const uint32_t LsdbDatasetInterestHandler::ERROR_CODE_UNSUPPORTED_COMMAND = 501;

LsdbDatasetInterestHandler::LsdbDatasetInterestHandler(Lsdb& lsdb,
                                                       ndn::Face& face,
                                                       const ndn::Name& routerName,
                                                       ndn::KeyChain& keyChain)
  : LOCALHOST_COMMAND_PREFIX(ndn::Name(Nlsr::LOCALHOST_PREFIX).append(Lsdb::NAME_COMPONENT))
  , ROUTER_NAME_COMMAND_PREFIX(ndn::Name(routerName).append(Lsdb::NAME_COMPONENT))
  , m_face(face)
  , m_keyChain(keyChain)
  , m_adjacencyLsaPublisher(lsdb, face, keyChain)
  , m_coordinateLsaPublisher(lsdb, face, keyChain)
  , m_nameLsaPublisher(lsdb, face, keyChain)
  , m_lsdbStatusPublisher(lsdb, face, keyChain,
                          m_adjacencyLsaPublisher,
                          m_coordinateLsaPublisher,
                          m_nameLsaPublisher)

{
}

void
LsdbDatasetInterestHandler::startListeningOnLocalhost()
{
  _LOG_DEBUG("Setting interest filter for: " << LOCALHOST_COMMAND_PREFIX);
  m_face.setInterestFilter(LOCALHOST_COMMAND_PREFIX,
                           std::bind(&LsdbDatasetInterestHandler::onInterest, this, _2,
                                     std::cref(LOCALHOST_COMMAND_PREFIX)));
}

void
LsdbDatasetInterestHandler::startListeningOnRouterPrefix()
{
  _LOG_DEBUG("Setting interest filter for: " << ROUTER_NAME_COMMAND_PREFIX);
  m_face.setInterestFilter(ROUTER_NAME_COMMAND_PREFIX,
                           std::bind(&LsdbDatasetInterestHandler::onInterest, this, _2,
                                     std::cref(ROUTER_NAME_COMMAND_PREFIX)));
}

void
LsdbDatasetInterestHandler::onInterest(const ndn::Interest& interest,
                                       const ndn::Name& commandPrefix)
{
  if (!isValidCommandPrefix(interest, commandPrefix))
  {
    _LOG_DEBUG("Received malformed interest: " << interest.getName());

    sendErrorResponse(interest.getName(), ERROR_CODE_MALFORMED_COMMAND, "Malformed command");
    return;
  }

  ndn::Name::Component command = interest.getName().get(commandPrefix.size());
  processCommand(interest, command);
}

bool
LsdbDatasetInterestHandler::isValidCommandPrefix(const ndn::Interest& interest,
                                                 const ndn::Name& commandPrefix)
{
  size_t commandSize = interest.getName().size();

  // Does the Interest match the command prefix with one additional component?
  return (commandSize == commandPrefix.size() + 1 && commandPrefix.isPrefixOf(interest.getName()));
}

void
LsdbDatasetInterestHandler::processCommand(const ndn::Interest& interest,
                                           const ndn::Name::Component& command)
{
  _LOG_TRACE("Received interest with command: " << command);

  if (command.equals(AdjacencyLsaPublisher::DATASET_COMPONENT)) {
    m_adjacencyLsaPublisher.publish(interest.getName());
  }
  else if (command.equals(CoordinateLsaPublisher::DATASET_COMPONENT)) {
    m_coordinateLsaPublisher.publish(interest.getName());
  }
  else if (command.equals(NameLsaPublisher::DATASET_COMPONENT)) {
    m_nameLsaPublisher.publish(interest.getName());
  }
  else if (command.equals(LsdbStatusPublisher::DATASET_COMPONENT)) {
    m_lsdbStatusPublisher.publish(interest.getName());
  }
  else {
    _LOG_DEBUG("Unsupported command: " << command);
    sendErrorResponse(interest.getName(), ERROR_CODE_UNSUPPORTED_COMMAND, "Unsupported command");
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
