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

/*! \file
 * Define parameters for NFD RIB commands
 *
 * When validating an NFD RIB command, NLSR is only concerned with the
 * name in the command. However, a request is rejected if it has
 * unsolicited fields, so the origin, which is set by NFD's RIB, must
 * be considered optional. We consider these to be secure because they
 * are currently only received over the localhost prefix. These serve
 * to support NFD RIB to NLSR route readvertising.
 *
 * \sa NfdRibCommandProcessor
 * \sa nlsr::Nlsr::LOCALHOST_PREFIX
 * \sa nlsr::Nlsr::getDispatcher
 */

#ifndef UPDATE_NFD_RIB_COMMANDS_HPP
#define UPDATE_NFD_RIB_COMMANDS_HPP

#include <ndn-cxx/mgmt/nfd/control-command.hpp>

namespace nlsr {
namespace update {

class NfdRibRegisterCommand : public ndn::nfd::ControlCommand
{
public:
  NfdRibRegisterCommand();
};

class NfdRibUnregisterCommand : public ndn::nfd::ControlCommand
{
public:
  NfdRibUnregisterCommand();
};

} // namespace update
} // namespace nlsr

#endif // UPDATE_NFD_RIB_COMMANDS_HPP
