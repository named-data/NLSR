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

#ifndef NLSR_UPDATE_COMMAND_PROCESSOR_HPP
#define NLSR_UPDATE_COMMAND_PROCESSOR_HPP

#include "lsdb.hpp"
#include "name-prefix-list.hpp"

#include <ndn-cxx/mgmt/dispatcher.hpp>
#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>

#include <boost/noncopyable.hpp>
#include <optional>

namespace nlsr::update {

enum { PREFIX_FLAG = 1 };

class CommandProcessor : boost::noncopyable
{
public:
  class Error : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  CommandProcessor(ndn::mgmt::Dispatcher& m_dispatcher,
                   NamePrefixList& m_namePrefixList,
                   Lsdb& lsdb);

  virtual
  ~CommandProcessor() = 0;

  /*! \brief Add desired name prefix to the advertised name prefix list
   *         or insert a prefix into the FIB if parameters is valid.
   */
  void
  advertiseAndInsertPrefix(const ndn::mgmt::ControlParametersBase& parameters,
                           const ndn::mgmt::CommandContinuation& done);

  /*! \brief Remove desired name prefix from the advertised name prefix list
   *         or remove a prefix from the FIB if parameters is valid.
   */
  void
  withdrawAndRemovePrefix(const ndn::mgmt::ControlParametersBase& parameters,
                          const ndn::mgmt::CommandContinuation& done);

  /*! \brief Processing after advertise command delegated to subclass.
   *         This is always treated as successful if not implemented.
   *  \return tuple {bool indicating success/failure, message string}.
   */
  virtual std::tuple<bool, std::string>
  afterAdvertise(const ndn::Name& prefix)
  {
    return {true, "OK"};
  }

  /*! \brief Processing after withdraw command delegated to subclass.
   *         This is always treated as successful if not implemented.
   *  \return tuple {bool indicating success/failure, message string}.
   */
  virtual std::tuple<bool, std::string>
  afterWithdraw(const ndn::Name& prefix)
  {
    return {true, "OK"};
  }

protected:
  ndn::mgmt::Dispatcher& m_dispatcher;
  NamePrefixList& m_namePrefixList;
  Lsdb& m_lsdb;

private:
  const uint64_t m_defaultResponseFaceId = 1;
};

} // namespace nlsr::update

#endif // NLSR_UPDATE_COMMAND_PROCESSOR_HPP
