/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
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

#include "command-processor.hpp"
#include "logger.hpp"

#include <ndn-cxx/mgmt/nfd/control-response.hpp>

namespace nlsr::update {

INIT_LOGGER(update.CommandProcessor);

CommandProcessor::CommandProcessor(ndn::mgmt::Dispatcher& dispatcher,
                                   NamePrefixList& namePrefixList,
                                   Lsdb& lsdb)
  : m_dispatcher(dispatcher)
  , m_namePrefixList(namePrefixList)
  , m_lsdb(lsdb)
{
}

CommandProcessor::~CommandProcessor() = default;

void
CommandProcessor::advertiseAndInsertPrefix(const ndn::mgmt::ControlParametersBase& parameters,
                                           const ndn::mgmt::CommandContinuation& done)
{
  const auto& castParams = static_cast<const ndn::nfd::ControlParameters&>(parameters);
  // This is a bit of a hack, waiting on the work in #5348 to complete for full refactoring
  ndn::nfd::ControlParameters responseParams(castParams.wireEncode());
  uint64_t responseFaceId = (castParams.hasFaceId() && castParams.getFaceId() > 0)
                            ? responseParams.getFaceId() : m_defaultResponseFaceId;
  responseParams.setFaceId(responseFaceId);
  // Only build a Name LSA if the added name is new
  double castParamCost = (castParams.hasCost() ? castParams.getCost() : 0);
  if (m_namePrefixList.insert(castParams.getName(), "", castParamCost)) {
    NLSR_LOG_INFO("Advertising name: " << castParams.getName());
    m_lsdb.buildAndInstallOwnNameLsa();
    if (castParams.hasFlags() && castParams.getFlags() == PREFIX_FLAG) {
      NLSR_LOG_INFO("Saving name to the configuration file ");
      auto [afterAdvertiseReturn, afterAdvertiseMessage] = afterAdvertise(castParams.getName());
      if (afterAdvertiseReturn) {
        return done(ndn::nfd::ControlResponse(205, afterAdvertiseMessage).setBody(responseParams.wireEncode()));
      }
      else {
        return done(ndn::nfd::ControlResponse(500, afterAdvertiseMessage)
                    .setBody(responseParams.wireEncode()));
      }
    }
    return done(ndn::nfd::ControlResponse(200, "OK").setBody(responseParams.wireEncode()));
  }
  else {
    if (castParams.hasFlags() && castParams.getFlags() == PREFIX_FLAG) {
      // Save an already advertised prefix
      NLSR_LOG_INFO("Saving an already advertised name: " << castParams.getName());
      auto [afterAdvertiseReturn, afterAdvertiseMessage] = afterAdvertise(castParams.getName());
      if (afterAdvertiseReturn) {
        return done(ndn::nfd::ControlResponse(205, afterAdvertiseMessage).setBody(responseParams.wireEncode()));
      }
      else {
        return done(ndn::nfd::ControlResponse(500, afterAdvertiseMessage)
                    .setBody(responseParams.wireEncode()));
      }
    }
    return done(ndn::nfd::ControlResponse(204, "Prefix is already advertised/inserted.")
                .setBody(responseParams.wireEncode()));
  }
}

void
CommandProcessor::withdrawAndRemovePrefix(const ndn::mgmt::ControlParametersBase& parameters,
                                          const ndn::mgmt::CommandContinuation& done)
{
  const auto& castParams = static_cast<const ndn::nfd::ControlParameters&>(parameters);
  // This is a bit of a hack, waiting on the work in #5348 to complete for full refactoring
  ndn::nfd::ControlParameters responseParams(castParams.wireEncode());
  uint64_t responseFaceId = (castParams.hasFaceId() && castParams.getFaceId() > 0)
                            ? responseParams.getFaceId() : m_defaultResponseFaceId;
  responseParams.setFaceId(responseFaceId);
  // Only build a Name LSA if the added name is new
  if (m_namePrefixList.erase(castParams.getName())) {
    NLSR_LOG_INFO("Withdrawing/Removing name: " << castParams.getName());
    m_lsdb.buildAndInstallOwnNameLsa();
    if (castParams.hasFlags() && castParams.getFlags() == PREFIX_FLAG) {
      auto [afterWithdrawReturn, afterWithdrawMessage] = afterWithdraw(castParams.getName());
      if (afterWithdrawReturn) {
        return done(ndn::nfd::ControlResponse(205, afterWithdrawMessage).setBody(responseParams.wireEncode()));
      }
      else {
        return done(ndn::nfd::ControlResponse(500, afterWithdrawMessage)
                    .setBody(responseParams.wireEncode()));
      }
    }
    return done(ndn::nfd::ControlResponse(200, "OK").setBody(responseParams.wireEncode()));
  }
  else {
    if (castParams.hasFlags() && castParams.getFlags() == PREFIX_FLAG) {
      // Delete an already withdrawn prefix
      NLSR_LOG_INFO("Deleting an already withdrawn name: " << castParams.getName());
      auto [afterWithdrawReturn, afterWithdrawMessage] = afterWithdraw(castParams.getName());
      if (afterWithdrawReturn) {
        return done(ndn::nfd::ControlResponse(205, afterWithdrawMessage).setBody(responseParams.wireEncode()));
      }
      else {
        return done(ndn::nfd::ControlResponse(500, afterWithdrawMessage)
                    .setBody(responseParams.wireEncode()));
      }
    }
    return done(ndn::nfd::ControlResponse(204, "Prefix is already withdrawn/removed.")
                .setBody(responseParams.wireEncode()));
  }
}

} // namespace nlsr::update
