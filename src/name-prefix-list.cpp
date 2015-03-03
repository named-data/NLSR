/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
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

#include <iostream>
#include <algorithm>

#include <ndn-cxx/common.hpp>

#include "common.hpp"
#include "name-prefix-list.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("NamePrefixList");

using namespace std;

NamePrefixList::NamePrefixList()
{
}

NamePrefixList::~NamePrefixList()
{
}

static bool
nameCompare(const ndn::Name& name1, const ndn::Name& name2)
{
  return name1 == name2;
}

bool
NamePrefixList::insert(const ndn::Name& name)
{
  std::list<ndn::Name>::iterator it = std::find_if(m_nameList.begin(),
                                                   m_nameList.end(),
                                                   ndn::bind(&nameCompare, _1 ,
                                                             ndn::cref(name)));
  if (it != m_nameList.end()) {
    return false;
  }
  m_nameList.push_back(name);
  return true;
}

bool
NamePrefixList::remove(const ndn::Name& name)
{
  std::list<ndn::Name>::iterator it = std::find_if(m_nameList.begin(),
                                                   m_nameList.end(),
                                                   ndn::bind(&nameCompare, _1 ,
                                                   ndn::cref(name)));
  if (it != m_nameList.end()) {
    m_nameList.erase(it);
    return true;
  }

  return false;
}

void
NamePrefixList::sort()
{
  m_nameList.sort();
}

void
NamePrefixList::writeLog()
{
  _LOG_DEBUG("-------Name Prefix List--------");
  int i = 1;
  for (std::list<ndn::Name>::iterator it = m_nameList.begin();
       it != m_nameList.end(); it++) {
    _LOG_DEBUG("Name " << i << " : " << (*it));
    i++;
  }
}

}//namespace nlsr
