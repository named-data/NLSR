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

#include "sync-protocol-adapter.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER(SyncProtocolAdapter);

#ifdef HAVE_CHRONOSYNC
const auto FIXED_SESSION = ndn::name::Component::fromNumber(0);
#endif

SyncProtocolAdapter::SyncProtocolAdapter(ndn::Face& face,
                                         ndn::KeyChain& keyChain,
                                         SyncProtocol syncProtocol,
                                         const ndn::Name& syncPrefix,
                                         const ndn::Name& userPrefix,
                                         ndn::time::milliseconds syncInterestLifetime,
                                         SyncUpdateCallback syncUpdateCallback)
  : m_syncProtocol(syncProtocol)
  , m_syncUpdateCallback(std::move(syncUpdateCallback))
{
  switch (m_syncProtocol) {
#ifdef HAVE_CHRONOSYNC
    case SyncProtocol::CHRONOSYNC:
      NDN_LOG_DEBUG("Using ChronoSync");
      m_chronoSyncLogic = std::make_shared<chronosync::Logic>(face,
                            syncPrefix,
                            userPrefix,
                            [this] (auto&&... args) { onChronoSyncUpdate(std::forward<decltype(args)>(args)...); },
                            chronosync::Logic::DEFAULT_NAME,
                            chronosync::Logic::DEFAULT_VALIDATOR,
                            chronosync::Logic::DEFAULT_RESET_TIMER,
                            chronosync::Logic::DEFAULT_CANCEL_RESET_TIMER,
                            chronosync::Logic::DEFAULT_RESET_INTEREST_LIFETIME,
                            syncInterestLifetime,
                            chronosync::Logic::DEFAULT_SYNC_REPLY_FRESHNESS,
                            chronosync::Logic::DEFAULT_RECOVERY_INTEREST_LIFETIME,
                            FIXED_SESSION);
      break;
#endif // HAVE_CHRONOSYNC
#ifdef HAVE_PSYNC
    case SyncProtocol::PSYNC: {
      NDN_LOG_DEBUG("Using PSync");
      psync::FullProducer::Options opts;
      opts.onUpdate = [this] (auto&&... args) { onPSyncUpdate(std::forward<decltype(args)>(args)...); };
      opts.syncInterestLifetime = syncInterestLifetime;
      m_psyncLogic = std::make_shared<psync::FullProducer>(face, keyChain, syncPrefix, opts);
      m_psyncLogic->addUserNode(userPrefix);
      break;
    }
#endif // HAVE_PSYNC
#ifdef HAVE_SVS
    case SyncProtocol::SVS:
      NDN_LOG_DEBUG("Using SVS");
      m_svsCore = std::make_shared<ndn::svs::SVSyncCore>(face,
                      syncPrefix,
                      [this] (auto&&... args) { onSvsUpdate(std::forward<decltype(args)>(args)...); });
      break;
#endif // HAVE_SVS
    default:
      NDN_CXX_UNREACHABLE;
  }
}

void
SyncProtocolAdapter::addUserNode(const ndn::Name& userPrefix)
{
  switch (m_syncProtocol) {
#ifdef HAVE_CHRONOSYNC
  case SyncProtocol::CHRONOSYNC:
    m_chronoSyncLogic->addUserNode(userPrefix, chronosync::Logic::DEFAULT_NAME, FIXED_SESSION);
    break;
#endif // HAVE_CHRONOSYNC
#ifdef HAVE_PSYNC
  case SyncProtocol::PSYNC:
    m_psyncLogic->addUserNode(userPrefix);
    break;
#endif // HAVE_PSYNC
#ifdef HAVE_SVS
  case SyncProtocol::SVS:
    break;
#endif // HAVE_SVS
  default:
    NDN_CXX_UNREACHABLE;
  }
}

void
SyncProtocolAdapter::publishUpdate(const ndn::Name& userPrefix, uint64_t seq)
{
  switch (m_syncProtocol) {
#ifdef HAVE_CHRONOSYNC
  case SyncProtocol::CHRONOSYNC:
    m_chronoSyncLogic->updateSeqNo(seq, userPrefix);
    break;
#endif // HAVE_CHRONOSYNC
#ifdef HAVE_PSYNC
  case SyncProtocol::PSYNC:
    m_psyncLogic->publishName(userPrefix, seq);
    break;
#endif // HAVE_PSYNC
#ifdef HAVE_SVS
  case SyncProtocol::SVS:
    m_svsCore->updateSeqNo(seq, userPrefix);
    break;
#endif // HAVE_SVS
  default:
    NDN_CXX_UNREACHABLE;
  }
}

#ifdef HAVE_CHRONOSYNC
void
SyncProtocolAdapter::onChronoSyncUpdate(const std::vector<chronosync::MissingDataInfo>& updates)
{
  NLSR_LOG_TRACE("Received ChronoSync update event");

  for (const auto& update : updates) {
    // Remove FIXED_SESSION
    m_syncUpdateCallback(update.session.getPrefix(-1), update.high, 0);
  }
}
#endif // HAVE_CHRONOSYNC

#ifdef HAVE_PSYNC
void
SyncProtocolAdapter::onPSyncUpdate(const std::vector<psync::MissingDataInfo>& updates)
{
  NLSR_LOG_TRACE("Received PSync update event");

  for (const auto& update : updates) {
    m_syncUpdateCallback(update.prefix, update.highSeq, update.incomingFace);
  }
}
#endif // HAVE_PSYNC

#ifdef HAVE_SVS
void
SyncProtocolAdapter::onSvsUpdate(const std::vector<ndn::svs::MissingDataInfo>& updates)
{
  NLSR_LOG_TRACE("Received SVS update event");

  for (const auto& update : updates) {
    m_syncUpdateCallback(update.nodeId, update.high, update.incomingFace);
  }
}
#endif // HAVE_SVS

} // namespace nlsr
