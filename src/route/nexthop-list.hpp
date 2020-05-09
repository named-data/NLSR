/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  The University of Memphis,
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

#ifndef NLSR_NEXTHOP_LIST_HPP
#define NLSR_NEXTHOP_LIST_HPP

#include "nexthop.hpp"
#include "adjacent.hpp"

#include <ndn-cxx/face.hpp>
#include <set>

namespace nlsr {

struct NextHopComparator {
  bool
  operator() (const NextHop& nh1, const NextHop& nh2) const {
    if (nh1.getRouteCostAsAdjustedInteger() < nh2.getRouteCostAsAdjustedInteger()) {
      return true;
    }
    else if (nh1.getRouteCostAsAdjustedInteger() == nh2.getRouteCostAsAdjustedInteger()) {
      return nh1.getConnectingFaceUri() < nh2.getConnectingFaceUri();
    }
    else {
      return false;
    }
  }
};

class NexthopList
{
public:
  NexthopList() = default;

  /*! \brief Adds a next hop to the list.
    \param nh The next hop.

    Adds a next hop to this object. If the next hop is new it is
    added. If the next hop already exists in the list then that next
    hop's route cost is updated.
  */
  void
  addNextHop(const NextHop& nh);

  /*! \brief Remove a next hop from the Next Hop list
      \param nh The NextHop we want to remove.

    The next hop gets removed only if both next hop face and route cost are same.
  */
  void
  removeNextHop(const NextHop& nh);

  size_t
  size() const
  {
    return m_nexthopList.size();
  }

  void
  clear()
  {
    m_nexthopList.clear();
  }

  const std::set<NextHop, NextHopComparator>&
  getNextHops() const
  {
    return m_nexthopList;
  }

  typedef std::set<NextHop, NextHopComparator>::iterator iterator;
  typedef std::set<NextHop, NextHopComparator>::const_iterator const_iterator;
  typedef std::set<NextHop, NextHopComparator>::reverse_iterator reverse_iterator;

  iterator
  begin()
  {
    return m_nexthopList.begin();
  }

  iterator
  end()
  {
    return m_nexthopList.end();
  }

  const_iterator
  cbegin() const
  {
    return m_nexthopList.begin();
  }

  const_iterator
  cend() const
  {
    return m_nexthopList.end();
  }

  reverse_iterator
  rbegin() const
  {
    return m_nexthopList.rbegin();
  }

  reverse_iterator
  rend() const
  {
    return m_nexthopList.rend();
  }

private:
  std::set<NextHop, NextHopComparator> m_nexthopList;
};

bool
operator==(NexthopList& lhs, NexthopList& rhs);

bool
operator==(const NexthopList& lhs, const NexthopList& rhs);

bool
operator!=(const NexthopList& lhs, const NexthopList& rhs);

std::ostream&
operator<<(std::ostream& os, const NexthopList& nhl);

} // namespace nlsr

#endif // NLSR_NEXTHOP_LIST_HPP
