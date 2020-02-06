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

#include "manager-base.hpp"

namespace nlsr {
namespace update {

INIT_LOGGER(update.PrefixCommandProcessor);

ManagerBase::ManagerBase(ndn::mgmt::Dispatcher& dispatcher,
                         const std::string& module)
  : m_dispatcher(dispatcher)
  , m_module(module)
{
}

ndn::PartialName
ManagerBase::makeRelPrefix(const std::string& verb) const
{
  return ndn::PartialName(m_module).append(verb);
}

CommandManagerBase::CommandManagerBase(ndn::mgmt::Dispatcher& dispatcher,
                                      NamePrefixList& namePrefixList,
                                      Lsdb& lsdb,
                                      const std::string& module)
  : ManagerBase(dispatcher, module)
  , m_namePrefixList(namePrefixList)
  , m_lsdb(lsdb)
{
}

void
CommandManagerBase::advertiseAndInsertPrefix(const ndn::Name& prefix,
                                             const ndn::Interest& interest,
                                             const ndn::mgmt::ControlParameters& parameters,
                                             const ndn::mgmt::CommandContinuation& done)
{
  const ndn::nfd::ControlParameters& castParams =
    static_cast<const ndn::nfd::ControlParameters&>(parameters);

  // Only build a Name LSA if the added name is new
  if (m_namePrefixList.insert(castParams.getName())) {
    NLSR_LOG_INFO("Advertising name: " << castParams.getName() << "\n");
    m_lsdb.buildAndInstallOwnNameLsa();
    if (castParams.hasFlags() && castParams.getFlags() == PREFIX_FLAG) {
      NLSR_LOG_INFO("Saving name to the configuration file ");
      if (afterAdvertise(castParams.getName()) == true) {
        return done(ndn::nfd::ControlResponse(205, "OK").setBody(parameters.wireEncode()));
      }
      else {
        return done(ndn::nfd::ControlResponse(406, "Failed to open configuration file.")
                    .setBody(parameters.wireEncode()));
      }
    }
    return done(ndn::nfd::ControlResponse(200, "OK").setBody(parameters.wireEncode()));
  }
  else {
    if (castParams.hasFlags() && castParams.getFlags() == PREFIX_FLAG) {
      // Save an already advertised prefix
      NLSR_LOG_INFO("Saving an already advertised name: " << castParams.getName() << "\n");
      if (afterAdvertise(castParams.getName()) == true) {
        return done(ndn::nfd::ControlResponse(205, "OK").setBody(parameters.wireEncode()));
      }
      else {
        return done(ndn::nfd::ControlResponse(406, "Prefix is already Saved/Failed to open configuration file.")
                    .setBody(parameters.wireEncode()));
      }
    }
    return done(ndn::nfd::ControlResponse(204, "Prefix is already advertised/inserted.")
                .setBody(parameters.wireEncode()));
  }
}

void
CommandManagerBase::withdrawAndRemovePrefix(const ndn::Name& prefix,
                                            const ndn::Interest& interest,
                                            const ndn::mgmt::ControlParameters& parameters,
                                            const ndn::mgmt::CommandContinuation& done)
{
  const ndn::nfd::ControlParameters& castParams =
    static_cast<const ndn::nfd::ControlParameters&>(parameters);

  // Only build a Name LSA if the added name is new
  if (m_namePrefixList.remove(castParams.getName())) {
    NLSR_LOG_INFO("Withdrawing/Removing name: " << castParams.getName() << "\n");
    m_lsdb.buildAndInstallOwnNameLsa();
    if (castParams.hasFlags() && castParams.getFlags() == PREFIX_FLAG) {
      if (afterWithdraw(castParams.getName()) == true) {
        return done(ndn::nfd::ControlResponse(205, "OK").setBody(parameters.wireEncode()));
      }
      else {
        return done(ndn::nfd::ControlResponse(406, "Failed to open configuration file.")
                    .setBody(parameters.wireEncode()));
      }
    }
    return done(ndn::nfd::ControlResponse(200, "OK").setBody(parameters.wireEncode()));
  }
  else {
    if (castParams.hasFlags() && castParams.getFlags() == PREFIX_FLAG) {
      // Delete an already withdrawn prefix
      NLSR_LOG_INFO("Deleting an already withdrawn name: " << castParams.getName() << "\n");
      if (afterWithdraw(castParams.getName()) == true) {
        return done(ndn::nfd::ControlResponse(205, "OK").setBody(parameters.wireEncode()));
      }
      else {
        return done(ndn::nfd::ControlResponse(406, "Prefix is already deleted/Failed to open configuration file.")
                    .setBody(parameters.wireEncode()));
      }
    }
    return done(ndn::nfd::ControlResponse(204, "Prefix is already withdrawn/removed.")
                .setBody(parameters.wireEncode()));
  }
}

} // namespace update
} // namespace nlsr
