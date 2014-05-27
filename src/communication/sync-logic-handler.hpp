/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#ifndef NLSR_SYNC_LOGIC_HANDLER_HPP
#define NLSR_SYNC_LOGIC_HANDLER_HPP

#include <iostream>
#include <boost/cstdint.hpp>

#include <ndn-cxx/face.hpp>
#include <nsync/sync-socket.h>
#include <ndn-cxx/security/validator-null.hpp>

#include "sequencing-manager.hpp"

extern "C" {
#include <unistd.h>
}

class InterestManager;
class ConfParameter;

namespace nlsr {

class SyncLogicHandler
{
public:
  SyncLogicHandler(boost::asio::io_service& ioService)
    : m_validator(new ndn::ValidatorNull())
    , m_syncFace(new ndn::Face(ioService))
  {
  }


  void
  createSyncSocket(Nlsr& pnlsr);

  void
  nsyncUpdateCallBack(const std::vector<Sync::MissingDataInfo>& v,
                      Sync::SyncSocket* socket, Nlsr& pnlsr);

  void
  nsyncRemoveCallBack(const std::string& prefix, Nlsr& pnlsr);

  void
  removeRouterFromSyncing(const ndn::Name& routerPrefix);

  void
  publishRoutingUpdate(SequencingManager& sm, const ndn::Name& updatePrefix);

  void
  setSyncPrefix(const std::string& sp)
  {
    m_syncPrefix.clear();
    m_syncPrefix.set(sp);
  }

private:
  void
  processUpdateFromSync(const ndn::Name& updateName, uint64_t seqNo,
                        Nlsr& pnlsr);

  void
  processRoutingUpdateFromSync(const ndn::Name& routerName, uint64_t seqNo,
                               Nlsr& pnlsr);

  void
  publishSyncUpdate(const ndn::Name& updatePrefix, uint64_t seqNo);

private:
  ndn::shared_ptr<ndn::ValidatorNull> m_validator;
  ndn::shared_ptr<ndn::Face> m_syncFace;
  ndn::shared_ptr<Sync::SyncSocket> m_syncSocket;
  ndn::Name m_syncPrefix;
};

} //namespace nlsr

#endif //NLSR_SYNC_LOGIC_HANDLER_HPP
