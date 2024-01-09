/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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

#include "lsdb.hpp"

#include "logger.hpp"
#include "nlsr.hpp"
#include "utility/name-helper.hpp"

#include <ndn-cxx/lp/tags.hpp>

namespace nlsr {

INIT_LOGGER(Lsdb);

Lsdb::Lsdb(ndn::Face& face, ndn::KeyChain& keyChain, ConfParameter& confParam)
  : m_face(face)
  , m_scheduler(face.getIoContext())
  , m_confParam(confParam)
  , m_sync(m_face, keyChain,
      [this] (const auto& routerName, Lsa::Type lsaType, uint64_t seqNo, uint64_t) {
        return isLsaNew(routerName, lsaType, seqNo);
      },
      SyncLogicOptions{
        confParam.getSyncProtocol(),
        confParam.getSyncPrefix(),
        confParam.getSyncUserPrefix(),
        confParam.getSyncInterestLifetime(),
        confParam.getRouterPrefix(),
        confParam.getHyperbolicState()
      })
  , m_lsaRefreshTime(ndn::time::seconds(m_confParam.getLsaRefreshTime()))
  , m_adjLsaBuildInterval(m_confParam.getAdjLsaBuildInterval())
  , m_thisRouterPrefix(m_confParam.getRouterPrefix())
  , m_sequencingManager(m_confParam.getStateFileDir(), m_confParam.getHyperbolicState())
  , m_onNewLsaConnection(m_sync.onNewLsa.connect(
      [this] (const ndn::Name& updateName, uint64_t sequenceNumber,
              const ndn::Name& originRouter, uint64_t incomingFaceId) {
        ndn::Name lsaInterest{updateName};
        lsaInterest.appendNumber(sequenceNumber);
        expressInterest(lsaInterest, 0, incomingFaceId);
      }))
  , m_segmenter(keyChain, m_confParam.getSigningInfo())
  , m_segmentFifo(100)
  , m_isBuildAdjLsaScheduled(false)
  , m_adjBuildCount(0)
{
  ndn::Name name = m_confParam.getLsaPrefix();
  NLSR_LOG_DEBUG("Setting interest filter for LsaPrefix: " << name);

  m_face.setInterestFilter(ndn::InterestFilter(name).allowLoopback(false),
    [this] (const auto& name, const auto& interest) { processInterest(name, interest); },
    [] (const auto& name) { NLSR_LOG_DEBUG("Successfully registered prefix: " << name); },
    [] (const auto& name, const auto& reason) {
      NLSR_LOG_ERROR("Failed to register prefix " << name);
      NDN_THROW(std::runtime_error("Register prefix failed: " + reason));
    },
    m_confParam.getSigningInfo(), ndn::nfd::ROUTE_FLAG_CAPTURE);

  buildAndInstallOwnNameLsa();
  // Install coordinate LSAs if using HR or dry-run HR.
  if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
    buildAndInstallOwnCoordinateLsa();
  }
}

Lsdb::~Lsdb()
{
  for (const auto& fetcher : m_fetchers) {
    fetcher->stop();
  }
}

void
Lsdb::buildAndInstallOwnNameLsa()
{
  NameLsa nameLsa(m_thisRouterPrefix, m_sequencingManager.getNameLsaSeq() + 1,
                  getLsaExpirationTimePoint(), m_confParam.getNamePrefixList());
  m_sequencingManager.increaseNameLsaSeq();
  m_sequencingManager.writeSeqNoToFile();
  m_sync.publishRoutingUpdate(Lsa::Type::NAME, m_sequencingManager.getNameLsaSeq());

  installLsa(std::make_shared<NameLsa>(nameLsa));
}

void
Lsdb::buildAndInstallOwnCoordinateLsa()
{
  CoordinateLsa corLsa(m_thisRouterPrefix, m_sequencingManager.getCorLsaSeq() + 1,
                       getLsaExpirationTimePoint(), m_confParam.getCorR(),
                       m_confParam.getCorTheta());
  m_sequencingManager.increaseCorLsaSeq();
  m_sequencingManager.writeSeqNoToFile();

  // Sync coordinate LSAs if using HR or HR dry run.
  if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
    m_sync.publishRoutingUpdate(Lsa::Type::COORDINATE, m_sequencingManager.getCorLsaSeq());
  }

  installLsa(std::make_shared<CoordinateLsa>(corLsa));
}

void
Lsdb::scheduleAdjLsaBuild()
{
  m_adjBuildCount++;

  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
    // Don't build adjacency LSAs in hyperbolic routing
    NLSR_LOG_DEBUG("Adjacency LSA not built while in hyperbolic routing state");
    return;
  }

  if (m_isBuildAdjLsaScheduled) {
    NLSR_LOG_DEBUG("Rescheduling Adjacency LSA build in " << m_adjLsaBuildInterval);
  }
  else {
    NLSR_LOG_DEBUG("Scheduling Adjacency LSA build in " << m_adjLsaBuildInterval);
    m_isBuildAdjLsaScheduled = true;
  }
  m_scheduledAdjLsaBuild = m_scheduler.schedule(m_adjLsaBuildInterval, [this] { buildAdjLsa(); });
}

void
Lsdb::writeLog() const
{
  for (auto type : {Lsa::Type::COORDINATE, Lsa::Type::NAME, Lsa::Type::ADJACENCY}) {
    if ((type == Lsa::Type::COORDINATE &&
         m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_OFF) ||
        (type == Lsa::Type::ADJACENCY &&
       m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON)) {
      continue;
    }

    NLSR_LOG_DEBUG("---------------" << type << " LSDB-------------------");
    auto lsaRange = m_lsdb.get<byType>().equal_range(type);
    for (auto lsaIt = lsaRange.first; lsaIt != lsaRange.second; ++lsaIt) {
      NLSR_LOG_DEBUG(**lsaIt);
    }
  }
}

void
Lsdb::processInterest(const ndn::Name& name, const ndn::Interest& interest)
{
  ndn::Name interestName(interest.getName());
  NLSR_LOG_DEBUG("Interest received for LSA: " << interestName);

  if (interestName[-2].isVersion()) {
    // Interest for particular segment
    auto data = m_segmentFifo.find(interestName);
    if (data) {
      NLSR_LOG_TRACE("Replying from FIFO buffer");
      m_face.put(*data);
      return;
    }

    // Remove version and segment
    interestName = interestName.getPrefix(-2);
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
  if (originRouter == m_thisRouterPrefix && lsaPosition >= 0) {
    uint64_t seqNo = interestName[-1].toNumber();
    NLSR_LOG_DEBUG("LSA sequence number from interest: " << seqNo);

    std::string lsaType = interestName[-2].toUri();
    Lsa::Type interestedLsType;
    std::istringstream(lsaType) >> interestedLsType;
    if (interestedLsType == Lsa::Type::BASE) {
      NLSR_LOG_WARN("Received unrecognized LSA type: " << lsaType);
      return;
    }

    incrementInterestRcvdStats(interestedLsType);
    if (processInterestForLsa(interest, originRouter, interestedLsType, seqNo)) {
      lsaIncrementSignal(Statistics::PacketType::SENT_LSA_DATA);
    }
  }
  // else the interest is for other router's LSA, serve signed data from LsaSegmentStorage
  else if (auto lsaSegment = m_lsaStorage.find(interest); lsaSegment) {
    NLSR_LOG_TRACE("Found data in LSA storage. Sending data for " << interest.getName());
    m_face.put(*lsaSegment);
  }
}

bool
Lsdb::processInterestForLsa(const ndn::Interest& interest, const ndn::Name& originRouter,
                            Lsa::Type lsaType, uint64_t seqNo)
{
  NLSR_LOG_DEBUG(interest << " received for " << lsaType);

  if (auto lsaPtr = findLsa(originRouter, lsaType); lsaPtr) {
    NLSR_LOG_TRACE("Verifying SeqNo for " << lsaType << " is same as requested");
    if (lsaPtr->getSeqNo() == seqNo) {
      auto segments = m_segmenter.segment(lsaPtr->wireEncode(),
                                          ndn::Name(interest.getName()).appendVersion(),
                                          ndn::MAX_NDN_PACKET_SIZE / 2, m_lsaRefreshTime);
      for (const auto& data : segments) {
        m_segmentFifo.insert(*data, m_lsaRefreshTime);
        m_scheduler.schedule(m_lsaRefreshTime,
                             [this, name = data->getName()] { m_segmentFifo.erase(name); });
      }

      uint64_t segNum = 0;
      if (interest.getName()[-1].isSegment()) {
        segNum = interest.getName()[-1].toSegment();
      }
      if (segNum < segments.size()) {
        m_face.put(*segments[segNum]);
      }
      incrementDataSentStats(lsaType);
      return true;
    }
  }
  else {
    NLSR_LOG_TRACE(interest << " was not found in our LSDB");
  }
  return false;
}

void
Lsdb::installLsa(std::shared_ptr<Lsa> lsa)
{
  auto timeToExpire = m_lsaRefreshTime;
  if (lsa->getOriginRouter() != m_thisRouterPrefix) {
    auto duration = lsa->getExpirationTimePoint() - ndn::time::system_clock::now();
    if (duration > ndn::time::seconds(0)) {
      timeToExpire = ndn::time::duration_cast<ndn::time::seconds>(duration);
    }
  }

  auto chkLsa = findLsa(lsa->getOriginRouter(), lsa->getType());
  if (chkLsa == nullptr) {
    NLSR_LOG_DEBUG("Adding LSA:\n" << *lsa);

    m_lsdb.emplace(lsa);
    onLsdbModified(lsa, LsdbUpdate::INSTALLED, {}, {});

    lsa->setExpiringEventId(scheduleLsaExpiration(lsa, timeToExpire));
  }
  // Else this is a known name LSA, so we are updating it.
  else if (chkLsa->getSeqNo() < lsa->getSeqNo()) {
    NLSR_LOG_DEBUG("Updating LSA:\n" << *chkLsa);
    chkLsa->setSeqNo(lsa->getSeqNo());
    chkLsa->setExpirationTimePoint(lsa->getExpirationTimePoint());

    auto [updated, namesToAdd, namesToRemove] = chkLsa->update(lsa);
    if (updated) {
      onLsdbModified(lsa, LsdbUpdate::UPDATED, namesToAdd, namesToRemove);
    }

    chkLsa->setExpiringEventId(scheduleLsaExpiration(chkLsa, timeToExpire));
    NLSR_LOG_DEBUG("Updated LSA:\n" << *chkLsa);
  }
}

void
Lsdb::removeLsa(const LsaContainer::index<Lsdb::byName>::type::iterator& lsaIt)
{
  if (lsaIt != m_lsdb.end()) {
    auto lsaPtr = *lsaIt;
    NLSR_LOG_DEBUG("Removing LSA:\n" << *lsaPtr);
    m_lsdb.erase(lsaIt);
    onLsdbModified(lsaPtr, LsdbUpdate::REMOVED, {}, {});
  }
}

void
Lsdb::removeLsa(const ndn::Name& router, Lsa::Type lsaType)
{
  removeLsa(m_lsdb.get<byName>().find(std::make_tuple(router, lsaType)));
}

void
Lsdb::buildAdjLsa()
{
  NLSR_LOG_TRACE("buildAdjLsa called");

  m_isBuildAdjLsaScheduled = false;

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
        removeLsa(m_thisRouterPrefix, Lsa::Type::ADJACENCY);
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
    m_isBuildAdjLsaScheduled = true;
    auto schedulingTime = ndn::time::seconds(m_confParam.getInterestRetryNumber() *
                                             m_confParam.getInterestResendTime());
    m_scheduledAdjLsaBuild = m_scheduler.schedule(schedulingTime, [this] { buildAdjLsa(); });
  }
}

void
Lsdb::buildAndInstallOwnAdjLsa()
{
  AdjLsa adjLsa(m_thisRouterPrefix, m_sequencingManager.getAdjLsaSeq() + 1,
                getLsaExpirationTimePoint(),
                m_confParam.getAdjacencyList());
  m_sequencingManager.increaseAdjLsaSeq();
  m_sequencingManager.writeSeqNoToFile();

  //Sync adjacency LSAs if link-state or dry-run HR is enabled.
  if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_ON) {
    m_sync.publishRoutingUpdate(Lsa::Type::ADJACENCY, m_sequencingManager.getAdjLsaSeq());
  }

  installLsa(std::make_shared<AdjLsa>(adjLsa));
}

ndn::scheduler::EventId
Lsdb::scheduleLsaExpiration(std::shared_ptr<Lsa> lsa, ndn::time::seconds expTime)
{
  NLSR_LOG_DEBUG("Scheduling expiration in: " << expTime + GRACE_PERIOD << " for " << lsa->getOriginRouter());
  return m_scheduler.schedule(expTime + GRACE_PERIOD, [this, lsa] { expireOrRefreshLsa(lsa); });
}

void
Lsdb::expireOrRefreshLsa(std::shared_ptr<Lsa> lsa)
{
  NLSR_LOG_DEBUG("ExpireOrRefreshLsa called for " << lsa->getType());
  NLSR_LOG_DEBUG("OriginRouter: " << lsa->getOriginRouter() << " Seq No: " << lsa->getSeqNo());

  auto lsaIt = m_lsdb.get<byName>().find(std::make_tuple(lsa->getOriginRouter(), lsa->getType()));

  // If this name LSA exists in the LSDB
  if (lsaIt != m_lsdb.end()) {
    auto lsaPtr = *lsaIt;
    NLSR_LOG_DEBUG(*lsaPtr);
    NLSR_LOG_DEBUG("LSA Exists with seq no: " << lsaPtr->getSeqNo());
    // If its seq no is the one we are expecting.
    if (lsaPtr->getSeqNo() == lsa->getSeqNo()) {
      if (lsaPtr->getOriginRouter() == m_thisRouterPrefix) {
        NLSR_LOG_DEBUG("Own " << lsaPtr->getType() << " LSA, so refreshing it");
        NLSR_LOG_DEBUG("Current LSA:\n" << *lsaPtr);
        lsaPtr->setSeqNo(lsaPtr->getSeqNo() + 1);
        m_sequencingManager.setLsaSeq(lsaPtr->getSeqNo(), lsaPtr->getType());
        lsaPtr->setExpirationTimePoint(getLsaExpirationTimePoint());
        NLSR_LOG_DEBUG("Updated LSA:\n" << *lsaPtr);
        // schedule refreshing event again
        lsaPtr->setExpiringEventId(scheduleLsaExpiration(lsaPtr, m_lsaRefreshTime));
        m_sequencingManager.writeSeqNoToFile();
        m_sync.publishRoutingUpdate(lsaPtr->getType(), m_sequencingManager.getLsaSeq(lsaPtr->getType()));
      }
      // Since we cannot refresh other router's LSAs, our only choice is to expire.
      else {
        NLSR_LOG_DEBUG("Other's " << lsaPtr->getType() << " LSA, so removing from LSDB");
        removeLsa(lsaIt);
      }
    }
  }
}

void
Lsdb::expressInterest(const ndn::Name& interestName, uint32_t timeoutCount, uint64_t incomingFaceId,
                      ndn::time::steady_clock::time_point deadline)
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
  if (incomingFaceId != 0) {
    interest.setTag(std::make_shared<ndn::lp::NextHopFaceIdTag>(incomingFaceId));
  }
  ndn::SegmentFetcher::Options options;
  options.interestLifetime = m_confParam.getLsaInterestLifetime();
  options.maxTimeout = m_confParam.getLsaInterestLifetime();

  NLSR_LOG_DEBUG("Fetching Data for LSA: " << interestName << " Seq number: " << seqNo);
  auto fetcher = ndn::SegmentFetcher::start(m_face, interest, m_confParam.getValidator(), options);

  auto it = m_fetchers.insert(fetcher).first;

  fetcher->afterSegmentValidated.connect([this] (const ndn::Data& data) {
    // Nlsr class subscribes to this to fetch certificates
    afterSegmentValidatedSignal(data);

    // If we don't do this IMS throws: std::bad_weak_ptr: bad_weak_ptr
    auto lsaSegment = std::make_shared<const ndn::Data>(data);
    m_lsaStorage.insert(*lsaSegment);
    // Schedule deletion of the segment
    m_scheduler.schedule(ndn::time::seconds(LSA_REFRESH_TIME_DEFAULT),
                         [this, name = lsaSegment->getName()] { m_lsaStorage.erase(name); });
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

  Lsa::Type lsaType;
  std::istringstream(interestName[-2].toUri()) >> lsaType;
  incrementInterestSentStats(lsaType);
}

void
Lsdb::onFetchLsaError(uint32_t errorCode, const std::string& msg, const ndn::Name& interestName,
                      uint32_t retransmitNo, const ndn::time::steady_clock::time_point& deadline,
                      ndn::Name lsaName, uint64_t seqNo)
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
      auto delay = m_confParam.getLsaInterestLifetime();
      if (errorCode == ndn::SegmentFetcher::ErrorCode::INTEREST_TIMEOUT) {
        delay = 0_s;
      }
      m_scheduler.schedule(delay, std::bind(&Lsdb::expressInterest, this, interestName,
                                            retransmitNo + 1, /*Multicast FaceID*/0, deadline));
    }
  }
}

void
Lsdb::afterFetchLsa(const ndn::ConstBufferPtr& bufferPtr, const ndn::Name& interestName)
{
  NLSR_LOG_DEBUG("Received data for LSA interest: " << interestName);
  lsaIncrementSignal(Statistics::PacketType::RCV_LSA_DATA);

  ndn::Name lsaName = interestName.getSubName(0, interestName.size()-1);
  uint64_t seqNo = interestName[-1].toNumber();

  if (m_highestSeqNo.find(lsaName) == m_highestSeqNo.end()) {
    m_highestSeqNo[lsaName] = seqNo;
  }
  else if (seqNo > m_highestSeqNo[lsaName]) {
    m_highestSeqNo[lsaName] = seqNo;
    NLSR_LOG_TRACE("SeqNo for LSA(name): " << interestName << " updated");
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
      Lsa::Type interestedLsType;
      std::istringstream(interestName[-2].toUri()) >> interestedLsType;

      if (interestedLsType == Lsa::Type::BASE) {
        NLSR_LOG_WARN("Received unrecognized LSA Type: " << interestName[-2].toUri());
        return;
      }

      ndn::Block block(bufferPtr);
      if (interestedLsType == Lsa::Type::NAME) {
        lsaIncrementSignal(Statistics::PacketType::RCV_NAME_LSA_DATA);
        if (isLsaNew(originRouter, interestedLsType, seqNo)) {
          installLsa(std::make_shared<NameLsa>(block));
        }
      }
      else if (interestedLsType == Lsa::Type::ADJACENCY) {
        lsaIncrementSignal(Statistics::PacketType::RCV_ADJ_LSA_DATA);
        if (isLsaNew(originRouter, interestedLsType, seqNo)) {
          installLsa(std::make_shared<AdjLsa>(block));
        }
      }
      else if (interestedLsType == Lsa::Type::COORDINATE) {
        lsaIncrementSignal(Statistics::PacketType::RCV_COORD_LSA_DATA);
        if (isLsaNew(originRouter, interestedLsType, seqNo)) {
          installLsa(std::make_shared<CoordinateLsa>(block));
        }
      }
    }
    catch (const std::exception& e) {
      NLSR_LOG_TRACE("LSA data decoding error: " << e.what());
    }
  }
}

} // namespace nlsr
