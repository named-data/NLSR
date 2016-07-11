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

#ifndef NLSR_SYNC_LOGIC_HANDLER_HPP
#define NLSR_SYNC_LOGIC_HANDLER_HPP

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-null.hpp>
#include <nsync/sync-socket.h>

#include <iostream>
#include <unistd.h>
#include <boost/cstdint.hpp>

#include "test-access-control.hpp"

class InterestManager;

namespace nlsr {

class ConfParameter;
class Lsdb;
class SequencingManager;
class SyncUpdate;

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

  SyncLogicHandler(ndn::Face& face, Lsdb& lsdb, ConfParameter& conf, SequencingManager& seqManager);

  /*! \brief Simple wrapper function to handle updates from Sync.

    \param v The information that Sync has acquired.
    \param socket The socket that Sync is using to synchronize updates.
   */
  void
  onNsyncUpdate(const std::vector<Sync::MissingDataInfo>& v, Sync::SyncSocket* socket);

  void
  onNsyncRemoval(const std::string& prefix);

  void
  publishRoutingUpdate();

  void
  createSyncSocket(const ndn::Name& syncPrefix);

private:
  void
  buildUpdatePrefix();

  void
  processUpdateFromSync(const SyncUpdate& updateName);

  bool
  isLsaNew(const ndn::Name& originRouter, const std::string& lsaType, uint64_t seqNo);

  void
  expressInterestForLsa(const SyncUpdate& updateName, std::string lsaType, uint64_t seqNo);

  void
  publishSyncUpdate(const ndn::Name& updatePrefix, uint64_t seqNo);

private:
  ndn::shared_ptr<ndn::ValidatorNull> m_validator;
  ndn::Face& m_syncFace;
  ndn::shared_ptr<Sync::SyncSocket> m_syncSocket;
  ndn::Name m_syncPrefix;

private:
  Lsdb& m_lsdb;
  ConfParameter& m_confParam;
  const SequencingManager& m_sequencingManager;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  ndn::Name m_updatePrefix;

private:
  static const std::string NLSR_COMPONENT;
  static const std::string LSA_COMPONENT;

};

} // namespace nlsr

#endif //NLSR_SYNC_LOGIC_HANDLER_HPP
