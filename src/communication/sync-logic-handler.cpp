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
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#include "nlsr.hpp"
#include "sync-logic-handler.hpp"
#include "utility/name-helper.hpp"
#include "lsa.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("SyncLogicHandler");

using namespace ndn;
using namespace std;

void
SyncLogicHandler::createSyncSocket(Nlsr& pnlsr)
{
  _LOG_DEBUG("Creating Sync socket. Sync Prefix: " << m_syncPrefix);
  m_syncSocket = ndn::make_shared<Sync::SyncSocket>(m_syncPrefix, m_validator,
                                                    m_syncFace,
                                                    ndn::bind(&SyncLogicHandler::nsyncUpdateCallBack,
                                                              this, _1, _2, ndn::ref(pnlsr)),
                                                    ndn::bind(&SyncLogicHandler::nsyncRemoveCallBack,
                                                              this, _1, ndn::ref(pnlsr)));
}

void
SyncLogicHandler::nsyncUpdateCallBack(const vector<Sync::MissingDataInfo>& v,
                                      Sync::SyncSocket* socket, Nlsr& pnlsr)
{
  _LOG_DEBUG("nsyncUpdateCallBack called ....");
  int32_t n = v.size();
  for (int32_t i = 0; i < n; i++){
    _LOG_DEBUG("Update Name: " << v[i].prefix << " Seq: " << v[i].high.getSeq());
    processUpdateFromSync(v[i].prefix, v[i].high.getSeq(), pnlsr);
  }
}

void
SyncLogicHandler::nsyncRemoveCallBack(const string& prefix, Nlsr& pnlsr)
{
  _LOG_DEBUG("nsyncRemoveCallBack called ....");
}

void
SyncLogicHandler::removeRouterFromSyncing(const ndn::Name& routerPrefix)
{
}

void
SyncLogicHandler::processUpdateFromSync(const ndn::Name& updateName,
                                        uint64_t seqNo,  Nlsr& pnlsr)
{
  string chkString("LSA");
  int32_t lasPosition = util::getNameComponentPosition(updateName, chkString);
  if (lasPosition >= 0) {
    ndn::Name routerName = updateName.getSubName(lasPosition + 1);
    processRoutingUpdateFromSync(routerName, seqNo, pnlsr);
    return;
  }
}

void
SyncLogicHandler::processRoutingUpdateFromSync(const ndn::Name& routerName,
                                               uint64_t seqNo,  Nlsr& pnlsr)
{
  ndn::Name rName = routerName;
  if (routerName != pnlsr.getConfParameter().getRouterPrefix()) {
    SequencingManager sm(seqNo);
    sm.writeLog();
    _LOG_DEBUG(routerName);
    try {
      if (pnlsr.getLsdb().isNameLsaNew(rName.append("name"), sm.getNameLsaSeq())) {
        _LOG_DEBUG("Updated Name LSA. Need to fetch it");
        ndn::Name interestName(pnlsr.getConfParameter().getLsaPrefix());
        interestName.append(routerName);
        interestName.append("name");
        interestName.appendNumber(sm.getNameLsaSeq());
        pnlsr.getLsdb().expressInterest(interestName,
                                        pnlsr.getConfParameter().getInterestResendTime(),
                                        0);
      }
      if (pnlsr.getLsdb().isAdjLsaNew(rName.append("adjacency"), sm.getAdjLsaSeq())) {
        _LOG_DEBUG("Updated Adj LSA. Need to fetch it");
        ndn::Name interestName(pnlsr.getConfParameter().getLsaPrefix());
        interestName.append(routerName);
        interestName.append("adjacency");
        interestName.appendNumber(sm.getAdjLsaSeq());
        pnlsr.getLsdb().expressInterest(interestName,
                                        pnlsr.getConfParameter().getInterestResendTime(),
                                        0);
      }
      if (pnlsr.getLsdb().isCoordinateLsaNew(rName.append("coordinate"),
                                             sm.getCorLsaSeq())) {
        _LOG_DEBUG("Updated Cor LSA. Need to fetch it");
        ndn::Name interestName(pnlsr.getConfParameter().getLsaPrefix());
        interestName.append(routerName);
        interestName.append("coordinate");
        interestName.appendNumber(sm.getCorLsaSeq());
        pnlsr.getLsdb().expressInterest(interestName,
                                        pnlsr.getConfParameter().getInterestResendTime(),
                                        0);
      }
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      return;
    }
  }
}

void
SyncLogicHandler::publishRoutingUpdate(SequencingManager& sm,
                                       const ndn::Name& updatePrefix)
{
  sm.writeSeqNoToFile();
  publishSyncUpdate(updatePrefix, sm.getCombinedSeqNo());
}

void
SyncLogicHandler::publishSyncUpdate(const ndn::Name& updatePrefix,
                                    uint64_t seqNo)
{
  _LOG_DEBUG("Publishing Sync Update. Prefix: " << updatePrefix << "Seq no: " << seqNo);
  ndn::Name updateName(updatePrefix);
  string data("NoData");
  m_syncSocket->publishData(updateName.toUri(), 0, data.c_str(), data.size(),
                            1000,
                            seqNo);
}

}//namespace nlsr
