/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022,  The University of Memphis,
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

 /*! \file dataset-interest-handler.hpp

  This file details a class that is used by NLSRC and other command-line
  tools to examine the state of NLSR. This class doesn't only handle interest
  from local host, but also handle interests from remote router.
  This system is not designed to
  be used by routers to publish data to each other.
 */

#ifndef NLSR_PUBLISHER_DATASET_INTEREST_HANDLER_HPP
#define NLSR_PUBLISHER_DATASET_INTEREST_HANDLER_HPP

#include "route/routing-table-entry.hpp"
#include "route/routing-table.hpp"
#include "route/nexthop-list.hpp"
#include "lsdb.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/mgmt/dispatcher.hpp>
#include <boost/noncopyable.hpp>

namespace nlsr {

namespace dataset {
inline const ndn::Name::Component ADJACENCY_COMPONENT{"adjacencies"};
inline const ndn::Name::Component NAME_COMPONENT{"names"};
inline const ndn::Name::Component COORDINATE_COMPONENT{"coordinates"};
} // namespace dataset

/*!
   \brief Class to publish all dataset
   \sa https://redmine.named-data.net/projects/nlsr/wiki/LSDB_DataSet
   \sa https://redmine.named-data.net/projects/nlsr/wiki/Routing_Table_DataSet
 */
class DatasetInterestHandler : boost::noncopyable
{
public:
  class Error : std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  DatasetInterestHandler(ndn::mgmt::Dispatcher& dispatcher,
                         const Lsdb& lsdb,
                         const RoutingTable& rt);

private:
  /*! \brief provide routing-table dataset
  */
  void
  publishRtStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                  ndn::mgmt::StatusDatasetContext& context);

  /*! \brief provide LSA status dataset
   */
  template<typename T>
  void
  publishLsaStatus(const ndn::Name& topPrefix, const ndn::Interest& interest,
                   ndn::mgmt::StatusDatasetContext& context);

private:
  const Lsdb& m_lsdb;
  const RoutingTable& m_routingTable;
};

} // namespace nlsr

#endif // NLSR_PUBLISHER_DATASET_INTEREST_HANDLER_HPP
