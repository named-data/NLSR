/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
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
#include "sequencing-manager.hpp"
#include "utility/name-helper.hpp"

namespace nlsr {

INIT_LOGGER("SyncLogicHandler");

using namespace ndn;
using namespace std;

class SyncUpdate
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
  SyncUpdate(const ndn::Name& name, uint64_t seqNo)
    : m_name(name)
    , m_seqManager(seqNo)
  {
  }

  const ndn::Name&
  getName() const
  {
    return m_name;
  }

  const ndn::Name
  getOriginRouter() const
  {
    int32_t nlsrPosition = util::getNameComponentPosition(m_name, NLSR_COMPONENT);
    int32_t lsaPosition = util::getNameComponentPosition(m_name, LSA_COMPONENT);

    if (nlsrPosition < 0 || lsaPosition < 0) {
      throw Error("Cannot parse update name because expected components are missing");
    }

    ndn::Name networkName = m_name.getSubName(0, nlsrPosition);
    ndn::Name routerName = m_name.getSubName(lsaPosition + 1);

    ndn::Name originRouter = networkName;
    originRouter.append(routerName);

    return originRouter;
  }

  uint64_t
  getNameLsaSeqNo() const
  {
    return m_seqManager.getNameLsaSeq();
  }

  uint64_t
  getAdjLsaSeqNo() const
  {
    return m_seqManager.getAdjLsaSeq();
  }

  uint64_t
  getCorLsaSeqNo() const
  {
    return m_seqManager.getCorLsaSeq();
  }

  const SequencingManager&
  getSequencingManager() const
  {
    return m_seqManager;
  }

private:
  const ndn::Name m_name;
  SequencingManager m_seqManager;

  static const std::string NLSR_COMPONENT;
  static const std::string LSA_COMPONENT;
};

const std::string SyncUpdate::NLSR_COMPONENT = "NLSR";
const std::string SyncUpdate::LSA_COMPONENT = "LSA";

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
                                   Lsdb& lsdb, ConfParameter& conf,
                                   SequencingManager& seqManager)
  : m_validator(new ndn::ValidatorNull())
  , m_syncFace(face)
  , m_lsdb(lsdb)
  , m_confParam(conf)
  , m_sequencingManager(seqManager)
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
  ndn::shared_ptr<ndn::Face> facePtr(&m_syncFace, NullDeleter<ndn::Face>());

  m_syncSocket = ndn::make_shared<Sync::SyncSocket>(m_syncPrefix, m_validator, facePtr,
                                                    ndn::bind(&SyncLogicHandler::onNsyncUpdate,
                                                              this, _1, _2),
                                                    ndn::bind(&SyncLogicHandler::onNsyncRemoval,
                                                              this, _1));
}

void
SyncLogicHandler::onNsyncUpdate(const vector<Sync::MissingDataInfo>& v, Sync::SyncSocket* socket)
{
  _LOG_DEBUG("Received Nsync update event");

  for (size_t i = 0; i < v.size(); i++){
    _LOG_DEBUG("Update Name: " << v[i].prefix << " Seq no: " << v[i].high.getSeq());

    SyncUpdate update(v[i].prefix, v[i].high.getSeq());

    processUpdateFromSync(update);
  }
}

void
SyncLogicHandler::onNsyncRemoval(const string& prefix)
{
  _LOG_DEBUG("Received Nsync removal event");
}

void
SyncLogicHandler::processUpdateFromSync(const SyncUpdate& update)
{
  ndn::Name originRouter;

  try {
    originRouter = update.getOriginRouter();
  }
  catch (std::exception& e) {
    _LOG_WARN("Received malformed sync update");
    return;
  }

  // A router should not try to fetch its own LSA
  if (originRouter != m_confParam.getRouterPrefix()) {

    update.getSequencingManager().writeLog();

    try {
      if (isLsaNew(originRouter, NameLsa::TYPE_STRING, update.getNameLsaSeqNo())) {
        _LOG_DEBUG("Received sync update with higher Name LSA sequence number than entry in LSDB");

        expressInterestForLsa(update, NameLsa::TYPE_STRING, update.getNameLsaSeqNo());
      }

      if (isLsaNew(originRouter, AdjLsa::TYPE_STRING, update.getAdjLsaSeqNo())) {
        _LOG_DEBUG("Received sync update with higher Adj LSA sequence number than entry in LSDB");

        expressInterestForLsa(update, AdjLsa::TYPE_STRING, update.getAdjLsaSeqNo());
      }

      if (isLsaNew(originRouter, CoordinateLsa::TYPE_STRING, update.getCorLsaSeqNo())) {
        _LOG_DEBUG("Received sync update with higher Cor LSA sequence number than entry in LSDB");

        expressInterestForLsa(update, CoordinateLsa::TYPE_STRING, update.getCorLsaSeqNo());
      }
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      return;
    }
  }
}

bool
SyncLogicHandler::isLsaNew(const ndn::Name& originRouter, const std::string& lsaType,
                           uint64_t seqNo)
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
SyncLogicHandler::expressInterestForLsa(const SyncUpdate& update, std::string lsaType,
                                        uint64_t seqNo)
{
  ndn::Name interest(update.getName());
  interest.append(lsaType);
  interest.appendNumber(seqNo);

  m_lsdb.expressInterest(interest, 0);
}

void
SyncLogicHandler::publishRoutingUpdate()
{
  if (m_syncSocket == nullptr) {
    _LOG_FATAL("Cannot publish routing update; SyncSocket does not exist");

    throw SyncLogicHandler::Error("Cannot publish routing update; SyncSocket does not exist");
  }

  m_sequencingManager.writeSeqNoToFile();

  publishSyncUpdate(m_updatePrefix, m_sequencingManager.getCombinedSeqNo());
}

void
SyncLogicHandler::buildUpdatePrefix()
{
  m_updatePrefix = m_confParam.getLsaPrefix();
  m_updatePrefix.append(m_confParam.getSiteName());
  m_updatePrefix.append(m_confParam.getRouterName());
}

void
SyncLogicHandler::publishSyncUpdate(const ndn::Name& updatePrefix, uint64_t seqNo)
{
  _LOG_DEBUG("Publishing Sync Update. Prefix: " << updatePrefix << " Seq No: " << seqNo);

  ndn::Name updateName(updatePrefix);
  string data("NoData");

  m_syncSocket->publishData(updateName.toUri(), 0, data.c_str(), data.size(), 1000, seqNo);
}

}//namespace nlsr
