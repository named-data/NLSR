/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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

  /*! \brief Class to publish adjacency lsa dataset

    \sa https://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet

  */
class AdjacencyLsaPublisher
{
public:
  AdjacencyLsaPublisher(Lsdb& lsdb,
                        ndn::Face& face,
                        ndn::KeyChain& keyChain);

  /*! \brief Generates an TLV-format AdjacencyLsa from AdjacencyLsas
   * and their Adjacents.
   */
  std::list<tlv::AdjacencyLsa>
  getTlvLsas();

public:
  static const ndn::Name::Component DATASET_COMPONENT;

private:
  const std::list<AdjLsa>& m_adjacencyLsas;
};

  /*! \brief Class to publish coordinate lsa dataset
    \sa https://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
  */
class CoordinateLsaPublisher
{
public:
  CoordinateLsaPublisher(Lsdb& lsdb,
                        ndn::Face& face,
                        ndn::KeyChain& keyChain);

  /*! \brief Generates a TLV-format CoordinateLsa from CoordinateLsas
   * and their hyperbolic coordinates.
   */
  std::list<tlv::CoordinateLsa>
  getTlvLsas();

public:
  static const ndn::Name::Component DATASET_COMPONENT;

private:
  const std::list<CoordinateLsa>& m_coordinateLsas;
};

  /*! \brief Class to publish name lsa dataset
    \sa https://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
  */
class NameLsaPublisher
{
public:
  NameLsaPublisher(Lsdb& lsdb,
                   ndn::Face& face,
                   ndn::KeyChain& keyChain);

  /*! \brief Generates a TLV-format NameLsa from NameLsas and their
   * list of name prefixes.
   */
  std::list<tlv::NameLsa>
  getTlvLsas();

public:
  static const ndn::Name::Component DATASET_COMPONENT;

private:
  const std::list<NameLsa>& m_nameLsas;
};

} // namespace nlsr

#endif // NLSR_PUBLISHER_LSA_PUBLISHER_HPP
