/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#ifndef NLSR_NEXTHOP_LIST_HPP
#define NLSR_NEXTHOP_LIST_HPP

#include <list>
#include <iostream>
#include <boost/cstdint.hpp>

#include <ndn-cxx/face.hpp>

#include "nexthop.hpp"
#include "adjacent.hpp"

namespace nlsr {

class NexthopList
{
public:
  NexthopList()
  {
  }

  ~NexthopList()
  {
  }

  void
  addNextHop(NextHop& nh);

  void
  removeNextHop(NextHop& nh);

  void
  sort();

  size_t
  getSize()
  {
    return m_nexthopList.size();
  }

  void
  reset()
  {
    m_nexthopList.clear();
  }

  std::list<NextHop>&
  getNextHops()
  {
    return m_nexthopList;
  }

  typedef std::list<NextHop>::iterator iterator;
  typedef std::list<NextHop>::const_iterator const_iterator;

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

  void
  writeLog();

private:
  std::list<NextHop> m_nexthopList;
};

}//namespace nlsr

#endif //NLSR_NEXTHOP_LIST_HPP
