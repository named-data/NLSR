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

#ifndef NLSR_PUBLISHER_LSDB_STATUS_PUBLISHER_HPP
#define NLSR_PUBLISHER_LSDB_STATUS_PUBLISHER_HPP

#include "lsa-publisher.hpp"
#include "lsdb.hpp"
#include "segment-publisher.hpp"

#include <ndn-cxx/face.hpp>

namespace nlsr {

/**
 * @brief Abstraction to publish lsdb status dataset
 * \sa http://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class LsdbStatusPublisher : public SegmentPublisher<ndn::Face>
{
public:
  LsdbStatusPublisher(Lsdb& lsdb,
                      ndn::Face& face,
                      ndn::KeyChain& keyChain,
                      AdjacencyLsaPublisher& adjacencyLsaPublisher,
                      CoordinateLsaPublisher& coordinateLsaPublisher,
                      NameLsaPublisher& nameLsaPublisher);

protected:
  virtual size_t
  generate(ndn::EncodingBuffer& outBuffer);

private:
  AdjacencyLsaPublisher& m_adjacencyLsaPublisher;
  CoordinateLsaPublisher& m_coordinateLsaPublisher;
  NameLsaPublisher& m_nameLsaPublisher;

public:
  static const ndn::Name::Component DATASET_COMPONENT;
};

} // namespace nlsr

#endif // NLSR_PUBLISHER_LSDB_STATUS_PUBLISHER_HPP
