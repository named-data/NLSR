/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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

INIT_LOGGER(SyncProtocolAdapter);

namespace nlsr {

const auto FIXED_SESSION = ndn::name::Component::fromNumber(0);

SyncProtocolAdapter::SyncProtocolAdapter(ndn::Face& face,
                                         SyncProtocol syncProtocol,
                                         const ndn::Name& syncPrefix,
                                         const ndn::Name& userPrefix,
                                         ndn::time::milliseconds syncInterestLifetime,
                                         const SyncUpdateCallback& syncUpdateCallback)
 : m_syncProtocol(syncProtocol)
 , m_syncUpdateCallback(syncUpdateCallback)
{
  NLSR_LOG_TRACE("SyncProtocol value: " << m_syncProtocol);

#ifdef HAVE_CHRONOSYNC
  if (m_syncProtocol == SYNC_PROTOCOL_CHRONOSYNC) {
    NDN_LOG_DEBUG("Using ChronoSync");
    m_chronoSyncLogic = std::make_shared<chronosync::Logic>(face,
                          syncPrefix,
                          userPrefix,
                          std::bind(&SyncProtocolAdapter::onChronoSyncUpdate, this, _1),
                          chronosync::Logic::DEFAULT_NAME,
                          chronosync::Logic::DEFAULT_VALIDATOR,
                          chronosync::Logic::DEFAULT_RESET_TIMER,
                          chronosync::Logic::DEFAULT_CANCEL_RESET_TIMER,
                          chronosync::Logic::DEFAULT_RESET_INTEREST_LIFETIME,
                          syncInterestLifetime,
                          chronosync::Logic::DEFAULT_SYNC_REPLY_FRESHNESS,
                          chronosync::Logic::DEFAULT_RECOVERY_INTEREST_LIFETIME,
                          FIXED_SESSION);
    return;
  }
#endif

  NDN_LOG_DEBUG("Using PSync");
  m_psyncLogic = std::make_shared<psync::FullProducer>(80,
                   face,
                   syncPrefix,
                   userPrefix,
                   std::bind(&SyncProtocolAdapter::onPSyncUpdate, this, _1),
                   syncInterestLifetime);
}

void
SyncProtocolAdapter::addUserNode(const ndn::Name& userPrefix)
{
#ifdef HAVE_CHRONOSYNC
  if (m_syncProtocol == SYNC_PROTOCOL_CHRONOSYNC) {
    m_chronoSyncLogic->addUserNode(userPrefix, chronosync::Logic::DEFAULT_NAME, FIXED_SESSION);
    return;
  }
#endif

  m_psyncLogic->addUserNode(userPrefix);
}

void
SyncProtocolAdapter::publishUpdate(const ndn::Name& userPrefix, uint64_t seq)
{
#ifdef HAVE_CHRONOSYNC
  if (m_syncProtocol == SYNC_PROTOCOL_CHRONOSYNC) {
    m_chronoSyncLogic->updateSeqNo(seq, userPrefix);
    return;
  }
#endif
  m_psyncLogic->publishName(userPrefix, seq);
}

#ifdef HAVE_CHRONOSYNC
void
SyncProtocolAdapter::onChronoSyncUpdate(const std::vector<chronosync::MissingDataInfo>& updates)
{
  NLSR_LOG_TRACE("Received ChronoSync update event");

  for (const auto& update : updates) {
    // Remove FIXED_SESSION
    m_syncUpdateCallback(update.session.getPrefix(-1), update.high);
  }
}
#endif

void
SyncProtocolAdapter::onPSyncUpdate(const std::vector<psync::MissingDataInfo>& updates)
{
  NLSR_LOG_TRACE("Received PSync update event");

  for (const auto& update : updates) {
    m_syncUpdateCallback(update.prefix, update.highSeq);
  }
}

} // namespace nlsr