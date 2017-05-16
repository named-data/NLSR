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

#ifndef NLSR_SYNC_LOGIC_HANDLER_HPP
#define NLSR_SYNC_LOGIC_HANDLER_HPP

#include <ndn-cxx/face.hpp>
#include <ChronoSync/socket.hpp>

#include <iostream>
#include <unistd.h>
#include <boost/cstdint.hpp>
#include <boost/throw_exception.hpp>

#include "test-access-control.hpp"

class InterestManager;

namespace nlsr {

class ConfParameter;
class Lsdb;

class SyncLogicHandler
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

  SyncLogicHandler(ndn::Face& face, Lsdb& lsdb, ConfParameter& conf);

  /*! \brief Receive and parse update from Sync

    Parses the router name the update came from and passes it to processUpdateFromSync

    \param v The information that Sync has acquired.
   */
  void
  onNsyncUpdate(const std::vector<chronosync::MissingDataInfo>& v);

  /*! \brief Wrapper function to call publishSyncUpdate with correct LSA type

    \param type The LSA type constant
    \param seqNo The latest seqNo known to lsdb
   */
  void
  publishRoutingUpdate(const ndn::Name& type, const uint64_t& seqNo);

  /*! \brief Creates ChronoSync socket and register additional sync nodes (user prefixes)

    \param syncPrefix /localhop/NLSR/sync
   */
  void
  createSyncSocket(const ndn::Name& syncPrefix);

private:
  void
  buildUpdatePrefix();

  void
  processUpdateFromSync(const ndn::Name& originRouter,
                        const ndn::Name& updateName, const uint64_t& seqNo);

  bool
  isLsaNew(const ndn::Name& originRouter, const std::string& lsaType,
           const uint64_t& seqNo);

  void
  expressInterestForLsa(const ndn::Name& updateName, const uint64_t& seqNo);

  void
  publishSyncUpdate(const ndn::Name& updatePrefix, uint64_t seqNo);

private:
  ndn::Face& m_syncFace;
  std::shared_ptr<chronosync::Socket> m_syncSocket;
  ndn::Name m_syncPrefix;

private:
  Lsdb& m_lsdb;
  ConfParameter& m_confParam;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  ndn::Name m_nameLsaUserPrefix;
  ndn::Name m_adjLsaUserPrefix;
  ndn::Name m_coorLsaUserPrefix;

private:
  static const std::string NLSR_COMPONENT;
  static const std::string LSA_COMPONENT;

};

} // namespace nlsr

#endif // NLSR_SYNC_LOGIC_HANDLER_HPP
