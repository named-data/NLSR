/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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

#include "lsa-segment-storage.hpp"
#include "logger.hpp"
#include "lsa.hpp"
#include "utility/name-helper.hpp"

namespace nlsr {

INIT_LOGGER(LsaSegmentStorage);

LsaSegmentStorage::LsaSegmentStorage(ndn::Scheduler& scheduler,
                                     const ndn::time::seconds lsaDeletionTimepoint)
  : m_scheduler(scheduler)
  , m_lsaDeletionTimepoint(lsaDeletionTimepoint)
{
}

void
LsaSegmentStorage::connectToFetcher(ndn::util::SegmentFetcher& fetcher)
{
  fetcher.afterSegmentValidated.connect(std::bind(&LsaSegmentStorage::afterFetcherSignalEmitted,
                                                  this, _1));
}

const ndn::Data*
LsaSegmentStorage::getLsaSegment(const ndn::Interest& interest)
{
  ndn::Name lsaSegmentsKey = interest.getName();

  // If this is the first interest then it does not contain the segment number,
  // so need to append zero segment component at the end to match with the data
  if (lsaSegmentsKey.size() > 0) {
    if (!lsaSegmentsKey.get(-1).isSegment()) {
      lsaSegmentsKey.appendSegment(0);
    }

    auto it = m_lsaSegments.find(lsaSegmentsKey);
    if (it == m_lsaSegments.end()) {
      NLSR_LOG_TRACE("Data for interest: " << interest.getName() << " cannot be found in the lsa storage");

      return nullptr;
    }
    else {
      NLSR_LOG_TRACE("Data for interest: " << interest.getName() << " is in the storage.");
      return &(it->second);
    }
  }
  else {
    NLSR_LOG_ERROR("Received interest has empty name.");
    return nullptr;
  }
}

void
LsaSegmentStorage::afterFetcherSignalEmitted(const ndn::Data& lsaSegment)
{
  NLSR_LOG_TRACE("Received a LSA segment: " << lsaSegment.getName());

  // lsaSegmentName is /<router-prefix>/<LS type>/<sequence no.>/<version no.>/<segment no.>
  auto lsaSegmentName = lsaSegment.getName();

  if (lsaSegmentName.size() > 0) {
    // lsaSegmentsKey is /<router-prefix>/<LS type>/<sequence no.>/<segment no.>
    ndn::Name lsaSegmentsKey(lsaSegmentName.getPrefix(lsaSegmentName.size() - 2));
    lsaSegmentsKey.append(lsaSegmentName.get(-1));

    // No need to store same LSA multiple time
    if (m_lsaSegments.find(lsaSegmentsKey) == m_lsaSegments.end()) {
      NLSR_LOG_TRACE("Received LSA segment is new. Storing it in the storage.\n"
                      << "                  LSA data name: " << lsaSegmentName);

      // Delete the same LSA with lower sequence number
      deleteOldLsas(lsaSegmentName);

      m_lsaSegments[lsaSegmentsKey] = lsaSegment;
    }
    else {
      NLSR_LOG_TRACE("The received segment is already in the storage.");
    }

    // schedule the segment deletion
    scheduleLsaSegmentDeletion(lsaSegmentsKey);
  }
  else {
    NLSR_LOG_ERROR("The received LSA segment has empty name.");
  }
}

void
LsaSegmentStorage::deleteOldLsas(const ndn::Name& newLsaName)
{
  auto newLsaKey = newLsaName.getPrefix(newLsaName.size() - 3);
  auto newSeqNo = newLsaName.get(-3).toNumber();

  std::vector<decltype(m_lsaSegments)::key_type> lsaToDelete;

  for (auto& segment : m_lsaSegments) {
    ndn::Name segmentKey = segment.first;
    auto oldSeqNo = segmentKey.get(-2).toNumber();
    auto existingLsaKey = segmentKey.getPrefix(segmentKey.size() - 2);

    if (newLsaKey == existingLsaKey) {
      if (newSeqNo > oldSeqNo) { // in the key the second last component is the sequence number
        NLSR_LOG_TRACE("Outdated LSA: " << segmentKey << " with seq no: " <<
                       oldSeqNo << " is deleted.");
        lsaToDelete.push_back(segmentKey);
      }
    }
  }

  for (auto& segmentKey : lsaToDelete) {
    m_lsaSegments.erase(segmentKey);
  }
}

void
LsaSegmentStorage::scheduleLsaSegmentDeletion(const ndn::Name& lsaSegmentsKey)
{
  m_scheduler.scheduleEvent(m_lsaDeletionTimepoint,
                            [&, this] {
                              m_lsaSegments.erase(lsaSegmentsKey);
                            });
}

} // namespace nlsr
