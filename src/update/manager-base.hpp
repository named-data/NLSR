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

#ifndef NLSR_MANAGER_BASE_HPP
#define NLSR_MANAGER_BASE_HPP

#include "name-prefix-list.hpp"
#include "test-access-control.hpp"
#include "lsdb.hpp"
#include "nfd-rib-commands.hpp"
#include "prefix-update-commands.hpp"
#include "logger.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/mgmt/nfd/control-command.hpp>
#include <ndn-cxx/mgmt/dispatcher.hpp>
#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/mgmt/nfd/control-response.hpp>

#include <boost/noncopyable.hpp>

namespace nlsr {

class Lsdb;

namespace update {

class ManagerBase : boost::noncopyable
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
  ManagerBase(ndn::mgmt::Dispatcher& m_dispatcher,
              const std::string& module);

protected:
  /*! \brief generate the relative prefix for a handler by appending the verb name to the module name
   */
  ndn::PartialName
  makeRelPrefix(const std::string& verb) const;

PUBLIC_WITH_TESTS_ELSE_PROTECTED:
  /*! \brief validate the parameters for a given command
    */
  template<typename T>
  bool
  validateParameters(const ndn::mgmt::ControlParameters& parameters)
  {
    const ndn::nfd::ControlParameters* castParams =
    dynamic_cast<const ndn::nfd::ControlParameters*>(&parameters);

    BOOST_ASSERT(castParams != nullptr);
    T command;
    try {
      command.validateRequest(*castParams);
    }
    catch (const ndn::nfd::ControlCommand::ArgumentError& ae) {
      throw ae;
    }
    catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
    return true;
  }

protected:
  ndn::mgmt::Dispatcher& m_dispatcher;

private:
  std::string m_module;
};

class CommandManagerBase: public ManagerBase
{
public:
  CommandManagerBase(ndn::mgmt::Dispatcher& m_dispatcher,
                     NamePrefixList& m_namePrefixList,
                     Lsdb& lsdb,
                     const std::string& module);

  /*! \brief add desired name prefix to the advertised name prefix list
   *         or insert a prefix into the FIB if parameters is valid.
   */
  void
  advertiseAndInsertPrefix(const ndn::Name& prefix,
                           const ndn::Interest& interest,
                           const ndn::mgmt::ControlParameters& parameters,
                           const ndn::mgmt::CommandContinuation& done);

  /*! \brief remove desired name prefix from the advertised name prefix list
   *         or remove a prefix from the FIB if parameters is valid.
   */
  void
  withdrawAndRemovePrefix(const ndn::Name& prefix,
                          const ndn::Interest& interest,
                          const ndn::mgmt::ControlParameters& parameters,
                          const ndn::mgmt::CommandContinuation& done);

protected:
  NamePrefixList& m_namePrefixList;
  Lsdb& m_lsdb;
};

} // namespace update
} // namespace nlsr

#endif // NLSR_MANAGER_BASE_HPP
