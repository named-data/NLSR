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
#ifndef NLSR_NAME_PREFIX_LIST_HPP
#define NLSR_NAME_PREFIX_LIST_HPP

#include <list>
#include <string>
#include <boost/cstdint.hpp>
#include <ndn-cxx/name.hpp>


namespace nlsr {
class NamePrefixList
{

public:
  NamePrefixList();

  ~NamePrefixList();

  int32_t
  insert(const ndn::Name& name);

  int32_t
  remove(const ndn::Name& name);

  void
  sort();

  size_t
  getSize()
  {
    return m_nameList.size();
  }

  std::list<ndn::Name>&
  getNameList()
  {
    return m_nameList;
  }

  void
  writeLog();

private:
  std::list<ndn::Name> m_nameList;

};

}//namespace nlsr

#endif //NLSR_NAME_PREFIX_LIST_HPP
