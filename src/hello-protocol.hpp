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

#ifndef NLSR_HELLO_PROTOCOL_HPP
#define NLSR_HELLO_PROTOCOL_HPP

#include "statistics.hpp"
#include "test-access-control.hpp"
#include "conf-parameter.hpp"
#include "lsdb.hpp"
#include "route/routing-table.hpp"

#include <ndn-cxx/util/signal.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/security/validation-error.hpp>
#include <ndn-cxx/security/validator-config.hpp>

namespace nlsr {

class HelloProtocol
{
public:
  HelloProtocol(ndn::Face& face, ndn::KeyChain& keyChain, ConfParameter& confParam,
                RoutingTable& routingTable, Lsdb& lsdb);

  /*! \brief Sends a Hello Interest packet.
   *
   * \param interestNamePrefix The name of the router that has published the
   * update we want. Here that should be: \<router name\>/NLSR/INFO
   *
   * \param seconds The lifetime of the Interest we construct, in seconds
   *
   * This function attempts to contact neighboring routers to
   * determine their status (which currently is one of: ACTIVE,
   * INACTIVE, or UNKNOWN)
   */
  void
  expressInterest(const ndn::Name& interestNamePrefix, uint32_t seconds);

  /*! \brief Sends Hello Interests to all neighbors
   *
   * This function is called as part of a schedule to regularly
   * determine the adjacency status of neighbors. This function
   * creates and sends a Hello Interest to the given adjacent.
   *
   * \param neighbor the name of the neighbor
   */
  void
  sendHelloInterest(const ndn::Name& neighbor);

  /*! \brief Processes a Hello Interest from a neighbor.
   *
   * \param name (ignored)
   *
   * \param interest The Interest object that we have received and need to
   * process.
   *
   * Processes a Hello Interest that this router receives from one of
   * its neighbors. If the neighbor that sent the Interest does not
   * have a Face, NLSR will attempt to create one. Also, if the
   * neighbor that sent the Interest was previously marked as
   * INACTIVE, NLSR will attempt to contact it with its own Hello
   * Interest.
   */
  void
  processInterest(const ndn::Name& name, const ndn::Interest& interest);

  ndn::util::signal::Signal<HelloProtocol, Statistics::PacketType> hpIncrementSignal;

private:
  /*! \brief Try to contact a neighbor via Hello protocol again
   *
   * This function will re-send Hello Interests a configured number
   * of times. After that many failures, HelloProtocol will mark the neighbor as
   * inactive and will not attempt to contact them until the next time
   * HelloProtocol::sendScheduledInterest is called.
   *
   * \sa nlsr::ConfParameter::getInterestRetryNumber
   */
  void
  processInterestTimedOut(const ndn::Interest& interest);

  /*! \brief Verify signatures and validate incoming Hello data.
   */
  void
  onContent(const ndn::Interest& interest, const ndn::Data& data);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:

  /*! \brief Change a neighbor's status
   *
   * Whenever incoming Hello data is verified and validated, change
   * the status of this neighbor and then schedule an adjacency LSA
   * build for us. This also resets the number of times we've failed
   * to contact this neighbor so that we will retry later.
   */
  void
  onContentValidated(const ndn::Data& data);

private:
  /*! \brief Log that incoming data couldn't be validated, but do nothing else.
   */
  void
  onContentValidationFailed(const ndn::Data& data,
                            const ndn::security::ValidationError& ve);

public:
  ndn::util::Signal<HelloProtocol, const ndn::Name&> onInitialHelloDataValidated;

private:
  ndn::Face& m_face;
  ndn::Scheduler m_scheduler;
  ndn::security::KeyChain& m_keyChain;
  const ndn::security::SigningInfo& m_signingInfo;
  ConfParameter& m_confParam;
  RoutingTable& m_routingTable;
  Lsdb& m_lsdb;
  AdjacencyList& m_adjacencyList;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  static const std::string INFO_COMPONENT;
  static const std::string NLSR_COMPONENT;
};

} // namespace nlsr

#endif // NLSR_HELLO_PROTOCOL_HPP
