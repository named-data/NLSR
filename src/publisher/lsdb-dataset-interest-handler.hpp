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

#ifndef NLSR_PUBLISHER_LSDB_DATASET_INTEREST_HANDLER_HPP
#define NLSR_PUBLISHER_LSDB_DATASET_INTEREST_HANDLER_HPP

#include "tlv/adjacency-lsa.hpp"
#include "tlv/coordinate-lsa.hpp"
#include "tlv/name-lsa.hpp"
#include "lsdb.hpp"

#include <ndn-cxx/mgmt/dispatcher.hpp>
#include <ndn-cxx/face.hpp>
#include <boost/noncopyable.hpp>

namespace nlsr {

namespace dataset {
  const ndn::Name::Component ADJACENCY_COMPONENT = ndn::Name::Component{"adjacencies"};
  const ndn::Name::Component NAME_COMPONENT = ndn::Name::Component{"names"};
  const ndn::Name::Component COORDINATE_COMPONENT = ndn::Name::Component{"coordinates"};
} // namespace dataset

/*!
   \brief Class to publish all lsa dataset
   \sa https://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
 */
class LsdbDatasetInterestHandler : boost::noncopyable
{
public:
  class Error : std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

  LsdbDatasetInterestHandler(Lsdb& lsdb,
                             ndn::mgmt::Dispatcher& localHostDispatcher,
                             ndn::mgmt::Dispatcher& routerNameDispatcher,
                             ndn::Face& face,
                             ndn::KeyChain& keyChain);

  ndn::Name&
  getRouterNameCommandPrefix()
  {
    return m_routerNamePrefix;
  }

  void
  setRouterNameCommandPrefix(const ndn::Name& routerName) {
    m_routerNamePrefix = routerName;
    m_routerNamePrefix.append(Lsdb::NAME_COMPONENT);
  }

private:
  /*! \brief Capture-point for Interests to verify Interests are
   * valid, and then process them.
   */
  void
  setDispatcher(ndn::mgmt::Dispatcher& dispatcher);

  /*! \brief provide adjacent status dataset
   */
  void
  publishAdjStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                   ndn::mgmt::StatusDatasetContext& context);

  /*! \brief provide coordinate status dataset
   */
  void
  publishCoordinateStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                          ndn::mgmt::StatusDatasetContext& context);

  /*! \brief provide name status dataset
   */
  void
  publishNameStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                    ndn::mgmt::StatusDatasetContext& context);

  /*! \brief provide ladb status dataset
   */
  void
  publishAllStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                   ndn::mgmt::StatusDatasetContext& context);

private:
  const Lsdb& m_lsdb;
  ndn::Name m_routerNamePrefix;

  ndn::mgmt::Dispatcher& m_localhostDispatcher;
  ndn::mgmt::Dispatcher& m_routerNameDispatcher;
};

template<typename T> std::list<T>
getTlvLsas(const Lsdb& lsdb);

template<> std::list<tlv::AdjacencyLsa>
getTlvLsas<tlv::AdjacencyLsa>(const Lsdb& lsdb);

template<> std::list<tlv::CoordinateLsa>
getTlvLsas<tlv::CoordinateLsa>(const Lsdb& lsdb);

template<> std::list<tlv::NameLsa>
getTlvLsas<tlv::NameLsa>(const Lsdb& lsdb);

} // namespace nlsr

#endif // NLSR_PUBLISHER_LSDB_DATASET_INTEREST_HANDLER_HPP
