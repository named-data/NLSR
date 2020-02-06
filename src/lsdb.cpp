/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  The University of Memphis,
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

#include "lsdb.hpp"

#include "logger.hpp"
#include "nlsr.hpp"
#include "utility/name-helper.hpp"

#include <ndn-cxx/security/signing-helpers.hpp>

namespace nlsr {

INIT_LOGGER(Lsdb);

const ndn::Name::Component Lsdb::NAME_COMPONENT = ndn::Name::Component("lsdb");
const ndn::time::seconds Lsdb::GRACE_PERIOD = ndn::time::seconds(10);
const ndn::time::steady_clock::TimePoint Lsdb::DEFAULT_LSA_RETRIEVAL_DEADLINE =
  ndn::time::steady_clock::TimePoint::min();

Lsdb::Lsdb(ndn::Face& face, ndn::KeyChain& keyChain, ConfParameter& confParam,
           NamePrefixTable& namePrefixTable, RoutingTable& routingTable)
  : m_face(face)
  , m_scheduler(face.getIoService())
  , m_confParam(confParam)
  , m_namePrefixTable(namePrefixTable)
  , m_routingTable(routingTable)
  , m_sync(m_face,
           [this] (const ndn::Name& routerName, const Lsa::Type& lsaType,
                   const uint64_t& sequenceNumber) {
             return isLsaNew(routerName, lsaType, sequenceNumber);
           }, m_confParam)
  , m_lsaRefreshTime(ndn::time::seconds(m_confParam.getLsaRefreshTime()))
  , m_thisRouterPrefix(m_confParam.getRouterPrefix().toUri())
  , m_adjLsaBuildInterval(m_confParam.getAdjLsaBuildInterval())
  , m_sequencingManager(m_confParam.getStateFileDir(), m_confParam.getHyperbolicState())
  , m_onNewLsaConnection(m_sync.onNewLsa->connect(
      [this] (const ndn::Name& updateName, uint64_t sequenceNumber,
              const ndn::Name& originRouter) {
        ndn::Name lsaInterest{updateName};
        lsaInterest.appendNumber(sequenceNumber);
        expressInterest(lsaInterest, 0);
      }))
  , m_segmentPublisher(m_face, keyChain)
  , m_isBuildAdjLsaSheduled(false)
  , m_adjBuildCount(0)
{
}

Lsdb::~Lsdb()
{
  for (const auto& sp : m_fetchers) {
    sp->stop();
  }
}

void
Lsdb::onFetchLsaError(uint32_t errorCode,
                      const std::string& msg,
                      const ndn::Name& interestName,
                      uint32_t retransmitNo,
                      const ndn::time::steady_clock::TimePoint& deadline,
                      ndn::Name lsaName,
                      uint64_t seqNo)
{
  NLSR_LOG_DEBUG("Failed to fetch LSA: " << lsaName << ", Error code: " << errorCode
                 << ", Message: " << msg);

  if (ndn::time::steady_clock::now() < deadline) {
    auto it = m_highestSeqNo.find(lsaName);
    if (it != m_highestSeqNo.end() && it->second == seqNo) {
      // If the SegmentFetcher failed due to an Interest timeout, it is safe to re-express
      // immediately since at the least the LSA Interest lifetime has elapsed.
      // Otherwise, it is necessary to delay the Interest re-expression to prevent
      // the potential for constant Interest flooding.
      ndn::time::seconds delay = m_confParam.getLsaInterestLifetime();

      if (errorCode == ndn::util::SegmentFetcher::ErrorCode::INTEREST_TIMEOUT) {
        delay = ndn::time::seconds(0);
      }

      m_scheduler.schedule(delay, std::bind(&Lsdb::expressInterest, this,
                                            interestName, retransmitNo + 1, deadline));
    }
  }
}

void
Lsdb::afterFetchLsa(const ndn::ConstBufferPtr& bufferPtr, const ndn::Name& interestName)
{
  NLSR_LOG_DEBUG("Received data for LSA interest: " << interestName);

  ndn::Name lsaName = interestName.getSubName(0, interestName.size()-1);
  uint64_t seqNo = interestName[-1].toNumber();

  if (m_highestSeqNo.find(lsaName) == m_highestSeqNo.end()) {
    m_highestSeqNo[lsaName] = seqNo;
  }
  else if (seqNo > m_highestSeqNo[lsaName]) {
    m_highestSeqNo[lsaName] = seqNo;
    NLSR_LOG_TRACE("SeqNo for LSA(name): " << interestName << "  updated");
  }
  else if (seqNo < m_highestSeqNo[lsaName]) {
    return;
  }

  std::string chkString("LSA");
  int32_t lsaPosition = util::getNameComponentPosition(interestName, chkString);

  if (lsaPosition >= 0) {
    // Extracts the prefix of the originating router from the data.
    ndn::Name originRouter = m_confParam.getNetwork();
    originRouter.append(interestName.getSubName(lsaPosition + 1,
                                                interestName.size() - lsaPosition - 3));

    try {
      ndn::Block block(bufferPtr);
      Lsa::Type interestedLsType;
      std::istringstream(interestName[-2].toUri()) >> interestedLsType;

      if (interestedLsType == Lsa::Type::NAME) {
        processContentNameLsa(originRouter.append(boost::lexical_cast<std::string>(interestedLsType)),
                              seqNo,
                              block);
      }
      else if (interestedLsType == Lsa::Type::ADJACENCY) {
        processContentAdjacencyLsa(originRouter.append(boost::lexical_cast<std::string>(interestedLsType)),
                                   seqNo,
                                   block);
      }
      else if (interestedLsType == Lsa::Type::COORDINATE) {
        processContentCoordinateLsa(originRouter.append(boost::lexical_cast<std::string>(interestedLsType)),
                                    seqNo,
                                    block);
      }
      else {
        NLSR_LOG_WARN("Received unrecognized LSA Type: " << interestedLsType);
      }
    }
    catch (const std::exception& e) {
      NLSR_LOG_TRACE("LSA data decoding error :( " << e.what());
      return;
    }

    lsaIncrementSignal(Statistics::PacketType::RCV_LSA_DATA);
  }
}

  /*! \brief Compares if a name LSA is the same as the one specified by key

    \param nlsa1 A name LSA object
    \param key A key of an originating router to compare to nlsa1
   */
static bool
nameLsaCompareByKey(const NameLsa& nlsa1, const ndn::Name& key)
{
  return nlsa1.getKey() == key;
}

bool
Lsdb::buildAndInstallOwnNameLsa()
{
  NameLsa nameLsa(m_confParam.getRouterPrefix(),
                  m_sequencingManager.getNameLsaSeq() + 1,
                  getLsaExpirationTimePoint(),
                  m_confParam.getNamePrefixList());
  m_sequencingManager.increaseNameLsaSeq();

  m_sequencingManager.writeSeqNoToFile();
  m_sync.publishRoutingUpdate(Lsa::Type::NAME, m_sequencingManager.getNameLsaSeq());

  return installNameLsa(nameLsa);
}

NameLsa*
Lsdb::findNameLsa(const ndn::Name& key)
{
  auto it = std::find_if(m_nameLsdb.begin(), m_nameLsdb.end(),
                         std::bind(nameLsaCompareByKey, _1, key));
  if (it != m_nameLsdb.end()) {
    return &*it;
  }
  return nullptr;
}

bool
Lsdb::isNameLsaNew(const ndn::Name& key, uint64_t seqNo)
{
  NameLsa* nameLsaCheck = findNameLsa(key);
  // Is the name in the LSDB
  if (nameLsaCheck != nullptr) {
    // And the supplied seq no is the highest so far
    if (nameLsaCheck->getSeqNo() < seqNo) {
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

ndn::scheduler::EventId
Lsdb::scheduleNameLsaExpiration(const ndn::Name& key, int seqNo,
                                const ndn::time::seconds& expTime)
{
  return m_scheduler.schedule(expTime + GRACE_PERIOD,
                              std::bind(&Lsdb::expireOrRefreshNameLsa, this, key, seqNo));
}

bool
Lsdb::installNameLsa(NameLsa& nlsa)
{
  NLSR_LOG_TRACE("installNameLsa");
  ndn::time::seconds timeToExpire = m_lsaRefreshTime;
  NameLsa* chkNameLsa = findNameLsa(nlsa.getKey());
  // Determines if the name LSA is new or not.
  if (chkNameLsa == nullptr) {
    addNameLsa(nlsa);
    NLSR_LOG_DEBUG("New Name LSA");
    NLSR_LOG_DEBUG("Adding Name Lsa");
    NLSR_LOG_DEBUG(nlsa);

    NLSR_LOG_TRACE("nlsa.getOriginRouter(): " << nlsa.getOriginRouter());
    NLSR_LOG_TRACE("m_confParam.getRouterPrefix(): " << m_confParam.getRouterPrefix());

    if (nlsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
      // If this name LSA is from another router, add the advertised
      // prefixes to the NPT.
      m_namePrefixTable.addEntry(nlsa.getOriginRouter(), nlsa.getOriginRouter());

      for (const auto& name : nlsa.getNpl().getNames()) {
        if (name != m_confParam.getRouterPrefix()) {
          m_namePrefixTable.addEntry(name, nlsa.getOriginRouter());
        }
      }
      auto duration = nlsa.getExpirationTimePoint() - ndn::time::system_clock::now();
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }

    nlsa.setExpiringEventId(scheduleNameLsaExpiration(nlsa.getKey(),
                                                      nlsa.getSeqNo(),
                                                      timeToExpire));
  }
  // Else this is a known name LSA, so we are updating it.
  else {
    NLSR_LOG_TRACE("Known name lsa");
    NLSR_LOG_TRACE("chkNameLsa->getSeqNo(): " << chkNameLsa->getSeqNo());
    NLSR_LOG_TRACE("nlsa.getSeqNo(): " << nlsa.getSeqNo());
    if (chkNameLsa->getSeqNo() < nlsa.getSeqNo()) {
      NLSR_LOG_DEBUG("Updated Name LSA. Updating LSDB");
      NLSR_LOG_DEBUG("Deleting Name Lsa");
      NLSR_LOG_DEBUG(chkNameLsa);
      chkNameLsa->setSeqNo(nlsa.getSeqNo());
      chkNameLsa->setExpirationTimePoint(nlsa.getExpirationTimePoint());
      chkNameLsa->getNpl().sort();
      nlsa.getNpl().sort();
      // Obtain the set difference of the current and the incoming
      // name prefix sets, and add those.
      std::list<ndn::Name> newNames = nlsa.getNpl().getNames();
      std::list<ndn::Name> oldNames = chkNameLsa->getNpl().getNames();
      std::list<ndn::Name> namesToAdd;
      std::set_difference(newNames.begin(), newNames.end(), oldNames.begin(), oldNames.end(),
                          std::inserter(namesToAdd, namesToAdd.begin()));
      for (const auto& name : namesToAdd) {
        chkNameLsa->addName(name);
        if (nlsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
          if (name != m_confParam.getRouterPrefix()) {
            m_namePrefixTable.addEntry(name, nlsa.getOriginRouter());
          }
        }
      }

      chkNameLsa->getNpl().sort();

      // Also remove any names that are no longer being advertised.
      std::list<ndn::Name> namesToRemove;
      std::set_difference(oldNames.begin(), oldNames.end(), newNames.begin(), newNames.end(),
                          std::inserter(namesToRemove, namesToRemove.begin()));
      for (const auto& name : namesToRemove) {
        NLSR_LOG_DEBUG("Removing name LSA no longer advertised: " << name);
        chkNameLsa->removeName(name);
        if (nlsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
          if (name != m_confParam.getRouterPrefix()) {
            m_namePrefixTable.removeEntry(name, nlsa.getOriginRouter());
          }
        }
      }

      if (nlsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
        auto duration = nlsa.getExpirationTimePoint() - ndn::time::system_clock::now();
        timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
      }
      chkNameLsa->getExpiringEventId().cancel();
      chkNameLsa->setExpiringEventId(scheduleNameLsaExpiration(nlsa.getKey(),
                                                               nlsa.getSeqNo(),
                                                               timeToExpire));
      NLSR_LOG_DEBUG("Adding Name Lsa");
      NLSR_LOG_DEBUG(chkNameLsa);
    }
  }
  return true;
}

bool
Lsdb::addNameLsa(NameLsa& nlsa)
{
  auto it = std::find_if(m_nameLsdb.begin(), m_nameLsdb.end(),
                         std::bind(nameLsaCompareByKey, _1, nlsa.getKey()));
  if (it == m_nameLsdb.end()) {
    m_nameLsdb.push_back(nlsa);
    return true;
  }
  return false;
}

bool
Lsdb::removeNameLsa(const ndn::Name& key)
{
  auto it = std::find_if(m_nameLsdb.begin(), m_nameLsdb.end(),
                         std::bind(nameLsaCompareByKey, _1, key));
  if (it != m_nameLsdb.end()) {
    NLSR_LOG_DEBUG("Deleting Name Lsa");
    NLSR_LOG_DEBUG(*it);
    // If the requested name LSA is not ours, we also need to remove
    // its entries from the NPT.
    if (it->getOriginRouter() != m_confParam.getRouterPrefix()) {
      m_namePrefixTable.removeEntry(it->getOriginRouter(), it->getOriginRouter());

      for (const auto& name : it->getNpl().getNames()) {
        if (name != m_confParam.getRouterPrefix()) {
          m_namePrefixTable.removeEntry(name, it->getOriginRouter());
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
  auto it = std::find_if(m_nameLsdb.begin(), m_nameLsdb.end(),
                         std::bind(nameLsaCompareByKey, _1, key));
  return it != m_nameLsdb.end();
}

void
Lsdb::writeNameLsdbLog()
{
  NLSR_LOG_DEBUG("---------------Name LSDB-------------------");
  for (const auto& nlsa : m_nameLsdb) {
    NLSR_LOG_DEBUG(nlsa);
  }
}

const std::list<NameLsa>&
Lsdb::getNameLsdb() const
{
  return m_nameLsdb;
}

// Cor LSA and LSDB related Functions start here

/*! \brief Compares whether an LSA object is the same as a key.
  \param clsa The cor. LSA to check the identity of.
  \param key The key of the publishing router to check against.
*/
static bool
corLsaCompareByKey(const CoordinateLsa& clsa, const ndn::Name& key)
{
  return clsa.getKey() == key;
}

bool
Lsdb::buildAndInstallOwnCoordinateLsa()
{
  CoordinateLsa corLsa(m_confParam.getRouterPrefix(),
                       m_sequencingManager.getCorLsaSeq() + 1,
                       getLsaExpirationTimePoint(),
                       m_confParam.getCorR(),
                       m_confParam.getCorTheta());

  // Sync coordinate LSAs if using HR or HR dry run.
  if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
    m_sequencingManager.increaseCorLsaSeq();
    m_sequencingManager.writeSeqNoToFile();
    m_sync.publishRoutingUpdate(Lsa::Type::COORDINATE, m_sequencingManager.getCorLsaSeq());
  }

  installCoordinateLsa(corLsa);

  return true;
}

CoordinateLsa*
Lsdb::findCoordinateLsa(const ndn::Name& key)
{
  auto it = std::find_if(m_corLsdb.begin(), m_corLsdb.end(),
                         std::bind(corLsaCompareByKey, _1, key));
  if (it != m_corLsdb.end()) {
    return &*it;
  }
  return nullptr;
}

bool
Lsdb::isCoordinateLsaNew(const ndn::Name& key, uint64_t seqNo)
{
  CoordinateLsa* clsa = findCoordinateLsa(key);
  // Is the coordinate LSA in the LSDB already
  if (clsa != nullptr) {
    // And the seq no is newer (higher) than the current one
    if (clsa->getSeqNo() < seqNo) {
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

  // Schedules a refresh/expire event in the scheduler.
  // \param key The name of the router that published the LSA.
  // \param seqNo the seq. no. associated with the LSA to check.
  // \param expTime How long to wait before triggering the event.
ndn::scheduler::EventId
Lsdb::scheduleCoordinateLsaExpiration(const ndn::Name& key, int seqNo,
                                      const ndn::time::seconds& expTime)
{
  return m_scheduler.schedule(expTime + GRACE_PERIOD,
                              std::bind(&Lsdb::expireOrRefreshCoordinateLsa, this, key, seqNo));
}

bool
Lsdb::installCoordinateLsa(CoordinateLsa& clsa)
{
  ndn::time::seconds timeToExpire = m_lsaRefreshTime;
  CoordinateLsa* chkCorLsa = findCoordinateLsa(clsa.getKey());
  // Checking whether the LSA is new or not.
  if (chkCorLsa == nullptr) {
    NLSR_LOG_DEBUG("New Coordinate LSA. Adding to LSDB");
    NLSR_LOG_DEBUG("Adding Coordinate Lsa");
    NLSR_LOG_DEBUG(clsa);
    addCoordinateLsa(clsa);

    // Register the LSA's origin router prefix
    if (clsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
      m_namePrefixTable.addEntry(clsa.getOriginRouter(), clsa.getOriginRouter());
    }
    if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
      m_routingTable.scheduleRoutingTableCalculation();
    }
    // Set the expiration time for the new LSA.
    if (clsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
      auto duration = clsa.getExpirationTimePoint() - ndn::time::system_clock::now();
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }
    scheduleCoordinateLsaExpiration(clsa.getKey(), clsa.getSeqNo(), timeToExpire);
  }
  // We are just updating this LSA.
  else {
    if (chkCorLsa->getSeqNo() < clsa.getSeqNo()) {
      NLSR_LOG_DEBUG("Updated Coordinate LSA. Updating LSDB");
      NLSR_LOG_DEBUG("Deleting Coordinate Lsa");
      NLSR_LOG_DEBUG(chkCorLsa);
      chkCorLsa->setSeqNo(clsa.getSeqNo());
      chkCorLsa->setExpirationTimePoint(clsa.getExpirationTimePoint());
      // If the new LSA contains new routing information, update the LSDB with it.
      if (!chkCorLsa->isEqualContent(clsa)) {
        chkCorLsa->setCorRadius(clsa.getCorRadius());
        chkCorLsa->setCorTheta(clsa.getCorTheta());
        if (m_confParam.getHyperbolicState() >= HYPERBOLIC_STATE_ON) {
          m_routingTable.scheduleRoutingTableCalculation();
        }
      }
      // If this is an LSA from another router, refresh its expiration time.
      if (clsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
        auto duration = clsa.getExpirationTimePoint() - ndn::time::system_clock::now();
        timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
      }
      chkCorLsa->getExpiringEventId().cancel();
      chkCorLsa->setExpiringEventId(scheduleCoordinateLsaExpiration(clsa.getKey(),
                                                                    clsa.getSeqNo(),
                                                                    timeToExpire));
      NLSR_LOG_DEBUG("Adding Coordinate Lsa");
      NLSR_LOG_DEBUG(chkCorLsa);
    }
  }
  return true;
}

bool
Lsdb::addCoordinateLsa(CoordinateLsa& clsa)
{
  auto it = std::find_if(m_corLsdb.begin(), m_corLsdb.end(),
                         std::bind(corLsaCompareByKey, _1, clsa.getKey()));
  if (it == m_corLsdb.end()) {
    m_corLsdb.push_back(clsa);
    return true;
  }
  return false;
}

bool
Lsdb::removeCoordinateLsa(const ndn::Name& key)
{
  auto it = std::find_if(m_corLsdb.begin(), m_corLsdb.end(),
                         std::bind(corLsaCompareByKey, _1, key));

  if (it != m_corLsdb.end()) {
    NLSR_LOG_DEBUG("Deleting Coordinate Lsa");
    NLSR_LOG_DEBUG(*it);

    if (it->getOriginRouter() != m_confParam.getRouterPrefix()) {
      m_namePrefixTable.removeEntry(it->getOriginRouter(), it->getOriginRouter());
    }

    m_corLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesCoordinateLsaExist(const ndn::Name& key)
{
  auto it = std::find_if(m_corLsdb.begin(), m_corLsdb.end(),
                         std::bind(corLsaCompareByKey, _1, key));
  return it != m_corLsdb.end();
}

void
Lsdb::writeCorLsdbLog()
{
  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_OFF) {
    return;
  }

  NLSR_LOG_DEBUG("---------------Cor LSDB-------------------");
  for (const auto& corLsa : m_corLsdb) {
    NLSR_LOG_DEBUG(corLsa);
  }
}

const std::list<CoordinateLsa>&
Lsdb::getCoordinateLsdb() const
{
  return m_corLsdb;
}

// Adj LSA and LSDB related function starts here

  /*! \brief Returns whether an adj. LSA object is from some router.
    \param alsa The adj. LSA object.
    \param key The router name that you want to compare the LSA with.
   */
static bool
adjLsaCompareByKey(AdjLsa& alsa, const ndn::Name& key)
{
  return alsa.getKey() == key;
}

void
Lsdb::scheduleAdjLsaBuild()
{
  m_adjBuildCount++;

  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
    // Don't build adjacency LSAs in hyperbolic routing
    NLSR_LOG_DEBUG("Adjacency LSA not built. Currently in hyperbolic routing state.");
    return;
  }

  if (m_isBuildAdjLsaSheduled) {
    NLSR_LOG_DEBUG("Rescheduling Adjacency LSA build in " << m_adjLsaBuildInterval);
  }
  else {
    NLSR_LOG_DEBUG("Scheduling Adjacency LSA build in " << m_adjLsaBuildInterval);
    m_isBuildAdjLsaSheduled = true;
  }
  m_scheduledAdjLsaBuild = m_scheduler.schedule(m_adjLsaBuildInterval, [this] { buildAdjLsa(); });
}

void
Lsdb::buildAdjLsa()
{
  NLSR_LOG_TRACE("Lsdb::buildAdjLsa called");

  m_isBuildAdjLsaSheduled = false;

  if (m_confParam.getAdjacencyList().isAdjLsaBuildable(m_confParam.getInterestRetryNumber())) {

    int adjBuildCount = m_adjBuildCount;
    // Only do the adjLsa build if there's one scheduled
    if (adjBuildCount > 0) {
      // It only makes sense to do the adjLsa build if we have neighbors
      if (m_confParam.getAdjacencyList().getNumOfActiveNeighbor() > 0) {
        NLSR_LOG_DEBUG("Building and installing own Adj LSA");
        buildAndInstallOwnAdjLsa();
      }
      // We have no active neighbors, meaning no one can route through
      // us.  So delete our entry in the LSDB. This prevents this
      // router from refreshing the LSA, eventually causing other
      // routers to delete it, too.
      else {
        NLSR_LOG_DEBUG("Removing own Adj LSA; no ACTIVE neighbors");
        // Get this router's key
        ndn::Name key = m_confParam.getRouterPrefix();
        key.append(boost::lexical_cast<std::string>(Lsa::Type::ADJACENCY));

        removeAdjLsa(key);
        // Recompute routing table after removal
        m_routingTable.scheduleRoutingTableCalculation();
      }
      // In the case that during building the adj LSA, the FIB has to
      // wait on an Interest response, the number of scheduled adj LSA
      // builds could change, so we shouldn't just set it to 0.
      m_adjBuildCount = m_adjBuildCount - adjBuildCount;
    }
  }
  // We are still waiting to know the adjacency status of some
  // neighbor, so schedule a build for later (when all that has
  // hopefully finished)
  else {
    m_isBuildAdjLsaSheduled = true;
    auto schedulingTime = ndn::time::seconds(m_confParam.getInterestRetryNumber() *
                                             m_confParam.getInterestResendTime());
    m_scheduledAdjLsaBuild = m_scheduler.schedule(schedulingTime, [this] { buildAdjLsa(); });
  }
}

bool
Lsdb::addAdjLsa(AdjLsa& alsa)
{
  auto it = std::find_if(m_adjLsdb.begin(), m_adjLsdb.end(),
                         std::bind(adjLsaCompareByKey, _1, alsa.getKey()));
  if (it == m_adjLsdb.end()) {
    m_adjLsdb.push_back(alsa);
    // Add any new name prefixes to the NPT
    // Only add NPT entries if this is an adj LSA from another router.
    if (alsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
      // Pass the originating router as both the name to register and
      // where it came from.
      m_namePrefixTable.addEntry(alsa.getOriginRouter(), alsa.getOriginRouter());
    }
    return true;
  }
  return false;
}

AdjLsa*
Lsdb::findAdjLsa(const ndn::Name& key)
{
  auto it = std::find_if(m_adjLsdb.begin(), m_adjLsdb.end(),
                         std::bind(adjLsaCompareByKey, _1, key));
  if (it != m_adjLsdb.end()) {
    return &*it;
  }
  return nullptr;
}

bool
Lsdb::isAdjLsaNew(const ndn::Name& key, uint64_t seqNo)
{
  AdjLsa* adjLsaCheck = findAdjLsa(key);
  // If it is in the LSDB
  if (adjLsaCheck != nullptr) {
    // And the supplied seq no is newer (higher) than the current one.
    if (adjLsaCheck->getSeqNo() < seqNo) {
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

ndn::scheduler::EventId
Lsdb::scheduleAdjLsaExpiration(const ndn::Name& key, int seqNo,
                               const ndn::time::seconds& expTime)
{
  return m_scheduler.schedule(expTime + GRACE_PERIOD,
                              std::bind(&Lsdb::expireOrRefreshAdjLsa, this, key, seqNo));
}

bool
Lsdb::installAdjLsa(AdjLsa& alsa)
{
  ndn::time::seconds timeToExpire = m_lsaRefreshTime;
  AdjLsa* chkAdjLsa = findAdjLsa(alsa.getKey());
  // If this adj. LSA is not in the LSDB already
  if (chkAdjLsa == nullptr) {
    NLSR_LOG_DEBUG("New Adj LSA. Adding to LSDB");
    NLSR_LOG_DEBUG("Adding Adj Lsa");
    NLSR_LOG_DEBUG(alsa);
    addAdjLsa(alsa);

    m_routingTable.scheduleRoutingTableCalculation();
    if (alsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
      ndn::time::system_clock::Duration duration = alsa.getExpirationTimePoint() -
                                                   ndn::time::system_clock::now();
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }
    scheduleAdjLsaExpiration(alsa.getKey(), alsa.getSeqNo(), timeToExpire);
  }
  else {
    if (chkAdjLsa->getSeqNo() < alsa.getSeqNo()) {
      NLSR_LOG_DEBUG("Updated Adj LSA. Updating LSDB");
      NLSR_LOG_DEBUG("Deleting Adj Lsa");
      NLSR_LOG_DEBUG(chkAdjLsa);
      chkAdjLsa->setSeqNo(alsa.getSeqNo());
      chkAdjLsa->setExpirationTimePoint(alsa.getExpirationTimePoint());
      // If the new adj LSA has new content, update the contents of
      // the LSDB entry. Additionally, since we've changed the
      // contents of the LSDB, we have to schedule a routing
      // calculation.
      if (!chkAdjLsa->isEqualContent(alsa)) {
        chkAdjLsa->resetAdl();
        for (const auto& adjacent : alsa.getAdl()) {
          chkAdjLsa->addAdjacent(adjacent);
        }
        m_routingTable.scheduleRoutingTableCalculation();
      }
      if (alsa.getOriginRouter() != m_confParam.getRouterPrefix()) {
        auto duration = alsa.getExpirationTimePoint() - ndn::time::system_clock::now();
        timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
      }
      chkAdjLsa->getExpiringEventId().cancel();
      chkAdjLsa->setExpiringEventId(scheduleAdjLsaExpiration(alsa.getKey(),
                                                             alsa.getSeqNo(),
                                                             timeToExpire));
      NLSR_LOG_DEBUG("Adding Adj Lsa");
      NLSR_LOG_DEBUG(chkAdjLsa);
    }
  }
  return true;
}

bool
Lsdb::buildAndInstallOwnAdjLsa()
{
  AdjLsa adjLsa(m_confParam.getRouterPrefix(),
                m_sequencingManager.getAdjLsaSeq() + 1,
                getLsaExpirationTimePoint(),
                m_confParam.getAdjacencyList().getNumOfActiveNeighbor(),
                m_confParam.getAdjacencyList());

  //Sync adjacency LSAs if link-state or dry-run HR is enabled.
  if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_ON) {
    m_sequencingManager.increaseAdjLsaSeq();
    m_sequencingManager.writeSeqNoToFile();
    m_sync.publishRoutingUpdate(Lsa::Type::ADJACENCY, m_sequencingManager.getAdjLsaSeq());
  }

  return installAdjLsa(adjLsa);
}

bool
Lsdb::removeAdjLsa(const ndn::Name& key)
{
  auto it = std::find_if(m_adjLsdb.begin(), m_adjLsdb.end(),
                         std::bind(adjLsaCompareByKey, _1, key));
  if (it != m_adjLsdb.end()) {
    NLSR_LOG_DEBUG("Deleting Adj Lsa");
    NLSR_LOG_DEBUG(*it);
    if (it->getOriginRouter() != m_confParam.getRouterPrefix()) {
      m_namePrefixTable.removeEntry(it->getOriginRouter(), it->getOriginRouter());
    }
    m_adjLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesAdjLsaExist(const ndn::Name& key)
{
  auto it = std::find_if(m_adjLsdb.begin(), m_adjLsdb.end(),
                         std::bind(adjLsaCompareByKey, _1, key));
  return it != m_adjLsdb.end();
}

const std::list<AdjLsa>&
Lsdb::getAdjLsdb() const
{
  return m_adjLsdb;
}

  // This function determines whether a name LSA should be refreshed
  // or expired. The conditions for getting refreshed are: it is still
  // in the LSDB, it hasn't been updated by something else already (as
  // evidenced by its seq. no.), and this is the originating router for
  // the LSA. Is it let expire in all other cases.
  // lsaKey is the key of the LSA's publishing router.
  // seqNo is the seq. no. of the candidate LSA.
void
Lsdb::expireOrRefreshNameLsa(const ndn::Name& lsaKey, uint64_t seqNo)
{
  NLSR_LOG_DEBUG("Lsdb::expireOrRefreshNameLsa Called");
  NLSR_LOG_DEBUG("LSA Key : " << lsaKey << " Seq No: " << seqNo);
  NameLsa* chkNameLsa = findNameLsa(lsaKey);
  // If this name LSA exists in the LSDB
  if (chkNameLsa != nullptr) {
    NLSR_LOG_DEBUG("LSA Exists with seq no: " << chkNameLsa->getSeqNo());
    // If its seq no is the one we are expecting.
    if (chkNameLsa->getSeqNo() == seqNo) {
      if (chkNameLsa->getOriginRouter() == m_thisRouterPrefix) {
        NLSR_LOG_DEBUG("Own Name LSA, so refreshing it");
        NLSR_LOG_DEBUG("Deleting Name Lsa");
        NLSR_LOG_DEBUG(chkNameLsa);
        chkNameLsa->setSeqNo(chkNameLsa->getSeqNo() + 1);
        m_sequencingManager.setNameLsaSeq(chkNameLsa->getSeqNo());
        chkNameLsa->setExpirationTimePoint(getLsaExpirationTimePoint());
        NLSR_LOG_DEBUG("Adding Name Lsa");
        NLSR_LOG_DEBUG(chkNameLsa);
        // schedule refreshing event again
        chkNameLsa->setExpiringEventId(scheduleNameLsaExpiration(chkNameLsa->getKey(),
                                                                 chkNameLsa->getSeqNo(),
                                                                 m_lsaRefreshTime));
        m_sequencingManager.writeSeqNoToFile();
        m_sync.publishRoutingUpdate(Lsa::Type::NAME, m_sequencingManager.getNameLsaSeq());
      }
      // Since we cannot refresh other router's LSAs, our only choice is to expire.
      else {
        NLSR_LOG_DEBUG("Other's Name LSA, so removing from LSDB");
        removeNameLsa(lsaKey);
      }
    }
  }
}

  // This function determines whether an adj. LSA should be refreshed
  // or expired. The conditions for getting refreshed are: it is still
  // in the LSDB, it hasn't been updated by something else already (as
  // evidenced by its seq. no.), and this is the originating router for
  // the LSA. Is it let expire in all other cases.
  // lsaKey is the key of the LSA's publishing router.
  // seqNo is the seq. no. of the candidate LSA.
void
Lsdb::expireOrRefreshAdjLsa(const ndn::Name& lsaKey, uint64_t seqNo)
{
  NLSR_LOG_DEBUG("Lsdb::expireOrRefreshAdjLsa Called");
  NLSR_LOG_DEBUG("LSA Key: " << lsaKey << " Seq No: " << seqNo);
  AdjLsa* chkAdjLsa = findAdjLsa(lsaKey);
  // If this is a valid LSA
  if (chkAdjLsa != nullptr) {
    NLSR_LOG_DEBUG("LSA Exists with seq no: " << chkAdjLsa->getSeqNo());
    // And if it hasn't been updated for some other reason
    if (chkAdjLsa->getSeqNo() == seqNo) {
      // If it is our own LSA
      if (chkAdjLsa->getOriginRouter() == m_thisRouterPrefix) {
        NLSR_LOG_DEBUG("Own Adj LSA, so refreshing it");
        NLSR_LOG_DEBUG("Deleting Adj Lsa");
        NLSR_LOG_DEBUG(chkAdjLsa);
        chkAdjLsa->setSeqNo(chkAdjLsa->getSeqNo() + 1);
        m_sequencingManager.setAdjLsaSeq(chkAdjLsa->getSeqNo());
        chkAdjLsa->setExpirationTimePoint(getLsaExpirationTimePoint());
        NLSR_LOG_DEBUG("Adding Adj Lsa");
        NLSR_LOG_DEBUG(chkAdjLsa);
        // schedule refreshing event again
        chkAdjLsa->setExpiringEventId(scheduleAdjLsaExpiration(chkAdjLsa->getKey(),
                                                               chkAdjLsa->getSeqNo(),
                                                               m_lsaRefreshTime));
        m_sequencingManager.writeSeqNoToFile();
        m_sync.publishRoutingUpdate(Lsa::Type::ADJACENCY, m_sequencingManager.getAdjLsaSeq());
      }
      // An LSA from another router is expiring
      else {
        NLSR_LOG_DEBUG("Other's Adj LSA, so removing from LSDB");
        removeAdjLsa(lsaKey);
      }
      // We have changed the contents of the LSDB, so we have to
      // schedule a routing calculation
      m_routingTable.scheduleRoutingTableCalculation();
    }
  }
}

  // This function determines whether an adj. LSA should be refreshed
  // or expired. The conditions for getting refreshed are: it is still
  // in the LSDB, it hasn't been updated by something else already (as
  // evidenced by its seq. no.), and this is the originating router for
  // the LSA. It is let expire in all other cases.
  // lsaKey is the key of the LSA's publishing router.
  // seqNo is the seq. no. of the candidate LSA.
void
Lsdb::expireOrRefreshCoordinateLsa(const ndn::Name& lsaKey,
                                    uint64_t seqNo)
{
  NLSR_LOG_DEBUG("Lsdb::expireOrRefreshCorLsa Called ");
  NLSR_LOG_DEBUG("LSA Key : " << lsaKey << " Seq No: " << seqNo);
  CoordinateLsa* chkCorLsa = findCoordinateLsa(lsaKey);
  // Whether the LSA is in the LSDB or not.
  if (chkCorLsa != nullptr) {
    NLSR_LOG_DEBUG("LSA Exists with seq no: " << chkCorLsa->getSeqNo());
    // Whether the LSA has been updated without our knowledge.
    if (chkCorLsa->getSeqNo() == seqNo) {
      if (chkCorLsa->getOriginRouter() == m_thisRouterPrefix) {
        NLSR_LOG_DEBUG("Own Cor LSA, so refreshing it");
        NLSR_LOG_DEBUG("Deleting Coordinate Lsa");
        NLSR_LOG_DEBUG(chkCorLsa);
        chkCorLsa->setSeqNo(chkCorLsa->getSeqNo() + 1);
        if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
          m_sequencingManager.setCorLsaSeq(chkCorLsa->getSeqNo());
        }

        chkCorLsa->setExpirationTimePoint(getLsaExpirationTimePoint());
        NLSR_LOG_DEBUG("Adding Coordinate Lsa");
        NLSR_LOG_DEBUG(chkCorLsa);
        // schedule refreshing event again
        chkCorLsa->setExpiringEventId(scheduleCoordinateLsaExpiration(
                                        chkCorLsa->getKey(),
                                        chkCorLsa->getSeqNo(),
                                        m_lsaRefreshTime));
        // Only sync coordinate LSAs if link-state routing is disabled
        if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
          m_sequencingManager.writeSeqNoToFile();
          m_sync.publishRoutingUpdate(Lsa::Type::COORDINATE, m_sequencingManager.getCorLsaSeq());
        }
      }
      // We can't refresh other router's LSAs, so we remove it.
      else {
        NLSR_LOG_DEBUG("Other's Cor LSA, so removing from LSDB");
        removeCoordinateLsa(lsaKey);
      }
      if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
        m_routingTable.scheduleRoutingTableCalculation();
      }
    }
  }
}

void
Lsdb::expressInterest(const ndn::Name& interestName, uint32_t timeoutCount,
                      ndn::time::steady_clock::TimePoint deadline)
{
  // increment SENT_LSA_INTEREST
  lsaIncrementSignal(Statistics::PacketType::SENT_LSA_INTEREST);

  if (deadline == DEFAULT_LSA_RETRIEVAL_DEADLINE) {
    deadline = ndn::time::steady_clock::now() + ndn::time::seconds(static_cast<int>(LSA_REFRESH_TIME_MAX));
  }
  // The first component of the interest is the name.
  ndn::Name lsaName = interestName.getSubName(0, interestName.size()-1);
  // The seq no is the last
  uint64_t seqNo = interestName[-1].toNumber();

  // If the LSA is not found in the list currently.
  if (m_highestSeqNo.find(lsaName) == m_highestSeqNo.end()) {
    m_highestSeqNo[lsaName] = seqNo;
  }
  // If the new seq no is higher, that means the LSA is valid
  else if (seqNo > m_highestSeqNo[lsaName]) {
    m_highestSeqNo[lsaName] = seqNo;
  }
  // Otherwise, its an old/invalid LSA
  else if (seqNo < m_highestSeqNo[lsaName]) {
    return;
  }

  ndn::Interest interest(interestName);
  ndn::util::SegmentFetcher::Options options;
  options.interestLifetime = m_confParam.getLsaInterestLifetime();

  NLSR_LOG_DEBUG("Fetching Data for LSA: " << interestName << " Seq number: " << seqNo);
  auto fetcher = ndn::util::SegmentFetcher::start(m_face, interest,
                                                  m_confParam.getValidator(), options);

  auto it = m_fetchers.insert(fetcher).first;

  fetcher->afterSegmentValidated.connect([this] (const ndn::Data& data) {
    // Nlsr class subscribes to this to fetch certificates
    afterSegmentValidatedSignal(data);

    // If we don't do this IMS throws: std::bad_weak_ptr: bad_weak_ptr
    auto lsaSegment = std::make_shared<const ndn::Data>(data);
    m_lsaStorage.insert(*lsaSegment);
    const ndn::Name& segmentName = lsaSegment->getName();
    // Schedule deletion of the segment
    m_scheduler.schedule(ndn::time::seconds(LSA_REFRESH_TIME_DEFAULT),
                         [this, segmentName] { m_lsaStorage.erase(segmentName); });
  });

  fetcher->onComplete.connect([=] (const ndn::ConstBufferPtr& bufferPtr) {
    m_lsaStorage.erase(ndn::Name(lsaName).appendNumber(seqNo - 1));
    afterFetchLsa(bufferPtr, interestName);
    m_fetchers.erase(it);
  });

  fetcher->onError.connect([=] (uint32_t errorCode, const std::string& msg) {
    onFetchLsaError(errorCode, msg, interestName, timeoutCount, deadline, lsaName, seqNo);
    m_fetchers.erase(it);
  });

  // increment a specific SENT_LSA_INTEREST
  Lsa::Type lsaType;
  std::istringstream(interestName[-2].toUri()) >> lsaType;
  switch (lsaType) {
  case Lsa::Type::ADJACENCY:
    lsaIncrementSignal(Statistics::PacketType::SENT_ADJ_LSA_INTEREST);
    break;
  case Lsa::Type::COORDINATE:
    lsaIncrementSignal(Statistics::PacketType::SENT_COORD_LSA_INTEREST);
    break;
  case Lsa::Type::NAME:
    lsaIncrementSignal(Statistics::PacketType::SENT_NAME_LSA_INTEREST);
    break;
  default:
    NLSR_LOG_ERROR("lsaType " << lsaType << " not recognized; failed Statistics::PacketType conversion");
  }
}

void
Lsdb::processInterest(const ndn::Name& name, const ndn::Interest& interest)
{
  ndn::Name interestName(interest.getName());
  NLSR_LOG_DEBUG("Interest received for LSA: " << interestName);

  if (interestName[-2].isVersion()) {
    // Interest for particular segment
    if (m_segmentPublisher.replyFromStore(interestName)) {
      NLSR_LOG_TRACE("Reply from SegmentPublisher storage");
      return;
    }
    // Remove version and segment
    interestName = interestName.getSubName(0, interestName.size() - 2);
    NLSR_LOG_TRACE("Interest w/o segment and version: " << interestName);
  }

  // increment RCV_LSA_INTEREST
  lsaIncrementSignal(Statistics::PacketType::RCV_LSA_INTEREST);

  std::string chkString("LSA");
  int32_t lsaPosition = util::getNameComponentPosition(interestName, chkString);

  // Forms the name of the router that the Interest packet came from.
  ndn::Name originRouter = m_confParam.getNetwork();
  originRouter.append(interestName.getSubName(lsaPosition + 1,
                                              interestName.size() - lsaPosition - 3));

  // if the interest is for this router's LSA
  if (originRouter == m_confParam.getRouterPrefix() && lsaPosition >= 0) {
    uint64_t seqNo = interestName[-1].toNumber();
    NLSR_LOG_DEBUG("LSA sequence number from interest: " << seqNo);

    std::string lsaType = interestName[-2].toUri();
    Lsa::Type interestedLsType;
    std::istringstream(lsaType) >> interestedLsType;

    if (interestedLsType == Lsa::Type::NAME) {
      processInterestForNameLsa(interest, originRouter.append(lsaType), seqNo);
    }
    else if (interestedLsType == Lsa::Type::ADJACENCY) {
      processInterestForAdjacencyLsa(interest, originRouter.append(lsaType), seqNo);
    }
    else if (interestedLsType == Lsa::Type::COORDINATE) {
      processInterestForCoordinateLsa(interest, originRouter.append(lsaType), seqNo);
    }
    else {
      NLSR_LOG_WARN("Received unrecognized LSA type: " << interestedLsType);
    }
    lsaIncrementSignal(Statistics::PacketType::SENT_LSA_DATA);
  }
  else { // else the interest is for other router's lsa, serve from LsaSegmentStorage
    std::shared_ptr<const ndn::Data> lsaSegment = m_lsaStorage.find(interest);
    if (lsaSegment) {
      NLSR_LOG_TRACE("Found data in lsa storage. Sending the data for " << interest.getName());
      m_face.put(*lsaSegment);
    }
    else {
      NLSR_LOG_TRACE(interest << " was not found in this lsa storage.");
    }
  }
}

  // \brief Finds and sends a requested name LSA.
  // \param interest The interest that seeks the name LSA.
  // \param lsaKey The LSA that the Interest is seeking.
  // \param seqNo A sequence number to ensure that we are sending the
  // version that was requested.
void
Lsdb::processInterestForNameLsa(const ndn::Interest& interest,
                                const ndn::Name& lsaKey,
                                uint64_t seqNo)
{
  // increment RCV_NAME_LSA_INTEREST
  lsaIncrementSignal(Statistics::PacketType::RCV_NAME_LSA_INTEREST);
  NLSR_LOG_DEBUG("nameLsa interest " << interest << " received");
  NameLsa* nameLsa = findNameLsa(lsaKey);
  if (nameLsa != nullptr) {
    NLSR_LOG_TRACE("Verifying SeqNo for NameLsa is same as requested.");
    if (nameLsa->getSeqNo() == seqNo) {
      m_segmentPublisher.publish(interest.getName(), interest.getName(),
                                 nameLsa->wireEncode(),
                                 m_lsaRefreshTime, m_confParam.getSigningInfo());

      lsaIncrementSignal(Statistics::PacketType::SENT_NAME_LSA_DATA);
    }
    else {
      NLSR_LOG_TRACE("SeqNo for nameLsa does not match");
    }
  }
  else {
    NLSR_LOG_TRACE(interest << "  was not found in this lsdb");
  }
}

  // \brief Finds and sends a requested adj. LSA.
  // \param interest The interest that seeks the adj. LSA.
  // \param lsaKey The LSA that the Interest is seeking.
  // \param seqNo A sequence number to ensure that we are sending the
  // version that was requested.
void
Lsdb::processInterestForAdjacencyLsa(const ndn::Interest& interest,
                                     const ndn::Name& lsaKey,
                                     uint64_t seqNo)
{
  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
    NLSR_LOG_ERROR("Received interest for an adjacency LSA when hyperbolic routing is enabled");
  }

  lsaIncrementSignal(Statistics::PacketType::RCV_ADJ_LSA_INTEREST);
  NLSR_LOG_DEBUG("AdjLsa interest " << interest << " received");
  AdjLsa* adjLsa = findAdjLsa(lsaKey);
  if (adjLsa != nullptr) {
    NLSR_LOG_TRACE("Verifying SeqNo for AdjLsa is same as requested.");
    if (adjLsa->getSeqNo() == seqNo) {
      m_segmentPublisher.publish(interest.getName(), interest.getName(),
                                 adjLsa->wireEncode(),
                                 m_lsaRefreshTime, m_confParam.getSigningInfo());

      lsaIncrementSignal(Statistics::PacketType::SENT_ADJ_LSA_DATA);
    }
    else {
      NLSR_LOG_TRACE("SeqNo for AdjLsa does not match");
    }
  }
  else {
    NLSR_LOG_TRACE(interest << "  was not found in this lsdb");
  }
}

  // \brief Finds and sends a requested cor. LSA.
  // \param interest The interest that seeks the cor. LSA.
  // \param lsaKey The LSA that the Interest is seeking.
  // \param seqNo A sequence number to ensure that we are sending the
  // version that was requested.
void
Lsdb::processInterestForCoordinateLsa(const ndn::Interest& interest,
                                      const ndn::Name& lsaKey,
                                      uint64_t seqNo)
{
  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_OFF) {
    NLSR_LOG_ERROR("Received Interest for a coordinate LSA when link-state routing is enabled");
  }

  lsaIncrementSignal(Statistics::PacketType::RCV_COORD_LSA_INTEREST);
  NLSR_LOG_DEBUG("CoordinateLsa interest " << interest << " received");
  CoordinateLsa* corLsa = findCoordinateLsa(lsaKey);
  if (corLsa != nullptr) {
    NLSR_LOG_TRACE("Verifying SeqNo for CoordinateLsa is same as requested.");
    if (corLsa->getSeqNo() == seqNo) {
      m_segmentPublisher.publish(interest.getName(), interest.getName(),
                                 corLsa->wireEncode(),
                                 m_lsaRefreshTime, m_confParam.getSigningInfo());

      lsaIncrementSignal(Statistics::PacketType::SENT_COORD_LSA_DATA);
    }
    else {
      NLSR_LOG_TRACE("SeqNo for CoordinateLsa does not match");
    }
  }
  else {
    NLSR_LOG_TRACE(interest << "  was not found in this lsdb");
  }
}

void
Lsdb::processContentNameLsa(const ndn::Name& lsaKey,
                            uint64_t lsSeqNo, const ndn::Block& block)
{
  lsaIncrementSignal(Statistics::PacketType::RCV_NAME_LSA_DATA);
  if (isNameLsaNew(lsaKey, lsSeqNo)) {
    NameLsa nameLsa(block);
    installNameLsa(nameLsa);
  }
}

void
Lsdb::processContentAdjacencyLsa(const ndn::Name& lsaKey,
                                 uint64_t lsSeqNo, const ndn::Block& block)
{
  lsaIncrementSignal(Statistics::PacketType::RCV_ADJ_LSA_DATA);
  if (isAdjLsaNew(lsaKey, lsSeqNo)) {
    AdjLsa adjLsa(block);
    installAdjLsa(adjLsa);
  }
}

void
Lsdb::processContentCoordinateLsa(const ndn::Name& lsaKey,
                                  uint64_t lsSeqNo, const ndn::Block& block)
{
  lsaIncrementSignal(Statistics::PacketType::RCV_COORD_LSA_DATA);
  if (isCoordinateLsaNew(lsaKey, lsSeqNo)) {
    CoordinateLsa corLsa(block);
    installCoordinateLsa(corLsa);
  }
}

ndn::time::system_clock::TimePoint
Lsdb::getLsaExpirationTimePoint()
{
  ndn::time::system_clock::TimePoint expirationTimePoint = ndn::time::system_clock::now();
  expirationTimePoint = expirationTimePoint +
                        ndn::time::seconds(m_confParam.getRouterDeadInterval());
  return expirationTimePoint;
}

void
Lsdb::writeAdjLsdbLog()
{
  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
    return;
  }

  NLSR_LOG_DEBUG("---------------Adj LSDB-------------------");
  for (const auto& adj : m_adjLsdb) {
    NLSR_LOG_DEBUG(adj);
  }
}

//-----utility function -----
bool
Lsdb::doesLsaExist(const ndn::Name& key, const Lsa::Type& lsType)
{
  switch (lsType) {
  case Lsa::Type::ADJACENCY:
    return doesAdjLsaExist(key);

  case Lsa::Type::COORDINATE:
    return doesCoordinateLsaExist(key);

  case Lsa::Type::NAME:
    return doesNameLsaExist(key);

  default:
    return false;
  }
}

bool
Lsdb::isLsaNew(const ndn::Name& routerName, const Lsa::Type& lsaType,
               const uint64_t& sequenceNumber) {
  ndn::Name lsaKey = routerName;
  lsaKey.append(boost::lexical_cast<std::string>(lsaType));

  switch (lsaType) {
  case Lsa::Type::ADJACENCY:
    return isAdjLsaNew(lsaKey, sequenceNumber);

  case Lsa::Type::COORDINATE:
    return isCoordinateLsaNew(lsaKey, sequenceNumber);

  case Lsa::Type::NAME:
    return isNameLsaNew(lsaKey, sequenceNumber);

  default:
    return false;
  }
}

} // namespace nlsr
