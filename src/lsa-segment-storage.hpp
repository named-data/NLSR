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

#ifndef NLSR_LSA_SEGMENT_STORAGE_HPP
#define NLSR_LSA_SEGMENT_STORAGE_HPP

#include "test-access-control.hpp"

#include <ndn-cxx/util/segment-fetcher.hpp>
#include <ndn-cxx/util/signal.hpp>
#include <ndn-cxx/util/time.hpp>

#include <vector>
#include <tuple>

namespace nlsr {

class LsaSegmentStorage
{
public:
  LsaSegmentStorage(ndn::Scheduler& scheduler,
                    const ndn::time::seconds lsaDeletionTimepoint);

  /*! \brief Get connected to the signal emitted by SegmentFetcher
   * \param fetcher The SegmentFetcher to whose signal LsaSegmentStorage will subscribe to.
   */
  void
  connectToFetcher(ndn::util::SegmentFetcher& fetcher);

  /*! \brief Returns an LSA segment for an interest from LsaSegmentStorage
   * \param interest Interest corresponding to the returned LSA segment.
   */
  const ndn::Data*
  getLsaSegment(const ndn::Interest& interest);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  /*! \brief Callback when SegmentFetcher retrieves a segment.
   */
   void
   afterFetcherSignalEmitted(const ndn::Data& lsaSegment);

private:
  /*! \brief Given an LSA name check whether Data for the same name exists in the
   * LsaSegmentStorage. If the matched LSA data are of a lower sequence number,
   * then remove them from LsaSegmentStorage.
   * \param newLsaName Name of the LSA that will be matched against
   */
  void
  deleteOldLsas(const ndn::Name& newLsaName);

  /*! \brief Schedules the deletion of a LSA data given the segmentKey
   */
  void
  scheduleLsaSegmentDeletion(const ndn::Name& segmentKey);


private:
  ndn::Scheduler& m_scheduler;

  // Key: /<router-prefix>/<LS type>/<sequence no.>/<segment no.>
  // Value: corresponding LSA data packet
  //        Data name: /<router-prefix>/<LS type>/<sequence no.>/<version no.>/<segment no.>
  std::unordered_map<ndn::Name, ndn::Data> m_lsaSegments;

  const ndn::time::seconds m_lsaDeletionTimepoint;
};

} // namespace nlsr

#endif // NLSR_LSA_SEGMENT_STORAGE_HPP
