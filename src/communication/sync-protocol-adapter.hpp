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

#ifndef NLSR_SYNC_PROTOCOL_ADAPTER_HPP
#define NLSR_SYNC_PROTOCOL_ADAPTER_HPP

#include "conf-parameter.hpp"

#include <ndn-cxx/face.hpp>
#ifdef HAVE_CHRONOSYNC
#include <ChronoSync/logic.hpp>
#endif
#include <PSync/full-producer.hpp>

namespace nlsr {

typedef std::function<void(const ndn::Name& updateName,
                           uint64_t seqNo)> SyncUpdateCallback;

class SyncProtocolAdapter
{
public:
  SyncProtocolAdapter(ndn::Face& facePtr,
                      SyncProtocol syncProtocol,
                      const ndn::Name& syncPrefix,
                      const ndn::Name& userPrefix,
                      ndn::time::milliseconds syncInterestLifetime,
                      const SyncUpdateCallback& syncUpdateCallback);

  /*! \brief Add user node to ChronoSync or PSync
   *
   * \param userPrefix the Name under which the application will publishData
   */
  void
  addUserNode(const ndn::Name& userPrefix);

  /*! \brief Publish update to ChronoSync or PSync
   *
   * NLSR forces sequences number on the sync protocol
   * as it manages is its own sequence number by storing it in a file.
   *
   * \param userPrefix the Name to be updated
   * \param seq the sequence of userPrefix
   */
  void
  publishUpdate(const ndn::Name& userPrefix, uint64_t seq);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
#ifdef HAVE_CHRONOSYNC
   /*! \brief Hook function to call whenever ChronoSync detects new data.
   *
   * This function packages the sync information into discrete updates
   * and passes those off to another function, m_syncUpdateCallback.
   * \sa m_syncUpdateCallback
   *
   * \param v A container with the new information sync has received
   */
  void
  onChronoSyncUpdate(const std::vector<chronosync::MissingDataInfo>& updates);
#endif

   /*! \brief Hook function to call whenever PSync detects new data.
   *
   * This function packages the sync information into discrete updates
   * and passes those off to another function, m_syncUpdateCallback.
   * \sa m_syncUpdateCallback
   *
   * \param v A container with the new information sync has received
   */
  void
  onPSyncUpdate(const std::vector<psync::MissingDataInfo>& updates);

private:
  SyncProtocol m_syncProtocol;
  SyncUpdateCallback m_syncUpdateCallback;
#ifdef HAVE_CHRONOSYNC
  std::shared_ptr<chronosync::Logic> m_chronoSyncLogic;
#endif
  std::shared_ptr<psync::FullProducer> m_psyncLogic;
};

} // namespace nlsr

#endif // NLSR_SYNC_PROTOCOL_ADAPTER_HPP
