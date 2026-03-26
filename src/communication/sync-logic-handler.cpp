/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2026,  The University of Memphis,
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

#include "sync-logic-handler.hpp"
#include "hello-protocol.hpp"
#include "logger.hpp"
#include "utility/name-helper.hpp"

namespace nlsr {

INIT_LOGGER(SyncLogicHandler);

const std::string LSA_COMPONENT{"LSA"};

SyncLogicHandler::SyncLogicHandler(ndn::Face& face, ndn::KeyChain& keyChain,
                                   const SyncLogicOptions& opts)
  : m_routerPrefix(opts.routerPrefix)
  , m_hyperbolicState(opts.hyperbolicState)
  , m_nameLsaUserPrefix(makeLsaUserPrefix(opts.userPrefix, Lsa::Type::NAME))
  , m_adjLsaUserPrefix(makeLsaUserPrefix(opts.userPrefix, Lsa::Type::ADJACENCY))
  , m_coorLsaUserPrefix(makeLsaUserPrefix(opts.userPrefix, Lsa::Type::COORDINATE))
  , m_syncLogic(face, keyChain, opts.syncProtocol, opts.syncPrefix,
                m_nameLsaUserPrefix, opts.syncInterestLifetime,
                std::bind(&SyncLogicHandler::processUpdate, this, _1, _2, _3))
{
  if (m_hyperbolicState != HYPERBOLIC_STATE_ON) {
    m_syncLogic.addUserNode(m_adjLsaUserPrefix);
  }

  if (m_hyperbolicState != HYPERBOLIC_STATE_OFF) {
    m_syncLogic.addUserNode(m_coorLsaUserPrefix);
  }
}

void
SyncLogicHandler::processUpdate(const ndn::Name& updateName, uint64_t highSeq, uint64_t incomingFaceId)
{
  NLSR_LOG_DEBUG("Update Name: " << updateName << " Seq no: " << highSeq);

  int32_t nlsrPosition = util::getNameComponentPosition(updateName, HelloProtocol::NLSR_COMPONENT);
  int32_t lsaPosition = util::getNameComponentPosition(updateName, LSA_COMPONENT);

  if (nlsrPosition < 0 || lsaPosition < 0) {
    NLSR_LOG_WARN("Received malformed sync update");
    return;
  }

  ndn::Name networkName = updateName.getSubName(1, nlsrPosition - 1);
  ndn::Name routerName = updateName.getSubName(lsaPosition + 1).getPrefix(-1);
  ndn::Name originRouter = networkName;
  originRouter.append(routerName);

  onSyncUpdate(updateName, highSeq, originRouter, incomingFaceId);
}

void
SyncLogicHandler::publishRoutingUpdate(Lsa::Type type, uint64_t seqNo)
{
  switch (type) {
  case Lsa::Type::ADJACENCY:
    m_syncLogic.publishUpdate(m_adjLsaUserPrefix, seqNo);
    break;
  case Lsa::Type::COORDINATE:
    m_syncLogic.publishUpdate(m_coorLsaUserPrefix, seqNo);
    break;
  case Lsa::Type::NAME:
    m_syncLogic.publishUpdate(m_nameLsaUserPrefix, seqNo);
    break;
  default:
    break;
  }
}

} // namespace nlsr
