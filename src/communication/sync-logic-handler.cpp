/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2019,  The University of Memphis,
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

#include "sync-logic-handler.hpp"
#include "common.hpp"
#include "conf-parameter.hpp"
#include "lsa.hpp"
#include "logger.hpp"
#include "utility/name-helper.hpp"

namespace nlsr {

const std::string NLSR_COMPONENT = "nlsr";
const std::string LSA_COMPONENT = "LSA";

INIT_LOGGER(SyncLogicHandler);

template<class T>
class NullDeleter
{
public:
  void
  operator()(T*)
  {
  }
};

SyncLogicHandler::SyncLogicHandler(ndn::Face& face, const IsLsaNew& isLsaNew,
                                   const ConfParameter& conf)
  : onNewLsa(std::make_unique<OnNewLsa>())
  , m_syncFace(face)
  , m_isLsaNew(isLsaNew)
  , m_confParam(conf)
{
  createSyncLogic(conf.getSyncPrefix());
}

void
SyncLogicHandler::createSyncLogic(const ndn::Name& syncPrefix, const ndn::time::milliseconds& syncInterestLifetime)
{
  if (m_syncLogic != nullptr) {
    NLSR_LOG_WARN("Trying to create Sync Logic object, but Sync Logic object already exists");
    return;
  }

  // Build LSA sync update prefix
  buildUpdatePrefix();

  NLSR_LOG_DEBUG("Creating Sync Logic object. Sync Prefix: " << syncPrefix);

  // The face's lifetime is managed in main.cpp; Logic should not manage the memory
  // of the object
  std::shared_ptr<ndn::Face> facePtr(&m_syncFace, NullDeleter<ndn::Face>());

  m_syncLogic = std::make_shared<SyncProtocolAdapter>(*facePtr,
                  m_confParam.getSyncProtocol(),
                  syncPrefix,
                  m_nameLsaUserPrefix,
                  syncInterestLifetime,
                  std::bind(&SyncLogicHandler::processUpdate, this, _1, _2));

  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_OFF) {
    m_syncLogic->addUserNode(m_adjLsaUserPrefix);
  }
  else if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
    m_syncLogic->addUserNode(m_coorLsaUserPrefix);
  }
  else {
    m_syncLogic->addUserNode(m_adjLsaUserPrefix);
    m_syncLogic->addUserNode(m_coorLsaUserPrefix);
  }
}

void
SyncLogicHandler::processUpdate(const ndn::Name& updateName, uint64_t highSeq)
{
  NLSR_LOG_DEBUG("Update Name: " << updateName << " Seq no: " << highSeq);

  int32_t nlsrPosition = util::getNameComponentPosition(updateName, nlsr::NLSR_COMPONENT);
  int32_t lsaPosition = util::getNameComponentPosition(updateName, nlsr::LSA_COMPONENT);

  if (nlsrPosition < 0 || lsaPosition < 0) {
    NLSR_LOG_WARN("Received malformed sync update");
    return;
  }

  ndn::Name networkName = updateName.getSubName(1, nlsrPosition-1);
  ndn::Name routerName = updateName.getSubName(lsaPosition + 1).getPrefix(-1);

  ndn::Name originRouter = networkName;
  originRouter.append(routerName);

  processUpdateFromSync(originRouter, updateName, highSeq);
}

void
SyncLogicHandler::processUpdateFromSync(const ndn::Name& originRouter,
                                        const ndn::Name& updateName, uint64_t seqNo)
{
  NLSR_LOG_DEBUG("Origin Router of update: " << originRouter);

  // A router should not try to fetch its own LSA
  if (originRouter != m_confParam.getRouterPrefix()) {

    Lsa::Type lsaType;
    std::istringstream(updateName.get(updateName.size()-1).toUri()) >> lsaType;

    NLSR_LOG_DEBUG("Received sync update with higher " << lsaType <<
                   " sequence number than entry in LSDB");

    if (m_isLsaNew(originRouter, lsaType, seqNo)) {
      if (lsaType == Lsa::Type::ADJACENCY && seqNo != 0 &&
          m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
        NLSR_LOG_ERROR("Got an update for adjacency LSA when hyperbolic routing " <<
                       "is enabled. Not going to fetch.");
        return;
      }

      if (lsaType == Lsa::Type::COORDINATE && seqNo != 0 &&
          m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_OFF) {
        NLSR_LOG_ERROR("Got an update for coordinate LSA when link-state " <<
                       "is enabled. Not going to fetch.");
        return;
      }
      (*onNewLsa)(updateName, seqNo, originRouter);
    }
  }
}

void
SyncLogicHandler::publishRoutingUpdate(const Lsa::Type& type, const uint64_t& seqNo)
{
  if (m_syncLogic == nullptr) {
    NLSR_LOG_FATAL("Cannot publish routing update; SyncLogic does not exist");

    BOOST_THROW_EXCEPTION(SyncLogicHandler::Error("Cannot publish routing update; SyncLogic does not exist"));
  }

  switch (type) {
  case Lsa::Type::ADJACENCY:
    m_syncLogic->publishUpdate(m_adjLsaUserPrefix, seqNo);
    break;
  case Lsa::Type::COORDINATE:
    m_syncLogic->publishUpdate(m_coorLsaUserPrefix, seqNo);
    break;
  case Lsa::Type::NAME:
    m_syncLogic->publishUpdate(m_nameLsaUserPrefix, seqNo);
    break;
  default:
    break;
  }
}

void
SyncLogicHandler::buildUpdatePrefix()
{
  ndn::Name updatePrefix = m_confParam.getLsaPrefix();
  updatePrefix.append(m_confParam.getSiteName());
  updatePrefix.append(m_confParam.getRouterName());

  m_nameLsaUserPrefix = updatePrefix;
  m_nameLsaUserPrefix.append(std::to_string(Lsa::Type::NAME));

  m_adjLsaUserPrefix = updatePrefix;
  m_adjLsaUserPrefix.append(std::to_string(Lsa::Type::ADJACENCY));

  m_coorLsaUserPrefix = updatePrefix;
  m_coorLsaUserPrefix.append(std::to_string(Lsa::Type::COORDINATE));
}

} // namespace nlsr
