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

#include "sync-logic-handler.hpp"

#include "common.hpp"
#include "conf-parameter.hpp"
#include "logger.hpp"
#include "lsa.hpp"
#include "lsdb.hpp"
#include "utility/name-helper.hpp"

namespace nlsr {

INIT_LOGGER("SyncLogicHandler");

using namespace ndn;
using namespace std;

const std::string NLSR_COMPONENT = "NLSR";
const std::string LSA_COMPONENT = "LSA";

template<class T>
class NullDeleter
{
public:
  void
  operator()(T*)
  {
  }
};

SyncLogicHandler::SyncLogicHandler(ndn::Face& face,
                                   Lsdb& lsdb, ConfParameter& conf)
  : m_syncFace(face)
  , m_lsdb(lsdb)
  , m_confParam(conf)
{
}

void
SyncLogicHandler::createSyncSocket(const ndn::Name& syncPrefix)
{
  if (m_syncSocket != nullptr) {
    _LOG_WARN("Trying to create Sync socket, but Sync socket already exists");
    return;
  }

  m_syncPrefix = syncPrefix;

  // Build LSA sync update prefix
  buildUpdatePrefix();

  _LOG_DEBUG("Creating Sync socket. Sync Prefix: " << m_syncPrefix);

  // The face's lifetime is managed in main.cpp; SyncSocket should not manage the memory
  // of the object
  std::shared_ptr<ndn::Face> facePtr(&m_syncFace, NullDeleter<ndn::Face>());


  m_syncSocket = std::make_shared<chronosync::Socket>(m_syncPrefix, m_nameLsaUserPrefix, *facePtr,
                                                      bind(&SyncLogicHandler::onChronoSyncUpdate, this, _1));

  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_OFF) {
    m_syncSocket->addSyncNode(m_adjLsaUserPrefix);
  }
  else {
    m_syncSocket->addSyncNode(m_coorLsaUserPrefix);
  }
}

void
SyncLogicHandler::onChronoSyncUpdate(const vector<chronosync::MissingDataInfo>& v)
{
  _LOG_DEBUG("Received ChronoSync update event");

  for (size_t i = 0; i < v.size(); i++){
    ndn::Name updateName = v[i].session.getPrefix(-1);

    _LOG_DEBUG("Update Name: " << updateName << " Seq no: " << v[i].high);

    int32_t nlsrPosition = util::getNameComponentPosition(updateName, nlsr::NLSR_COMPONENT);
    int32_t lsaPosition = util::getNameComponentPosition(updateName, nlsr::LSA_COMPONENT);

    if (nlsrPosition < 0 || lsaPosition < 0) {
      _LOG_WARN("Received malformed sync update");
      return;
    }

    ndn::Name networkName = updateName.getSubName(1, nlsrPosition-1);
    ndn::Name routerName = updateName.getSubName(lsaPosition + 1).getPrefix(-1);

    ndn::Name originRouter = networkName;
    originRouter.append(routerName);

    processUpdateFromSync(originRouter, updateName, v[i].high);
  }
}

void
SyncLogicHandler::processUpdateFromSync(const ndn::Name& originRouter,
                                        const ndn::Name& updateName, const uint64_t& seqNo)
{
  _LOG_DEBUG("Origin Router of update: " << originRouter);

  // A router should not try to fetch its own LSA
  if (originRouter != m_confParam.getRouterPrefix()) {

    std::string lsaType = updateName.get(updateName.size()-1).toUri();

    _LOG_DEBUG("Received sync update with higher " << lsaType
               << " sequence number than entry in LSDB");

    if (isLsaNew(originRouter, lsaType, seqNo)) {
      if (lsaType == AdjLsa::TYPE_STRING && seqNo != 0 &&
          m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
        _LOG_ERROR("Got an update for adjacency LSA when hyperbolic routing"
                   << " is enabled. Not going to fetch.");
        return;
      }

      if (lsaType == CoordinateLsa::TYPE_STRING && seqNo != 0 &&
          m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_OFF) {
        _LOG_ERROR("Got an update for coordinate LSA when link-state"
                   << " is enabled. Not going to fetch.");
        return;
      }
      expressInterestForLsa(updateName, seqNo);
    }
  }
}

bool
SyncLogicHandler::isLsaNew(const ndn::Name& originRouter, const std::string& lsaType,
                           const uint64_t& seqNo)
{
  ndn::Name lsaKey = originRouter;
  lsaKey.append(lsaType);

  if (lsaType == NameLsa::TYPE_STRING)
  {
    return m_lsdb.isNameLsaNew(lsaKey, seqNo);
  }
  else if (lsaType == AdjLsa::TYPE_STRING)
  {
    return m_lsdb.isAdjLsaNew(lsaKey, seqNo);
  }
  else if (lsaType == CoordinateLsa::TYPE_STRING)
  {
    return m_lsdb.isCoordinateLsaNew(lsaKey, seqNo);
  }

  return false;
}

void
SyncLogicHandler::expressInterestForLsa(const ndn::Name& updateName,
                                        const uint64_t& seqNo)
{
  ndn::Name interest(updateName);
  interest.appendNumber(seqNo);

  m_lsdb.expressInterest(interest, 0);
}

void
SyncLogicHandler::publishRoutingUpdate(const ndn::Name& type, const uint64_t& seqNo)
{
  if (m_syncSocket == nullptr) {
    _LOG_FATAL("Cannot publish routing update; SyncSocket does not exist");

    BOOST_THROW_EXCEPTION(SyncLogicHandler::Error("Cannot publish routing update; SyncSocket does not exist"));
  }

  if (type == NameLsa::TYPE_STRING) {
    publishSyncUpdate(m_nameLsaUserPrefix, seqNo);
  }
  else if (type == AdjLsa::TYPE_STRING) {
    publishSyncUpdate(m_adjLsaUserPrefix, seqNo);
  }
  else {
    publishSyncUpdate(m_coorLsaUserPrefix, seqNo);
  }
}

void
SyncLogicHandler::buildUpdatePrefix()
{
  ndn::Name updatePrefix = m_confParam.getLsaPrefix();
  updatePrefix.append(m_confParam.getSiteName());
  updatePrefix.append(m_confParam.getRouterName());

  m_nameLsaUserPrefix = updatePrefix;
  m_nameLsaUserPrefix.append(NameLsa::TYPE_STRING);

  m_adjLsaUserPrefix = updatePrefix;
  m_adjLsaUserPrefix.append(AdjLsa::TYPE_STRING);

  m_coorLsaUserPrefix = updatePrefix;
  m_coorLsaUserPrefix.append(CoordinateLsa::TYPE_STRING);
}

void
SyncLogicHandler::publishSyncUpdate(const ndn::Name& updatePrefix, uint64_t seqNo)
{
  _LOG_DEBUG("Publishing Sync Update. Prefix: " << updatePrefix << " Seq No: " << seqNo);

  ndn::Name updateName(updatePrefix);
  string data("NoData");

  m_syncSocket->publishData(reinterpret_cast<const uint8_t*>(data.c_str()), data.size(),
                            ndn::time::milliseconds(1000), seqNo, updateName);
}

} // namespace nlsr
