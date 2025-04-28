/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
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

#ifndef NLSR_BOOST_INFO_EDITOR_HPP
#define NLSR_BOOST_INFO_EDITOR_HPP

#include <string>

namespace nlsr::util::boost_info_editor {

/*! Insert a specified combination of configuration field name and value into a specific file path
  Returns false if any errors occur on I/O, otherwise true.
  \param fileName Path to conf file as string
  \param section Section name to add/modify in conf file; expects [section].[subsection]... formatting
  \param value The value to insert into the sheet as a string. Typechecking should be done in caller.
*/
bool put(const std::string& fileName, const std::string& section, const std::string& value);

/*! Remove a specified key and its value from a configuration file at a specific path
  Returns false if any errors occur on I/O, otherwise true.
  \param fileName Path to conf file as string
  \param section Section name to add/modify in conf file; expects [section].[subsection]... formatting
*/
bool remove(const std::string& fileName, const std::string& section);
} // namespace nlsr::util::boost_info_editor

#endif // NLSR_BOOST_INFO_EDITOR_HPP
