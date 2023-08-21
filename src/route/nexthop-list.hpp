/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023,  The University of Memphis,
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
#include <ndn-cxx/util/ostream-joiner.hpp>

#include <set>

namespace nlsr {

struct NextHopUriSortedComparator
{
  bool
  operator()(const NextHop& lhs, const NextHop& rhs) const
  {
    return lhs.getConnectingFaceUri() < rhs.getConnectingFaceUri();
  }
};

template<typename T = std::less<NextHop>>
class NexthopListT
{
public:
  NexthopListT() = default;

  /*! \brief Adds a next hop to the list.
    \param nh The next hop.

    Adds a next hop to this object. If the next hop is new it is
    added. If the next hop already exists but has a higher cost then
    its route cost is updated.
  */
  void
  addNextHop(const NextHop& nh)
  {
    auto it = std::find_if(m_nexthopList.begin(), m_nexthopList.end(),
      [&nh] (const auto& item) {
        return item.getConnectingFaceUri() == nh.getConnectingFaceUri();
      });
    if (it == m_nexthopList.end()) {
      m_nexthopList.insert(nh);
    }
    else if (it->getRouteCost() > nh.getRouteCost()) {
      m_nexthopList.erase(it);
      m_nexthopList.insert(nh);
    }
  }

  /*! \brief Remove a next hop from the Next Hop list
      \param nh The NextHop we want to remove.

    The next hop gets removed only if both next hop face and route cost are same.
  */
  void
  removeNextHop(const NextHop& nh)
  {
    auto it = std::find(m_nexthopList.begin(), m_nexthopList.end(), nh);
    if (it != m_nexthopList.end()) {
      m_nexthopList.erase(it);
    }
  }

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

  const std::set<NextHop, T>&
  getNextHops() const
  {
    return m_nexthopList;
  }

  typedef T value_type;
  typedef typename std::set<NextHop, T>::iterator iterator;
  typedef typename std::set<NextHop, T>::const_iterator const_iterator;
  typedef typename std::set<NextHop, T>::reverse_iterator reverse_iterator;

  iterator
  begin() const
  {
    return m_nexthopList.begin();
  }

  iterator
  end() const
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
  std::set<NextHop, T> m_nexthopList;
};

typedef NexthopListT<> NexthopList;

template<typename T>
bool
operator==(const NexthopListT<T>& lhs, const NexthopListT<T>& rhs)
{
  return lhs.getNextHops() == rhs.getNextHops();
}

template<typename T>
bool
operator!=(const NexthopListT<T>& lhs, const NexthopListT<T>& rhs)
{
  return !(lhs == rhs);
}

template<typename T>
std::ostream&
operator<<(std::ostream& os, const NexthopListT<T>& nhl)
{
  os << "    ";
  std::copy(nhl.cbegin(), nhl.cend(), ndn::make_ostream_joiner(os, "\n    "));
  return os;
}

} // namespace nlsr

#endif // NLSR_NEXTHOP_LIST_HPP
