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

#ifndef NLSR_PUBLISHER_LSA_PUBLISHER_HPP
#define NLSR_PUBLISHER_LSA_PUBLISHER_HPP

#include "lsdb.hpp"
#include "segment-publisher.hpp"
#include "tlv/adjacency-lsa.hpp"
#include "tlv/coordinate-lsa.hpp"
#include "tlv/name-lsa.hpp"

#include <ndn-cxx/face.hpp>

namespace nlsr {

template <class TlvType>
class LsaPublisher : public SegmentPublisher<ndn::Face>
{
public:
  LsaPublisher(ndn::Face& face, ndn::KeyChain& keyChain)
  : SegmentPublisher<ndn::Face>(face, keyChain)
  {
  }

  virtual
  ~LsaPublisher()
  {
  }

protected:
  virtual size_t
  generate(ndn::EncodingBuffer& outBuffer)
  {
    size_t totalLength = 0;

    for (const TlvType& lsaTlv : getTlvLsas()) {
      totalLength += lsaTlv.wireEncode(outBuffer);
    }

    return totalLength;
  }

  virtual std::list<TlvType>
  getTlvLsas() = 0;
};

/**
 * @brief Abstraction to publish adjacency lsa dataset
 * \sa http://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class AdjacencyLsaPublisher : public LsaPublisher<tlv::AdjacencyLsa>
{
public:
  AdjacencyLsaPublisher(Lsdb& lsdb,
                        ndn::Face& face,
                        ndn::KeyChain& keyChain);

  std::list<tlv::AdjacencyLsa>
  getTlvLsas();

public:
  static const ndn::Name::Component DATASET_COMPONENT;

private:
  const std::list<AdjLsa>& m_adjacencyLsas;
};

/**
 * @brief Abstraction to publish coordinate lsa dataset
 * \sa http://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class CoordinateLsaPublisher : public LsaPublisher<tlv::CoordinateLsa>
{
public:
  CoordinateLsaPublisher(Lsdb& lsdb,
                        ndn::Face& face,
                        ndn::KeyChain& keyChain);

  std::list<tlv::CoordinateLsa>
  getTlvLsas();

public:
  static const ndn::Name::Component DATASET_COMPONENT;

private:
  const std::list<CoordinateLsa>& m_coordinateLsas;
};

/**
 * @brief Abstraction to publish name lsa dataset
 * \sa http://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class NameLsaPublisher : public LsaPublisher<tlv::NameLsa>
{
public:
  NameLsaPublisher(Lsdb& lsdb,
                   ndn::Face& face,
                   ndn::KeyChain& keyChain);

  std::list<tlv::NameLsa>
  getTlvLsas();

public:
  static const ndn::Name::Component DATASET_COMPONENT;

private:
  const std::list<NameLsa>& m_nameLsas;
};

} // namespace nlsr

#endif // NLSR_PUBLISHER_LSA_PUBLISHER_HPP
