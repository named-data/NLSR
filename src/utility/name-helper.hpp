/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022,  The University of Memphis,
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
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 */

#ifndef NLSR_NAME_HELPER_HPP
#define NLSR_NAME_HELPER_HPP

#include "common.hpp"

namespace nlsr::util {

/*!
   \brief search a name component in ndn::Name and return the position of the component
   \param name         where to search the searchString
   \param searchString the string to search in name
   \return -1 if searchString not found else return the position
   starting from 0
 */
inline int32_t
getNameComponentPosition(const ndn::Name& name, const std::string& searchString)
{
  ndn::name::Component component(searchString);
  size_t nameSize = name.size();
  for (uint32_t i = 0; i < nameSize; i++) {
    if (component == name[i]) {
      return static_cast<int32_t>(i);
    }
  }
  return -1;
}

} // namespace nlsr::util

#endif // NLSR_NAME_HELPER_HPP
