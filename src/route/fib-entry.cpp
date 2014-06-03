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
#include <list>
#include "fib-entry.hpp"
#include "nexthop.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("FibEntry");

using namespace std;

bool
FibEntry::isEqualNextHops(NexthopList& nhlOther)
{
  if (m_nexthopList.getSize() != nhlOther.getSize()) {
    return false;
  }
  else {
    uint32_t nhCount = 0;
    std::list<NextHop>::iterator it1, it2;
    for (it1 = m_nexthopList.getNextHops().begin(),
         it2 = nhlOther.getNextHops().begin() ;
         it1 != m_nexthopList.getNextHops().end() ; it1++, it2++) {
      if (it1->getConnectingFaceUri() == it2->getConnectingFaceUri()) {
        it1->setRouteCost(it2->getRouteCost());
        nhCount++;
      }
      else {
        break;
      }
    }
    return nhCount == m_nexthopList.getSize();
  }
}

void
FibEntry::writeLog()
{
  _LOG_DEBUG("Name Prefix: " << m_name);
  _LOG_DEBUG("Time to Refresh: " << m_expirationTimePoint);
  _LOG_DEBUG("Seq No: " << m_seqNo);
  m_nexthopList.writeLog();
}

}//namespace nlsr
