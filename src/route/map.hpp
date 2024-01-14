/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
 *                           Regents of the University of California
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

#ifndef NLSR_MAP_HPP
#define NLSR_MAP_HPP

#include "common.hpp"
#include "lsa/adj-lsa.hpp"

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include <optional>

namespace nlsr {

class Map
{
public:
  /*! \brief Add a map entry to this map.
    \param rtrName The name of the router.

    Adds a router to this map. Each entry is also given an arbitrary,
    ascending mappingNo (mapping number).
  */
  void
  addEntry(const ndn::Name& rtrName);

  /*! Populates the Map with AdjacencyLsas.

    \note IteratorType must an iterator type, and begin to end must represent a valid range.
  */
  template<typename IteratorType>
  void
  createFromAdjLsdb(IteratorType begin, IteratorType end)
  {
    BOOST_STATIC_ASSERT_MSG(is_iterator<IteratorType>::value, "IteratorType must be an iterator!");
    for (auto lsa = begin; lsa != end; lsa++) {
      auto adjLsa = std::static_pointer_cast<AdjLsa>(*lsa);
      addEntry(adjLsa->getOriginRouter());
      for (const auto& adjacent : adjLsa->getAdl().getAdjList()) {
        addEntry(adjacent.getName());
      }
    }
  }

  /*! Populates the Map with CoordinateLsas.

    \note IteratorType must an iterator type, and begin to end must represent a valid range.
  */
  template<typename IteratorType>
  void
  createFromCoordinateLsdb(IteratorType begin, IteratorType end)
  {
    BOOST_STATIC_ASSERT_MSG(is_iterator<IteratorType>::value, "IteratorType must be an iterator!");
    for (auto lsa = begin; lsa != end; lsa++) {
      addEntry((*lsa)->getOriginRouter());
    }
  }

  std::optional<ndn::Name>
  getRouterNameByMappingNo(int32_t mn) const;

  std::optional<int32_t>
  getMappingNoByRouterName(const ndn::Name& rName);

  size_t
  size() const
  {
    return m_bimap.size();
  }

private:
  struct MappingNo;
  boost::bimap<
    boost::bimaps::unordered_set_of<
      boost::bimaps::tagged<ndn::Name, ndn::Name>,
      std::hash<ndn::Name>
    >,
    boost::bimaps::unordered_set_of<
      boost::bimaps::tagged<int32_t, MappingNo>
    >
  > m_bimap;

  friend std::ostream&
  operator<<(std::ostream& os, const Map& map);
};

std::ostream&
operator<<(std::ostream& os, const Map& map);

} // namespace nlsr

#endif // NLSR_MAP_HPP
