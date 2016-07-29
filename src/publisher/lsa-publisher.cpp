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

#include "lsa-publisher.hpp"

#include "lsa.hpp"

#include <ndn-cxx/face.hpp>

namespace nlsr {

const ndn::Name::Component AdjacencyLsaPublisher::DATASET_COMPONENT =
  ndn::Name::Component("adjacencies");

AdjacencyLsaPublisher::AdjacencyLsaPublisher(Lsdb& lsdb,
                                             ndn::Face& face,
                                             ndn::KeyChain& keyChain)
  : LsaPublisher(face, keyChain)
  , m_adjacencyLsas(lsdb.getAdjLsdb())
{
}

std::list<tlv::AdjacencyLsa>
AdjacencyLsaPublisher::getTlvLsas()
{
  std::list<tlv::AdjacencyLsa> lsas;

  for (AdjLsa lsa : m_adjacencyLsas) {
    tlv::AdjacencyLsa tlvLsa;

    std::shared_ptr<tlv::LsaInfo> tlvLsaInfo = tlv::makeLsaInfo(lsa);
    tlvLsa.setLsaInfo(*tlvLsaInfo);

    for (const Adjacent& adj : lsa.getAdl().getAdjList()) {
      tlv::Adjacency tlvAdj;
      tlvAdj.setName(adj.getName());
      tlvAdj.setUri(adj.getConnectingFaceUri());
      tlvAdj.setCost(adj.getLinkCost());
      tlvLsa.addAdjacency(tlvAdj);
    }

    lsas.push_back(tlvLsa);
  }

  return lsas;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const ndn::Name::Component CoordinateLsaPublisher::DATASET_COMPONENT =
  ndn::Name::Component("coordinates");

CoordinateLsaPublisher::CoordinateLsaPublisher(Lsdb& lsdb,
                                               ndn::Face& face,
                                               ndn::KeyChain& keyChain)
  : LsaPublisher(face, keyChain)
  , m_coordinateLsas(lsdb.getCoordinateLsdb())
{
}

std::list<tlv::CoordinateLsa>
CoordinateLsaPublisher::getTlvLsas()
{
  std::list<tlv::CoordinateLsa> lsas;

  for (const CoordinateLsa lsa : m_coordinateLsas) {
    tlv::CoordinateLsa tlvLsa;

    std::shared_ptr<tlv::LsaInfo> tlvLsaInfo = tlv::makeLsaInfo(lsa);
    tlvLsa.setLsaInfo(*tlvLsaInfo);

    tlvLsa.setHyperbolicRadius(lsa.getCorRadius());
    tlvLsa.setHyperbolicAngle(lsa.getCorTheta());

    lsas.push_back(tlvLsa);
  }

  return lsas;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const ndn::Name::Component NameLsaPublisher::DATASET_COMPONENT =
  ndn::Name::Component("names");

NameLsaPublisher::NameLsaPublisher(Lsdb& lsdb,
                                   ndn::Face& face,
                                   ndn::KeyChain& keyChain)
  : LsaPublisher(face, keyChain)
  , m_nameLsas(lsdb.getNameLsdb())
{
}

std::list<tlv::NameLsa>
NameLsaPublisher::getTlvLsas()
{
  std::list<tlv::NameLsa> lsas;

  for (NameLsa lsa : m_nameLsas) {
    tlv::NameLsa tlvLsa;

    std::shared_ptr<tlv::LsaInfo> tlvLsaInfo = tlv::makeLsaInfo(lsa);
    tlvLsa.setLsaInfo(*tlvLsaInfo);

    for (const ndn::Name& name : lsa.getNpl().getNameList()) {
      tlvLsa.addName(name);
    }

    lsas.push_back(tlvLsa);
  }

  return lsas;
}

} // namespace nlsr
