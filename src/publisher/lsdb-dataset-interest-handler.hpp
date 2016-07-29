/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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

#ifndef NLSR_PUBLISHER_LSDB_DATASET_INTEREST_HANDLER_HPP
#define NLSR_PUBLISHER_LSDB_DATASET_INTEREST_HANDLER_HPP

#include "lsa-publisher.hpp"
#include "lsdb-status-publisher.hpp"

#include <ndn-cxx/face.hpp>

namespace nlsr {

/**
 * @brief Abstraction to publish all lsa dataset
 * \sa http://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class LsdbDatasetInterestHandler
{
public:
  class Error : std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

  LsdbDatasetInterestHandler(Lsdb& lsdb,
                             ndn::Face& face,
                             ndn::KeyChain& keyChain);

  void
  startListeningOnLocalhost();

  void
  startListeningOnRouterPrefix();

  const ndn::Name&
  getLocalhostCommandPrefix()
  {
    return LOCALHOST_COMMAND_PREFIX;
  }

  ndn::Name&
  getRouterNameCommandPrefix()
  {
    return m_routerNameCommandPrefix;
  }

  void
  setRouterNameCommandPrefix(const ndn::Name& routerName) {
    m_routerNameCommandPrefix = routerName;
    m_routerNameCommandPrefix.append(Lsdb::NAME_COMPONENT);
  }

private:
  void
  onInterest(const ndn::Interest& interest, const ndn::Name& commandPrefix);

  bool
  isValidCommandPrefix(const ndn::Interest& interest, const ndn::Name& commandPrefix);

  void
  processCommand(const ndn::Interest& interest, const ndn::Name::Component& command);

  void
  sendErrorResponse(const ndn::Name& name, uint32_t code, const std::string& error);

private:
  const ndn::Name LOCALHOST_COMMAND_PREFIX;
  ndn::Name m_routerNameCommandPrefix;

  ndn::Face& m_face;
  ndn::KeyChain& m_keyChain;

  AdjacencyLsaPublisher m_adjacencyLsaPublisher;
  CoordinateLsaPublisher m_coordinateLsaPublisher;
  NameLsaPublisher m_nameLsaPublisher;
  LsdbStatusPublisher m_lsdbStatusPublisher;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  static const uint32_t ERROR_CODE_MALFORMED_COMMAND;
  static const uint32_t ERROR_CODE_UNSUPPORTED_COMMAND;
};

} // namespace nlsr

#endif // NLSR_PUBLISHER_LSDB_DATASET_INTEREST_HANDLER_HPP
