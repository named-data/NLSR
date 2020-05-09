/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  The University of Memphis,
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

#include "nexthop-list.hpp"
#include "common.hpp"
#include "nexthop.hpp"

#include <ndn-cxx/util/ostream-joiner.hpp>

namespace nlsr {

static bool
nexthopAddCompare(const NextHop& nh1, const NextHop& nh2)
{
  return nh1.getConnectingFaceUri() == nh2.getConnectingFaceUri();
}

static bool
nexthopRemoveCompare(const NextHop& nh1, const NextHop& nh2)
{
  return (nh1.getConnectingFaceUri() == nh2.getConnectingFaceUri() &&
          nh1.getRouteCostAsAdjustedInteger() == nh2.getRouteCostAsAdjustedInteger()) ;
}

bool
operator==(const NexthopList& lhs, const NexthopList& rhs)
{
  if (lhs.size() != rhs.size()) {
    return false;
  }

  NexthopList slhs = lhs;
  NexthopList srhs = rhs;

  for (struct {std::set<NextHop>::iterator lItr;
    std::set<NextHop>::iterator rItr;} pair = {slhs.begin(), srhs.begin()};
       (pair.lItr != slhs.end() || pair.rItr != srhs.end());
       pair.rItr++, pair.lItr++) {
    if (!((*pair.lItr) == (*pair.rItr))) {
      return false;
    }
  }
  return true;
}

bool
operator!=(const NexthopList& lhs, const NexthopList& rhs)
{
  return !(lhs == rhs);
}

std::ostream&
operator<<(std::ostream& os, const NexthopList& nhl)
{
  os << "    ";
  std::copy(nhl.cbegin(), nhl.cend(), ndn::make_ostream_joiner(os, "\n    "));
  return os;
}

void
NexthopList::addNextHop(const NextHop& nh)
{
  auto it = std::find_if(m_nexthopList.begin(), m_nexthopList.end(),
                         std::bind(&nexthopAddCompare, _1, nh));
  if (it == m_nexthopList.end()) {
    m_nexthopList.insert(nh);
  }
  else if (it->getRouteCost() > nh.getRouteCost()) {
    removeNextHop(*it);
    m_nexthopList.insert(nh);
  }
}

void
NexthopList::removeNextHop(const NextHop& nh)
{
  auto it = std::find_if(m_nexthopList.begin(), m_nexthopList.end(),
                         std::bind(&nexthopRemoveCompare, _1, nh));
  if (it != m_nexthopList.end()) {
    m_nexthopList.erase(it);
  }
}

} // namespace nlsr
