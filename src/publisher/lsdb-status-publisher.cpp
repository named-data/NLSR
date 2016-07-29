/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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

#include "lsdb-status-publisher.hpp"

#include "lsa.hpp"
#include "tlv/lsdb-status.hpp"

#include <ndn-cxx/face.hpp>

namespace nlsr {

const ndn::Name::Component LsdbStatusPublisher::DATASET_COMPONENT = ndn::Name::Component("list");

LsdbStatusPublisher::LsdbStatusPublisher(Lsdb& lsdb,
                                         ndn::Face& face,
                                         ndn::KeyChain& keyChain,
                                         AdjacencyLsaPublisher& adjacencyLsaPublisher,
                                         CoordinateLsaPublisher& coordinateLsaPublisher,
                                         NameLsaPublisher& nameLsaPublisher)
  : SegmentPublisher<ndn::Face>(face, keyChain)
  , m_adjacencyLsaPublisher(adjacencyLsaPublisher)
  , m_coordinateLsaPublisher(coordinateLsaPublisher)
  , m_nameLsaPublisher(nameLsaPublisher)
{
}

size_t
LsdbStatusPublisher::generate(ndn::EncodingBuffer& outBuffer)
{
  size_t totalLength = 0;

  tlv::LsdbStatus lsdbStatus;
  for (const tlv::AdjacencyLsa& tlvLsa : m_adjacencyLsaPublisher.getTlvLsas()) {
    lsdbStatus.addAdjacencyLsa(tlvLsa);
  }

  for (const tlv::CoordinateLsa& tlvLsa : m_coordinateLsaPublisher.getTlvLsas()) {
    lsdbStatus.addCoordinateLsa(tlvLsa);
  }

  for (const tlv::NameLsa& tlvLsa : m_nameLsaPublisher.getTlvLsas()) {
    lsdbStatus.addNameLsa(tlvLsa);
  }

  totalLength += lsdbStatus.wireEncode(outBuffer);

  return totalLength;
}

} // namespace nlsr
