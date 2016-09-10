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

#ifndef NLSR_HELLO_PROTOCOL_HPP
#define NLSR_HELLO_PROTOCOL_HPP

#include "test-access-control.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/util/scheduler.hpp>

namespace nlsr {

class Nlsr;

class HelloProtocol
{
public:
  HelloProtocol(Nlsr& nlsr, ndn::Scheduler& scheduler)
    : m_nlsr(nlsr)
    , m_scheduler(scheduler)
  {
  }

  /*! \brief Schedules a hello Interest event.

    \param seconds The number of seconds to wait before calling the event.
   */
  void
  scheduleInterest(uint32_t seconds);

  /*! \brief Sends a hello Interest packet.

    \param interestNamePrefix The name of the router that has published the
    update we want.  Here that should be: \<router name\>/NLSR/INFO

    \param seconds The lifetime of the Interest we construct, in seconds

    This function attempts to contact neighboring routers to
    determine their status (which currently is one of: ACTIVE,
    INACTIVE, or UNKNOWN)
   */
  void
  expressInterest(const ndn::Name& interestNamePrefix, uint32_t seconds);

  /*! \brief Sends a hello Interest packet that was previously scheduled.

    \param seconds (ignored)

    This function is called as part of a schedule to regularly
    determine the adjacency status of neighbors. This function checks
    if the specified neighbor has a Face, and if not creates one. If
    the neighbor had a Face, it calls \c expressInterest, else it will
    attempt to create a Face, which will itself attempt to contact the
    neighbor. Then the function schedules for this function to be invoked again.
   */
  void
  sendScheduledInterest(uint32_t seconds);

  /*! \brief Processes a hello Interest from a neighbor.

    \param name (ignored)

    \param interest The Interest object that we have received and need to
    process.

    Processes a hello Interest that this router receives from one of
    its neighbors. If the neighbor that sent the Interest does not
    have a Face, NLSR will attempt to register one. Also, if the
    neighbor that sent the Interest was previously marked as INACTIVE,
    NLSR will attempt to contact it with its own hello Interest.
   */
  void
  processInterest(const ndn::Name& name, const ndn::Interest& interest);

private:
  void
  processInterestTimedOut(const ndn::Interest& interest);

  void
  onContent(const ndn::Interest& interest, const ndn::Data& data);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  void
  onContentValidated(const ndn::shared_ptr<const ndn::Data>& data);

private:
  void
  onContentValidationFailed(const ndn::shared_ptr<const ndn::Data>& data,
                            const std::string& msg);

  void
  onRegistrationFailure(const ndn::nfd::ControlResponse& response,
                        const ndn::Name& name);

  void
  onRegistrationSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                        const ndn::Name& neighbor, const ndn::time::milliseconds& timeout);

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
