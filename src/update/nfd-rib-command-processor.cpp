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

#include "nfd-rib-command-processor.hpp"
#include "logger.hpp"

namespace nlsr {
namespace update {

INIT_LOGGER("NfdRibProcessor");

const ndn::PartialName REGISTER_VERB = ndn::PartialName("rib/register");
const ndn::PartialName UNREGISTER_VERB = ndn::PartialName("rib/unregister");
const ndn::Name COMMAND_PREFIX = ndn::Name("/localhost/nlsr");

NfdRibCommandProcessor::NfdRibCommandProcessor(ndn::mgmt::Dispatcher& dispatcher,
                                               NamePrefixList& namePrefixes,
                                               Lsdb& lsdb,
                                               SyncLogicHandler& sync)
  : m_dispatcher(dispatcher)
  , m_namePrefixList(namePrefixes)
  , m_lsdb(lsdb)
  , m_sync(sync)
{
}

void
NfdRibCommandProcessor::startListening()
{
  _LOG_DEBUG("Registering control command prefixes for: " << REGISTER_VERB <<
             " and " << UNREGISTER_VERB);
  m_dispatcher.addControlCommand<ndn::nfd::ControlParameters>(REGISTER_VERB,
                                 ndn::mgmt::makeAcceptAllAuthorization(),
                                 std::bind(&NfdRibCommandProcessor::validate<NfdRibRegisterCommand>,
                                           this, _1, REGISTER_VERB),
                                 [this] (const ndn::Name& prefix, const ndn::Interest& interest,
                                         const ndn::mgmt::ControlParameters& parameters,
                                         const ndn::mgmt::CommandContinuation& done) {
                                   _LOG_DEBUG("Params verified, calling insertPrefix()");
                                   this->insertPrefix(parameters);
                                 });
  m_dispatcher.addControlCommand<ndn::nfd::ControlParameters>(UNREGISTER_VERB,
                                 ndn::mgmt::makeAcceptAllAuthorization(),
                                 std::bind(&NfdRibCommandProcessor::validate<NfdRibUnregisterCommand>,
                                           this, _1, UNREGISTER_VERB),
                                 [this] (const ndn::Name& prefix, const ndn::Interest& interest,
                                         const ndn::mgmt::ControlParameters& parameters,
                                         const ndn::mgmt::CommandContinuation& done) {
                                   _LOG_DEBUG("Params verified, calling removePrefix()");
                                   this->removePrefix(parameters);
                                 });
}

template<typename T>
bool
NfdRibCommandProcessor::validate(const ndn::mgmt::ControlParameters& parameters,
                                 const ndn::PartialName& command)
{
  _LOG_DEBUG("Attempting to verify incoming params for " << command <<
             " command...");
  bool wasValidated = true;
  try {
      wasValidated = this->validateParameters<T>(parameters);
  } catch (const ndn::nfd::ControlCommand::ArgumentError& ae) {
    _LOG_DEBUG("Could not parse params. Message: " << ae.what());
    wasValidated = false;
  }
  return wasValidated;
}

void
NfdRibCommandProcessor::insertPrefix(const ndn::mgmt::ControlParameters& parameters)
{
  const ndn::nfd::ControlParameters& castParams =
    static_cast<const ndn::nfd::ControlParameters&>(parameters);

  _LOG_DEBUG("Inserting prefix into the FIB: " << castParams.getName() << "\n");

  if (m_namePrefixList.insert(castParams.getName())) {
    m_lsdb.buildAndInstallOwnNameLsa();
    m_sync.publishRoutingUpdate();
  }
}

void
NfdRibCommandProcessor::removePrefix(const ndn::mgmt::ControlParameters& parameters)
{
  const ndn::nfd::ControlParameters& castParams =
    static_cast<const ndn::nfd::ControlParameters&>(parameters);

  _LOG_DEBUG("Removing prefix from the FIB: " << castParams.getName() << "\n");

  if (m_namePrefixList.remove(castParams.getName())) {
    m_lsdb.buildAndInstallOwnNameLsa();
    m_sync.publishRoutingUpdate();
  }
}

} // namespace update
} // namespace nlsr
