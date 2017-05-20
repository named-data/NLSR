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

#ifndef UPDATE_NFD_RIB_COMMAND_PROCESSOR_HPP
#define UPDATE_NFD_RIB_COMMAND_PROCESSOR_HPP

#include "route/fib.hpp"
#include "nfd-rib-commands.hpp"
#include "name-prefix-list.hpp"
#include "lsdb.hpp"
#include "communication/sync-logic-handler.hpp"

#include <ndn-cxx/mgmt/nfd/control-command.hpp>
#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/mgmt/dispatcher.hpp>
#include <ndn-cxx/encoding/tlv-nfd.hpp>

#include <boost/noncopyable.hpp>
#include <typeinfo>

namespace nlsr {
namespace update {

class NfdRibCommandProcessor : boost::noncopyable
{
public:
  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

public:
  NfdRibCommandProcessor(ndn::mgmt::Dispatcher& dispatcher,
                         NamePrefixList& namePrefixes,
                         Lsdb& lsdb,
                         SyncLogicHandler& sync);

  /*! \brief Registers an Interest filter with face

    Registers with face an Interest filter that reacts to any
    interests on COMMAND_PREFIX.
   */
  void
  startListening();

PUBLIC_WITH_TESTS_ELSE_PRIVATE:

  /*! \brief inserts a prefix into the FIB if parameters is valid.

    We consider parameters to be valid if origin is set to CLIENT and
    a name is present.
   */
  void
  insertPrefix(const ndn::mgmt::ControlParameters& parameters);

  /*! \brief remove a prefix from the FIB if parameters is valid.

    We consider parameters to be valid if origin is set to CLIENT and
    a name is present.
   */
  void
  removePrefix(const ndn::mgmt::ControlParameters& parameters);

  /*! \brief uses command's validator to check that parameters is valid.

    The command's validator's behavior is defined per command.
   */
  template<typename T>
  bool
  validateParameters(const ndn::mgmt::ControlParameters& parameters)
  {
    BOOST_ASSERT(dynamic_cast<const ndn::nfd::ControlParameters*>(&parameters) != nullptr);
    const ndn::nfd::ControlParameters& castParams =
      static_cast<const ndn::nfd::ControlParameters&>(parameters);
    T command;
    try {
      command.validateRequest(castParams);
    }
    catch (const ndn::nfd::ControlCommand::ArgumentError& ae) {
      throw ae;
    }
    catch (...) {
      return false;
    }
    return true;
  }

private:
  template<typename T>
  bool
  validate(const ndn::mgmt::ControlParameters& parameters, const ndn::PartialName& command);

  ndn::mgmt::Dispatcher& m_dispatcher;
  NamePrefixList& m_namePrefixList;
  Lsdb& m_lsdb;
  SyncLogicHandler& m_sync;
};

} // namespace update
} // namespace nlsr

#endif // UPDATE_NFD_RIB_COMMAND_PROCESSOR_HPP
