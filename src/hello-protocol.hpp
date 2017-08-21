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

#ifndef NLSR_HELLO_PROTOCOL_HPP
#define NLSR_HELLO_PROTOCOL_HPP

#include "statistics.hpp"
#include "test-access-control.hpp"

#include <ndn-cxx/util/signal.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/security/v2/validation-error.hpp>

namespace nlsr {

class Nlsr;

class HelloProtocol
{
public:
  HelloProtocol(Nlsr& nlsr, ndn::Scheduler& scheduler);

  /*! \brief Schedules a Hello Interest event.
   *
   * This function serves as the Hello Interest loop, and must be
   * explicitly called to start the Hello cycle. This is done at
   * NLSR's initialization.
   *
   * \sa Nlsr::initialize
   * \param seconds The number of seconds to wait before calling the event.
   */
  void
  scheduleInterest(uint32_t seconds);

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
   * \param seconds (ignored)
   *
   * This function is called as part of a schedule to regularly
   * determine the adjacency status of neighbors. This function
   * creates and sends a Hello Interest to each neighbor in
   * Nlsr::m_adjacencyList. If the neighbor has not been contacted
   * before and curerntly has no Face in NFD, this method will call a
   * different pipeline that creates the Face first, then registers
   * prefixes.
   */
  void
  sendScheduledInterest(uint32_t seconds);

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
                            const ndn::security::v2::ValidationError& ve);

  /*! \brief Treat a failed Face registration as an INACTIVE neighbor.
   *
   * If NLSR fails to register a Face when contacting a neighbor, it
   * will instantly toggle that neighbor to INACTIVE. This is
   * necessary because NLSR will put off building its own adjacency
   * LSA until the status of each neighbor is definitively
   * known. Without this, NLSR might have to wait many scheduled Hello
   * intervals to finish building an adjacency LSA.
   */
  void
  onRegistrationFailure(const ndn::nfd::ControlResponse& response,
                        const ndn::Name& name);

  /*! \brief Set up a Face for NLSR use.
   *
   * When NLSR receives a Hello Interest from a neighbor that it has
   * not seen before, it may need to create a Face for that
   * neighbor. After doing so, it will be necessary to inform NFD
   * about the standard prefixes that NLSR needs a node to have in
   * order to conduct normal operations. This function accomplishes
   * that, and then sends its own Hello Interest to confirm the
   * contact.
   */
  void
  onRegistrationSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                        const ndn::Name& neighbor, const ndn::time::milliseconds& timeout);

  /*! \brief Create a Face for an adjacency
   * \sa HelloProtocol::onRegistrationSuccess
   */
  void
  registerPrefixes(const ndn::Name& adjName, const std::string& faceUri,
                   double linkCost, const ndn::time::milliseconds& timeout);
private:
  Nlsr& m_nlsr;
  ndn::Scheduler& m_scheduler;

  static const std::string INFO_COMPONENT;
  static const std::string NLSR_COMPONENT;
};

} // namespace nlsr

#endif // NLSR_HELLO_PROTOCOL_HPP
