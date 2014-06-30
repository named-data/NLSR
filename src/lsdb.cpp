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
#include <string>
#include <utility>

#include "lsdb.hpp"
#include "nlsr.hpp"
#include "conf-parameter.hpp"
#include "utility/name-helper.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("Lsdb");

using namespace std;

void
Lsdb::cancelScheduleLsaExpiringEvent(ndn::EventId eid)
{
  m_nlsr.getScheduler().cancelEvent(eid);
}

static bool
nameLsaCompareByKey(const NameLsa& nlsa1, const ndn::Name& key)
{
  return nlsa1.getKey() == key;
}


bool
Lsdb::buildAndInstallOwnNameLsa()
{
  NameLsa nameLsa(m_nlsr.getConfParameter().getRouterPrefix(),
                  "name",
                  m_nlsr.getSequencingManager().getNameLsaSeq() + 1,
                  getLsaExpirationTimePoint(),
                  m_nlsr.getNamePrefixList());
  m_nlsr.getSequencingManager().increaseNameLsaSeq();
  return installNameLsa(nameLsa);
}

NameLsa*
Lsdb::findNameLsa(const ndn::Name& key)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 bind(nameLsaCompareByKey, _1, key));
  if (it != m_nameLsdb.end()) {
    return &(*it);
  }
  return 0;
}

bool
Lsdb::isNameLsaNew(const ndn::Name& key, uint64_t seqNo)
{
  NameLsa* nameLsaCheck = findNameLsa(key);
  if (nameLsaCheck != 0) {
    if (nameLsaCheck->getLsSeqNo() < seqNo) {
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

ndn::EventId
Lsdb::scheduleNameLsaExpiration(const ndn::Name& key, int seqNo,
                                const ndn::time::seconds& expTime)
{
  return m_nlsr.getScheduler().scheduleEvent(expTime,
                                             ndn::bind(&Lsdb::exprireOrRefreshNameLsa,
                                                       this, key, seqNo));
}

bool
Lsdb::installNameLsa(NameLsa& nlsa)
{
  ndn::time::seconds timeToExpire = ndn::time::seconds(m_lsaRefreshTime);
  NameLsa* chkNameLsa = findNameLsa(nlsa.getKey());
  if (chkNameLsa == 0) {
    addNameLsa(nlsa);
    _LOG_DEBUG("New Name LSA");
    _LOG_DEBUG("Adding Name Lsa");
    nlsa.writeLog();

    if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      m_nlsr.getNamePrefixTable().addEntry(nlsa.getOrigRouter(),
                                           nlsa.getOrigRouter());
      std::list<ndn::Name> nameList = nlsa.getNpl().getNameList();
      for (std::list<ndn::Name>::iterator it = nameList.begin(); it != nameList.end();
           it++) {
        if ((*it) != m_nlsr.getConfParameter().getRouterPrefix()) {
          m_nlsr.getNamePrefixTable().addEntry((*it), nlsa.getOrigRouter());
        }
      }
    }
    if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      ndn::time::system_clock::Duration duration = nlsa.getExpirationTimePoint() -
                                                   ndn::time::system_clock::now();
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }
    nlsa.setExpiringEventId(scheduleNameLsaExpiration(nlsa.getKey(),
                                                      nlsa.getLsSeqNo(),
                                                      timeToExpire));
  }
  else {
    if (chkNameLsa->getLsSeqNo() < nlsa.getLsSeqNo()) {
      _LOG_DEBUG("Updated Name LSA. Updating LSDB");
      _LOG_DEBUG("Deleting Name Lsa");
      chkNameLsa->writeLog();
      chkNameLsa->setLsSeqNo(nlsa.getLsSeqNo());
      chkNameLsa->setExpirationTimePoint(nlsa.getExpirationTimePoint());
      chkNameLsa->getNpl().sort();
      nlsa.getNpl().sort();
      std::list<ndn::Name> nameToAdd;
      std::set_difference(nlsa.getNpl().getNameList().begin(),
                          nlsa.getNpl().getNameList().end(),
                          chkNameLsa->getNpl().getNameList().begin(),
                          chkNameLsa->getNpl().getNameList().end(),
                          std::inserter(nameToAdd, nameToAdd.begin()));
      for (std::list<ndn::Name>::iterator it = nameToAdd.begin();
           it != nameToAdd.end(); ++it) {
        chkNameLsa->addName((*it));
        if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
          if ((*it) != m_nlsr.getConfParameter().getRouterPrefix()) {
            m_nlsr.getNamePrefixTable().addEntry((*it), nlsa.getOrigRouter());
          }
        }
      }
      std::list<ndn::Name> nameToRemove;
      std::set_difference(chkNameLsa->getNpl().getNameList().begin(),
                          chkNameLsa->getNpl().getNameList().end(),
                          nlsa.getNpl().getNameList().begin(),
                          nlsa.getNpl().getNameList().end(),
                          std::inserter(nameToRemove, nameToRemove.begin()));
      for (std::list<ndn::Name>::iterator it = nameToRemove.begin();
           it != nameToRemove.end(); ++it) {
        chkNameLsa->removeName((*it));
        if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
          if ((*it) != m_nlsr.getConfParameter().getRouterPrefix()) {
            m_nlsr.getNamePrefixTable().removeEntry((*it), nlsa.getOrigRouter());
          }
        }
      }
      if (nlsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
        ndn::time::system_clock::Duration duration = nlsa.getExpirationTimePoint() -
                                                     ndn::time::system_clock::now();
        timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
      }
      cancelScheduleLsaExpiringEvent(chkNameLsa->getExpiringEventId());
      chkNameLsa->setExpiringEventId(scheduleNameLsaExpiration(nlsa.getKey(),
                                                               nlsa.getLsSeqNo(),
                                                               timeToExpire));
      _LOG_DEBUG("Adding Name Lsa");
      chkNameLsa->writeLog();
    }
  }
  return true;
}

bool
Lsdb::addNameLsa(NameLsa& nlsa)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 bind(nameLsaCompareByKey, _1,
                                                      nlsa.getKey()));
  if (it == m_nameLsdb.end()) {
    m_nameLsdb.push_back(nlsa);
    return true;
  }
  return false;
}

bool
Lsdb::removeNameLsa(const ndn::Name& key)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 ndn::bind(nameLsaCompareByKey, _1, key));
  if (it != m_nameLsdb.end()) {
    _LOG_DEBUG("Deleting Name Lsa");
    (*it).writeLog();
    if ((*it).getOrigRouter() !=
        m_nlsr.getConfParameter().getRouterPrefix()) {
      m_nlsr.getNamePrefixTable().removeEntry((*it).getOrigRouter(),
                                              (*it).getOrigRouter());
      for (std::list<ndn::Name>::iterator nit = (*it).getNpl().getNameList().begin();
           nit != (*it).getNpl().getNameList().end(); ++nit) {
        if ((*nit) != m_nlsr.getConfParameter().getRouterPrefix()) {
          m_nlsr.getNamePrefixTable().removeEntry((*nit), (*it).getOrigRouter());
        }
      }
    }
    m_nameLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesNameLsaExist(const ndn::Name& key)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 ndn::bind(nameLsaCompareByKey, _1, key));
  if (it == m_nameLsdb.end()) {
    return false;
  }
  return true;
}

void
Lsdb::writeNameLsdbLog()
{
  _LOG_DEBUG("---------------Name LSDB-------------------");
  for (std::list<NameLsa>::iterator it = m_nameLsdb.begin();
       it != m_nameLsdb.end() ; it++) {
    (*it).writeLog();
  }
}

// Cor LSA and LSDB related Functions start here

static bool
corLsaCompareByKey(const CoordinateLsa& clsa, const ndn::Name& key)
{
  return clsa.getKey() == key;
}

bool
Lsdb::buildAndInstallOwnCoordinateLsa()
{
  CoordinateLsa corLsa(m_nlsr.getConfParameter().getRouterPrefix(),
                       "coordinate",
                       m_nlsr.getSequencingManager().getCorLsaSeq() + 1,
                       getLsaExpirationTimePoint(),
                       m_nlsr.getConfParameter().getCorR(),
                       m_nlsr.getConfParameter().getCorTheta());
  m_nlsr.getSequencingManager().increaseCorLsaSeq();
  installCoordinateLsa(corLsa);
  return true;
}

CoordinateLsa*
Lsdb::findCoordinateLsa(const ndn::Name& key)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       ndn::bind(corLsaCompareByKey, _1, key));
  if (it != m_corLsdb.end()) {
    return &(*it);
  }
  return 0;
}

bool
Lsdb::isCoordinateLsaNew(const ndn::Name& key, uint64_t seqNo)
{
  CoordinateLsa* clsa = findCoordinateLsa(key);
  if (clsa != 0) {
    if (clsa->getLsSeqNo() < seqNo) {
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

ndn::EventId
Lsdb::scheduleCoordinateLsaExpiration(const ndn::Name& key, int seqNo,
                                      const ndn::time::seconds& expTime)
{
  return m_nlsr.getScheduler().scheduleEvent(expTime,
                                             ndn::bind(&Lsdb::exprireOrRefreshCoordinateLsa,
                                                       this, key, seqNo));
}

bool
Lsdb::installCoordinateLsa(CoordinateLsa& clsa)
{
  ndn::time::seconds timeToExpire = ndn::time::seconds(m_lsaRefreshTime);
  CoordinateLsa* chkCorLsa = findCoordinateLsa(clsa.getKey());
  if (chkCorLsa == 0) {
    _LOG_DEBUG("New Coordinate LSA. Adding to LSDB");
    _LOG_DEBUG("Adding Coordinate Lsa");
    clsa.writeLog();
    addCoordinateLsa(clsa);

    if (clsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      m_nlsr.getNamePrefixTable().addEntry(clsa.getOrigRouter(),
                                           clsa.getOrigRouter());
    }
    if (m_nlsr.getConfParameter().getHyperbolicState() >= HYPERBOLIC_STATE_ON) {
      m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
    }
    if (clsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      ndn::time::system_clock::Duration duration = clsa.getExpirationTimePoint() -
                                                   ndn::time::system_clock::now();
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }
    scheduleCoordinateLsaExpiration(clsa.getKey(),
                                    clsa.getLsSeqNo(), timeToExpire);
  }
  else {
    if (chkCorLsa->getLsSeqNo() < clsa.getLsSeqNo()) {
      _LOG_DEBUG("Updated Coordinate LSA. Updating LSDB");
      _LOG_DEBUG("Deleting Coordinate Lsa");
      chkCorLsa->writeLog();
      chkCorLsa->setLsSeqNo(clsa.getLsSeqNo());
      chkCorLsa->setExpirationTimePoint(clsa.getExpirationTimePoint());
      if (!chkCorLsa->isEqualContent(clsa)) {
        chkCorLsa->setCorRadius(clsa.getCorRadius());
        chkCorLsa->setCorTheta(clsa.getCorTheta());
        if (m_nlsr.getConfParameter().getHyperbolicState() >= HYPERBOLIC_STATE_ON) {
          m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
        }
      }
      if (clsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
        ndn::time::system_clock::Duration duration = clsa.getExpirationTimePoint() -
                                                     ndn::time::system_clock::now();
        timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
      }
      cancelScheduleLsaExpiringEvent(chkCorLsa->getExpiringEventId());
      chkCorLsa->setExpiringEventId(scheduleCoordinateLsaExpiration(clsa.getKey(),
                                                                    clsa.getLsSeqNo(),
                                                                    timeToExpire));
      _LOG_DEBUG("Adding Coordinate Lsa");
      chkCorLsa->writeLog();
    }
  }
  return true;
}

bool
Lsdb::addCoordinateLsa(CoordinateLsa& clsa)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       ndn::bind(corLsaCompareByKey, _1,
                                                                 clsa.getKey()));
  if (it == m_corLsdb.end()) {
    m_corLsdb.push_back(clsa);
    return true;
  }
  return false;
}

bool
Lsdb::removeCoordinateLsa(const ndn::Name& key)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       ndn::bind(corLsaCompareByKey,
                                                                 _1, key));
  if (it != m_corLsdb.end()) {
    _LOG_DEBUG("Deleting Coordinate Lsa");
    (*it).writeLog();
    if ((*it).getOrigRouter() !=
        m_nlsr.getConfParameter().getRouterPrefix()) {
      m_nlsr.getNamePrefixTable().removeEntry((*it).getOrigRouter(),
                                              (*it).getOrigRouter());
    }
    m_corLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesCoordinateLsaExist(const ndn::Name& key)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       ndn::bind(corLsaCompareByKey,
                                                                 _1, key));
  if (it == m_corLsdb.end()) {
    return false;
  }
  return true;
}

void
Lsdb::writeCorLsdbLog()
{
  _LOG_DEBUG("---------------Cor LSDB-------------------");
  for (std::list<CoordinateLsa>::iterator it = m_corLsdb.begin();
       it != m_corLsdb.end() ; it++) {
    (*it).writeLog();
  }
}

// Adj LSA and LSDB related function starts here

static bool
adjLsaCompareByKey(AdjLsa& alsa, const ndn::Name& key)
{
  return alsa.getKey() == key;
}

void
Lsdb::scheduledAdjLsaBuild()
{
  _LOG_DEBUG("scheduledAdjLsaBuild Called");
  m_nlsr.setIsBuildAdjLsaSheduled(false);
  if (m_nlsr.getAdjacencyList().isAdjLsaBuildable(m_nlsr)) {
    int adjBuildCount = m_nlsr.getAdjBuildCount();
    if (adjBuildCount > 0) {
      if (m_nlsr.getAdjacencyList().getNumOfActiveNeighbor() > 0) {
        _LOG_DEBUG("Building and installing Adj LSA");
        buildAndInstallOwnAdjLsa();
      }
      else {
        ndn::Name key = m_nlsr.getConfParameter().getRouterPrefix();
        key.append("adjacency");
        removeAdjLsa(key);
        m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
      }
      m_nlsr.setAdjBuildCount(m_nlsr.getAdjBuildCount() - adjBuildCount);
    }
  }
  else {
    m_nlsr.setIsBuildAdjLsaSheduled(true);
    int schedulingTime = m_nlsr.getConfParameter().getInterestRetryNumber() *
                         m_nlsr.getConfParameter().getInterestResendTime();
    m_nlsr.getScheduler().scheduleEvent(ndn::time::seconds(schedulingTime),
                                        ndn::bind(&Lsdb::scheduledAdjLsaBuild,
                                                  this));
  }
}


bool
Lsdb::addAdjLsa(AdjLsa& alsa)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                bind(adjLsaCompareByKey, _1,
                                                     alsa.getKey()));
  if (it == m_adjLsdb.end()) {
    m_adjLsdb.push_back(alsa);
    return true;
  }
  return false;
}

AdjLsa*
Lsdb::findAdjLsa(const ndn::Name& key)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                bind(adjLsaCompareByKey, _1, key));
  if (it != m_adjLsdb.end()) {
    return &(*it);
  }
  return 0;
}


bool
Lsdb::isAdjLsaNew(const ndn::Name& key, uint64_t seqNo)
{
  AdjLsa*  adjLsaCheck = findAdjLsa(key);
  if (adjLsaCheck != 0) {
    if (adjLsaCheck->getLsSeqNo() < seqNo) {
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}


ndn::EventId
Lsdb::scheduleAdjLsaExpiration(const ndn::Name& key, int seqNo,
                               const ndn::time::seconds& expTime)
{
  return m_nlsr.getScheduler().scheduleEvent(expTime,
                                             ndn::bind(&Lsdb::exprireOrRefreshAdjLsa,
                                                       this, key, seqNo));
}

bool
Lsdb::installAdjLsa(AdjLsa& alsa)
{
  ndn::time::seconds timeToExpire = ndn::time::seconds(m_lsaRefreshTime);
  AdjLsa* chkAdjLsa = findAdjLsa(alsa.getKey());
  if (chkAdjLsa == 0) {
    _LOG_DEBUG("New Adj LSA. Adding to LSDB");
    _LOG_DEBUG("Adding Adj Lsa");
    alsa.writeLog();
    addAdjLsa(alsa);
    alsa.addNptEntries(m_nlsr);
    m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
    if (alsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
      ndn::time::system_clock::Duration duration = alsa.getExpirationTimePoint() -
                                                   ndn::time::system_clock::now();
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }
    scheduleAdjLsaExpiration(alsa.getKey(),
                             alsa.getLsSeqNo(), timeToExpire);
  }
  else {
    if (chkAdjLsa->getLsSeqNo() < alsa.getLsSeqNo()) {
      _LOG_DEBUG("Updated Adj LSA. Updating LSDB");
      _LOG_DEBUG("Deleting Adj Lsa");
      chkAdjLsa->writeLog();
      chkAdjLsa->setLsSeqNo(alsa.getLsSeqNo());
      chkAdjLsa->setExpirationTimePoint(alsa.getExpirationTimePoint());
      if (!chkAdjLsa->isEqualContent(alsa)) {
        chkAdjLsa->getAdl().reset();
        chkAdjLsa->getAdl().addAdjacents(alsa.getAdl());
        m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
      }
      if (alsa.getOrigRouter() != m_nlsr.getConfParameter().getRouterPrefix()) {
        ndn::time::system_clock::Duration duration = alsa.getExpirationTimePoint() -
                                                     ndn::time::system_clock::now();
        timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
      }
      cancelScheduleLsaExpiringEvent(chkAdjLsa->getExpiringEventId());
      chkAdjLsa->setExpiringEventId(scheduleAdjLsaExpiration(alsa.getKey(),
                                                             alsa.getLsSeqNo(),
                                                             timeToExpire));
      _LOG_DEBUG("Adding Adj Lsa");
      chkAdjLsa->writeLog();
    }
  }
  return true;
}

bool
Lsdb::buildAndInstallOwnAdjLsa()
{
  AdjLsa adjLsa(m_nlsr.getConfParameter().getRouterPrefix(),
                "adjacency",
                m_nlsr.getSequencingManager().getAdjLsaSeq() + 1,
                getLsaExpirationTimePoint(),
                m_nlsr.getAdjacencyList().getNumOfActiveNeighbor(),
                m_nlsr.getAdjacencyList());
  m_nlsr.getSequencingManager().increaseAdjLsaSeq();
  // publish routing update
  //ndn::Name lsaPrefix = m_nlsr.getConfParameter().getLsaPrefix();
  //lsaPrefix.append(m_nlsr.getConfParameter().getRouterPrefix());
  ndn::Name lsaPrefix = m_nlsr.getConfParameter().getLsaPrefix();
  lsaPrefix.append(m_nlsr.getConfParameter().getSiteName());
  lsaPrefix.append(m_nlsr.getConfParameter().getRouterName());
  m_nlsr.getSyncLogicHandler().publishRoutingUpdate(m_nlsr.getSequencingManager(),
                                                    lsaPrefix);
  return installAdjLsa(adjLsa);
}

bool
Lsdb::removeAdjLsa(const ndn::Name& key)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                ndn::bind(adjLsaCompareByKey, _1, key));
  if (it != m_adjLsdb.end()) {
    _LOG_DEBUG("Deleting Adj Lsa");
    (*it).writeLog();
    (*it).removeNptEntries(m_nlsr);
    m_adjLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesAdjLsaExist(const ndn::Name& key)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                bind(adjLsaCompareByKey, _1, key));
  if (it == m_adjLsdb.end()) {
    return false;
  }
  return true;
}

std::list<AdjLsa>&
Lsdb::getAdjLsdb()
{
  return m_adjLsdb;
}

void
Lsdb::setLsaRefreshTime(int lrt)
{
  m_lsaRefreshTime = lrt;
}

void
Lsdb::setThisRouterPrefix(string trp)
{
  m_thisRouterPrefix = trp;
}

void
Lsdb::exprireOrRefreshNameLsa(const ndn::Name& lsaKey, uint64_t seqNo)
{
  _LOG_DEBUG("Lsdb::exprireOrRefreshNameLsa Called");
  _LOG_DEBUG("LSA Key : " << lsaKey << " Seq No: " << seqNo);
  NameLsa* chkNameLsa = findNameLsa(lsaKey);
  if (chkNameLsa != 0) {
    _LOG_DEBUG("LSA Exists with seq no: " << chkNameLsa->getLsSeqNo());
    if (chkNameLsa->getLsSeqNo() == seqNo) {
      if (chkNameLsa->getOrigRouter() == m_thisRouterPrefix) {
        _LOG_DEBUG("Own Name LSA, so refreshing it");
        _LOG_DEBUG("Deleting Name Lsa");
        chkNameLsa->writeLog();
        chkNameLsa->setLsSeqNo(chkNameLsa->getLsSeqNo() + 1);
        m_nlsr.getSequencingManager().setNameLsaSeq(chkNameLsa->getLsSeqNo());
        chkNameLsa->setExpirationTimePoint(getLsaExpirationTimePoint());
        _LOG_DEBUG("Adding Name Lsa");
        chkNameLsa->writeLog();
        // schedule refreshing event again
        chkNameLsa->setExpiringEventId(scheduleNameLsaExpiration(chkNameLsa->getKey(),
                                                                 chkNameLsa->getLsSeqNo(),
                                                                 ndn::time::seconds(m_lsaRefreshTime)));
        // publish routing update
        //ndn::Name lsaPrefix = m_nlsr.getConfParameter().getLsaPrefix();
        //lsaPrefix.append(m_nlsr.getConfParameter().getRouterPrefix());
        ndn::Name lsaPrefix = m_nlsr.getConfParameter().getLsaPrefix();
        lsaPrefix.append(m_nlsr.getConfParameter().getSiteName());
        lsaPrefix.append(m_nlsr.getConfParameter().getRouterName());
        m_nlsr.getSyncLogicHandler().publishRoutingUpdate(m_nlsr.getSequencingManager(),
                                                          lsaPrefix);
      }
      else {
        _LOG_DEBUG("Other's Name LSA, so removing form LSDB");
        removeNameLsa(lsaKey);
      }
    }
  }
}

void
Lsdb::exprireOrRefreshAdjLsa(const ndn::Name& lsaKey, uint64_t seqNo)
{
  _LOG_DEBUG("Lsdb::exprireOrRefreshAdjLsa Called");
  _LOG_DEBUG("LSA Key : " << lsaKey << " Seq No: " << seqNo);
  AdjLsa* chkAdjLsa = findAdjLsa(lsaKey);
  if (chkAdjLsa != 0) {
    _LOG_DEBUG("LSA Exists with seq no: " << chkAdjLsa->getLsSeqNo());
    if (chkAdjLsa->getLsSeqNo() == seqNo) {
      if (chkAdjLsa->getOrigRouter() == m_thisRouterPrefix) {
        _LOG_DEBUG("Own Adj LSA, so refreshing it");
        _LOG_DEBUG("Deleting Adj Lsa");
        chkAdjLsa->writeLog();
        chkAdjLsa->setLsSeqNo(chkAdjLsa->getLsSeqNo() + 1);
        m_nlsr.getSequencingManager().setAdjLsaSeq(chkAdjLsa->getLsSeqNo());
        chkAdjLsa->setExpirationTimePoint(getLsaExpirationTimePoint());
        _LOG_DEBUG("Adding Adj Lsa");
        chkAdjLsa->writeLog();
        // schedule refreshing event again
        chkAdjLsa->setExpiringEventId(scheduleAdjLsaExpiration(chkAdjLsa->getKey(),
                                                               chkAdjLsa->getLsSeqNo(),
                                                               ndn::time::seconds(m_lsaRefreshTime)));
        // publish routing update
        //ndn::Name lsaPrefix = m_nlsr.getConfParameter().getLsaPrefix();
        //lsaPrefix.append(m_nlsr.getConfParameter().getRouterPrefix());
        ndn::Name lsaPrefix = m_nlsr.getConfParameter().getLsaPrefix();
        lsaPrefix.append(m_nlsr.getConfParameter().getSiteName());
        lsaPrefix.append(m_nlsr.getConfParameter().getRouterName());
        m_nlsr.getSyncLogicHandler().publishRoutingUpdate(m_nlsr.getSequencingManager(),
                                                          lsaPrefix);
      }
      else {
        _LOG_DEBUG("Other's Adj LSA, so removing form LSDB");
        removeAdjLsa(lsaKey);
      }
      // schedule Routing table calculaiton
      m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
    }
  }
}

void
Lsdb::exprireOrRefreshCoordinateLsa(const ndn::Name& lsaKey,
                                    uint64_t seqNo)
{
  _LOG_DEBUG("Lsdb::exprireOrRefreshCorLsa Called ");
  _LOG_DEBUG("LSA Key : " << lsaKey << " Seq No: " << seqNo);
  CoordinateLsa* chkCorLsa = findCoordinateLsa(lsaKey);
  if (chkCorLsa != 0) {
    _LOG_DEBUG("LSA Exists with seq no: " << chkCorLsa->getLsSeqNo());
    if (chkCorLsa->getLsSeqNo() == seqNo) {
      if (chkCorLsa->getOrigRouter() == m_thisRouterPrefix) {
        _LOG_DEBUG("Own Cor LSA, so refreshing it");
        _LOG_DEBUG("Deleting Coordinate Lsa");
        chkCorLsa->writeLog();
        chkCorLsa->setLsSeqNo(chkCorLsa->getLsSeqNo() + 1);
        m_nlsr.getSequencingManager().setCorLsaSeq(chkCorLsa->getLsSeqNo());
        chkCorLsa->setExpirationTimePoint(getLsaExpirationTimePoint());
        _LOG_DEBUG("Adding Coordinate Lsa");
        chkCorLsa->writeLog();
        // schedule refreshing event again
        chkCorLsa->setExpiringEventId(scheduleCoordinateLsaExpiration(
                                        chkCorLsa->getKey(),
                                        chkCorLsa->getLsSeqNo(),
                                        ndn::time::seconds(m_lsaRefreshTime)));
        // publish routing update
        //ndn::Name lsaPrefix = m_nlsr.getConfParameter().getLsaPrefix();
        //lsaPrefix.append(m_nlsr.getConfParameter().getRouterPrefix());
        ndn::Name lsaPrefix = m_nlsr.getConfParameter().getLsaPrefix();
        lsaPrefix.append(m_nlsr.getConfParameter().getSiteName());
        lsaPrefix.append(m_nlsr.getConfParameter().getRouterName());
        m_nlsr.getSyncLogicHandler().publishRoutingUpdate(m_nlsr.getSequencingManager(),
                                                          lsaPrefix);
      }
      else {
        _LOG_DEBUG("Other's Cor LSA, so removing form LSDB");
        removeCoordinateLsa(lsaKey);
      }
      if (m_nlsr.getConfParameter().getHyperbolicState() >= HYPERBOLIC_STATE_ON) {
        m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
      }
    }
  }
}


void
Lsdb::expressInterest(const ndn::Name& interestName, uint32_t interestLifeTime,
                      uint32_t timeoutCount)
{
  _LOG_DEBUG("Expressing Interest for LSA(name): " << interestName);
  ndn::Interest interest(interestName);
  interest.setInterestLifetime(ndn::time::seconds(interestLifeTime));
  interest.setMustBeFresh(true);
  m_nlsr.getNlsrFace().expressInterest(interest,
                                       ndn::bind(&Lsdb::onContent,
                                                 this, _1, _2),
                                       ndn::bind(&Lsdb::processInterestTimedOut,
                                                 this, _1, timeoutCount));
}

void
Lsdb::processInterest(const ndn::Name& name, const ndn::Interest& interest)
{
  const ndn::Name& intName(interest.getName());
  _LOG_DEBUG("Interest recevied for LSA(name): " << intName);
  string chkString("LSA");
  int32_t lsaPosition = util::getNameComponentPosition(interest.getName(),
                                                       chkString);
  if (lsaPosition >= 0) {
    std::string interestedLsType;
    uint64_t interestedLsSeqNo;
    ndn::Name origRouter = m_nlsr.getConfParameter().getNetwork();
    origRouter.append(intName.getSubName(lsaPosition + 1,
                                         interest.getName().size() - lsaPosition - 3));
    interestedLsType  = intName[-2].toUri();
    interestedLsSeqNo = intName[-1].toNumber();
    if (interestedLsType == "name") {
      processInterestForNameLsa(interest,
                                origRouter.append(interestedLsType),
                                interestedLsSeqNo);
      return;
    }
    else if (interestedLsType == "adjacency") {
      processInterestForAdjacencyLsa(interest,
                                     origRouter.append(interestedLsType),
                                     interestedLsSeqNo);
      return;
    }
    else if (interestedLsType == "coordinate") {
      processInterestForCoordinateLsa(interest,
                                      origRouter.append(interestedLsType),
                                      interestedLsSeqNo);
      return;
    }
    else {
      _LOG_DEBUG("Unrecognized LSA Type :(");
    }
  }
}

void
Lsdb::processInterestForNameLsa(const ndn::Interest& interest,
                                const ndn::Name& lsaKey,
                                uint32_t interestedlsSeqNo)
{
  NameLsa*  nameLsa = m_nlsr.getLsdb().findNameLsa(lsaKey);
  if (nameLsa != 0) {
    if (nameLsa->getLsSeqNo() >= interestedlsSeqNo) {
      ndn::Data data(ndn::Name(interest.getName()).appendVersion());
      _LOG_DEBUG("Sending data for LSA(name): " << interest.getName());
      data.setFreshnessPeriod(ndn::time::seconds(10)); // 10 sec
      std::string content = nameLsa->getData();
      data.setContent(reinterpret_cast<const uint8_t*>(content.c_str()),
                      content.size());
      m_nlsr.getKeyChain().sign(data, m_nlsr.getDefaultCertName());
      m_nlsr.getNlsrFace().put(data);
    }
  }
}

void
Lsdb::processInterestForAdjacencyLsa(const ndn::Interest& interest,
                                     const ndn::Name& lsaKey,
                                     uint32_t interestedlsSeqNo)
{
  AdjLsa* adjLsa = m_nlsr.getLsdb().findAdjLsa(lsaKey);
  if (adjLsa != 0) {
    if (adjLsa->getLsSeqNo() >= interestedlsSeqNo) {
      ndn::Data data(ndn::Name(interest.getName()).appendVersion());
      _LOG_DEBUG("Sending data for LSA(name): " << interest.getName());
      data.setFreshnessPeriod(ndn::time::seconds(10)); // 10 sec
      std::string content = adjLsa->getData();
      data.setContent(reinterpret_cast<const uint8_t*>(content.c_str()),
                      content.size());
      m_nlsr.getKeyChain().sign(data, m_nlsr.getDefaultCertName());
      m_nlsr.getNlsrFace().put(data);
    }
  }
}

void
Lsdb::processInterestForCoordinateLsa(const ndn::Interest& interest,
                                      const ndn::Name& lsaKey,
                                      uint32_t interestedlsSeqNo)
{
  CoordinateLsa* corLsa = m_nlsr.getLsdb().findCoordinateLsa(lsaKey);
  if (corLsa != 0) {
    if (corLsa->getLsSeqNo() >= interestedlsSeqNo) {
      ndn::Data data(ndn::Name(interest.getName()).appendVersion());
      _LOG_DEBUG("Sending data for LSA(name): " << interest.getName());
      data.setFreshnessPeriod(ndn::time::seconds(10)); // 10 sec
      std::string content = corLsa->getData();
      data.setContent(reinterpret_cast<const uint8_t*>(content.c_str()),
                      content.size());
      m_nlsr.getKeyChain().sign(data, m_nlsr.getDefaultCertName());
      m_nlsr.getNlsrFace().put(data);
    }
  }
}

void
Lsdb::onContent(const ndn::Interest& interest, const ndn::Data& data)
{
  m_nlsr.getValidator().validate(data,
                                 ndn::bind(&Lsdb::onContentValidated, this, _1),
                                 ndn::bind(&Lsdb::onContentValidationFailed, this, _1, _2));

}

void
Lsdb::onContentValidated(const ndn::shared_ptr<const ndn::Data>& data)
{
  const ndn::Name& dataName = data->getName();
  _LOG_DEBUG("Data received for LSA(name): " << dataName);
  string dataContent(reinterpret_cast<const char*>(data->getContent().value()));
  string chkString("LSA");
  int32_t lsaPosition = util::getNameComponentPosition(dataName, chkString);
  if (lsaPosition >= 0) {
    std::string interestedLsType;
    uint64_t interestedLsSeqNo;
    ndn::Name origRouter = m_nlsr.getConfParameter().getNetwork();
    origRouter.append(dataName.getSubName(lsaPosition + 1,
                                          dataName.size() - lsaPosition - 4));
    interestedLsType  = dataName[-3].toUri();
    interestedLsSeqNo = dataName[-2].toNumber();
    if (interestedLsType == "name") {
      processContentNameLsa(origRouter.append(interestedLsType),
                            interestedLsSeqNo, dataContent);
      return;
    }
    else if (interestedLsType == "adjacency") {
      processContentAdjacencyLsa(origRouter.append(interestedLsType),
                                 interestedLsSeqNo, dataContent);
      return;
    }
    else if (interestedLsType == "coordinate") {
      processContentCoordinateLsa(origRouter.append(interestedLsType),
                                  interestedLsSeqNo, dataContent);
      return;
    }
    else {
      _LOG_DEBUG("Unrecognized LSA Type :(");
    }
  }
}

void
Lsdb::onContentValidationFailed(const ndn::shared_ptr<const ndn::Data>& data, const std::string& msg)
{
  _LOG_DEBUG("Validation Error: " << msg);
}

void
Lsdb::processContentNameLsa(const ndn::Name& lsaKey,
                            uint32_t lsSeqNo, std::string& dataContent)
{
  if (isNameLsaNew(lsaKey, lsSeqNo)) {
    NameLsa nameLsa;
    if (nameLsa.initializeFromContent(dataContent)) {
      installNameLsa(nameLsa);
    }
    else {
      _LOG_DEBUG("LSA data decoding error :(");
    }
  }
}

void
Lsdb::processContentAdjacencyLsa(const ndn::Name& lsaKey,
                                 uint32_t lsSeqNo, std::string& dataContent)
{
  if (isAdjLsaNew(lsaKey, lsSeqNo)) {
    AdjLsa adjLsa;
    if (adjLsa.initializeFromContent(dataContent)) {
      installAdjLsa(adjLsa);
    }
    else {
      _LOG_DEBUG("LSA data decoding error :(");
    }
  }
}

void
Lsdb::processContentCoordinateLsa(const ndn::Name& lsaKey,
                                  uint32_t lsSeqNo, std::string& dataContent)
{
  if (isCoordinateLsaNew(lsaKey, lsSeqNo)) {
    CoordinateLsa corLsa;
    if (corLsa.initializeFromContent(dataContent)) {
      installCoordinateLsa(corLsa);
    }
    else {
      _LOG_DEBUG("LSA data decoding error :(");
    }
  }
}

void
Lsdb::processInterestTimedOut(const ndn::Interest& interest, uint32_t timeoutCount)
{
  const ndn::Name& interestName(interest.getName());
  _LOG_DEBUG("Interest timed out for  LSA(name): " << interestName);
  if ((timeoutCount + 1) <= m_nlsr.getConfParameter().getInterestRetryNumber()) {
    _LOG_DEBUG("Interest timeoutCount: " << (timeoutCount + 1));
    _LOG_DEBUG("Need to express interest again for LSA(name): " << interestName);
    expressInterest(interestName,
                    m_nlsr.getConfParameter().getInterestResendTime(),
                    timeoutCount + 1);
  }
}

ndn::time::system_clock::TimePoint
Lsdb::getLsaExpirationTimePoint()
{
  ndn::time::system_clock::TimePoint expirationTimePoint = ndn::time::system_clock::now();
  expirationTimePoint = expirationTimePoint +
                        ndn::time::seconds(m_nlsr.getConfParameter().getRouterDeadInterval());
  return expirationTimePoint;
}

void
Lsdb::writeAdjLsdbLog()
{
  _LOG_DEBUG("---------------Adj LSDB-------------------");
  for (std::list<AdjLsa>::iterator it = m_adjLsdb.begin();
       it != m_adjLsdb.end() ; it++) {
    (*it).writeLog();
  }
}

//-----utility function -----
bool
Lsdb::doesLsaExist(const ndn::Name& key, const std::string& lsType)
{
  if (lsType == "name") {
    return doesNameLsaExist(key);
  }
  else if (lsType == "adjacency") {
    return doesAdjLsaExist(key);
  }
  else if (lsType == "coordinate") {
    return doesCoordinateLsaExist(key);
  }
  return false;
}

}//namespace nlsr
