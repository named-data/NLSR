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

#ifndef NLSR_NAME_MAP_HPP
#define NLSR_NAME_MAP_HPP

#include "common.hpp"
#include "lsa/adj-lsa.hpp"

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/concept_check.hpp>

#include <optional>

namespace nlsr {

/**
 * @brief Assigning numbers to router names.
 *
 * NameMap assigns a "mapping number" to each inserted router name. It then provides bidirectional
 * lookups between router names and their mapping numbers.
 *
 * These numbers are non-negative integers assigned sequentially, starting from zero. They can
 * support constructing a matrix of routers, where the mapping numbers are used as row and column
 * indices in place of router names.
 */
class NameMap
{
public:
  /**
   * @brief Create a NameMap populated with router names in Adjacency LSAs.
   * @tparam IteratorType A *LegacyInputIterator* whose value type is convertible to
   *                      `std::shared_ptr<AdjLsa>`.
   * @param first Range begin iterator.
   * @param last Range past-end iterator. It must be reachable by incrementing @p first .
   * @returns NameMap populated with origin and adjacent router names.
   */
  template<typename IteratorType>
  static NameMap
  createFromAdjLsdb(IteratorType first, IteratorType last)
  {
    BOOST_CONCEPT_ASSERT((boost::InputIterator<IteratorType>));
    NameMap map;
    for (auto it = first; it != last; ++it) {
      // *it has type std::shared_ptr<Lsa> ; it->get() has type Lsa*
      auto lsa = static_cast<const AdjLsa*>(it->get());
      map.addEntry(lsa->getOriginRouter());
      for (const auto& adjacent : lsa->getAdl().getAdjList()) {
        map.addEntry(adjacent.getName());
      }
    }
    return map;
  }

  /**
   * @brief Create a NameMap populated with router names in Coordinate LSAs.
   * @tparam IteratorType A *LegacyInputIterator* whose value type is `std::shared_ptr<Lsa>`.
   * @param first Range begin iterator.
   * @param last Range past-end iterator. It must be reachable by incrementing @p first .
   * @returns NameMap populated with origin router names.
   */
  template<typename IteratorType>
  static NameMap
  createFromCoordinateLsdb(IteratorType first, IteratorType last)
  {
    BOOST_CONCEPT_ASSERT((boost::InputIterator<IteratorType>));
    NameMap map;
    for (auto it = first; it != last; ++it) {
      map.addEntry((*it)->getOriginRouter());
    }
    return map;
  }

  /**
   * @brief Insert a router name.
   * @param rtrName Router name.
   */
  void
  addEntry(const ndn::Name& rtrName);

  /**
   * @brief Find router name by its mapping number.
   * @param mn Mapping number.
   * @returns Router name, or @c std::nullopt if it does not exist.
   */
  std::optional<ndn::Name>
  getRouterNameByMappingNo(int32_t mn) const;

  /**
   * @brief Find mapping number of a router name.
   * @param rtrName Router name.
   * @returns Mapping number, or @c std::nullopt if it does not exist.
   */
  std::optional<int32_t>
  getMappingNoByRouterName(const ndn::Name& rtrName) const;

  /**
   * @brief Return number of entries in this container.
   * @returns Number of entries in this container.
   */
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
  operator<<(std::ostream& os, const NameMap& map);
};

std::ostream&
operator<<(std::ostream& os, const NameMap& map);

} // namespace nlsr

#endif // NLSR_NAME_MAP_HPP
