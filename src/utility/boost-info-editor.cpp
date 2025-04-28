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

#include "boost-info-editor.hpp"
#include "logger.hpp"

#include <boost/property_tree/info_parser.hpp>

namespace nlsr::util::boost_info_editor {

// Incorporates Apache 2.0 licensed code by Yanbiao Li under a compatible license
boost::property_tree::ptree
load(const std::string& fileName)
{
  boost::property_tree::ptree info;
  std::ifstream input(fileName);

  // Thrown errors are received at calling function
  boost::property_tree::info_parser::read_info(input, info);

  input.close();
  return info;
}

bool
save(const std::string& fileName, boost::property_tree::ptree& info)
{
  std::ofstream output(fileName);
  try {
    write_info(output, info);
  } catch (const boost::property_tree::info_parser::info_parser_error& error) {
    return false;
  }
  output.close();
  return true;
}

bool
put(const std::string& fileName, const std::string& section, const std::string& value)
{
  boost::property_tree::ptree info;
  try {
    info = load(fileName);
  }
  catch (const boost::property_tree::info_parser::info_parser_error& error) {
    return false;
  }
  info.put(section.c_str(), value.c_str());
  return save(fileName, info);
}

bool
remove(const std::string& fileName, const std::string& section)
{
  boost::property_tree::ptree info;
  try {
    info = load(fileName);
  }
  catch (const boost::property_tree::info_parser::info_parser_error& error) {
    return false;
  }
  std::size_t pos = section.find_last_of(".");
  if (pos == std::string::npos) {
    info.erase(section.c_str());
  }
  else {
    boost::optional<boost::property_tree::ptree&> child =
      info.get_child_optional(section.substr(0, pos));
    if (child) {
      child->erase(section.substr(pos + 1));
    }
  }
  return save(fileName, info);
}
} // namespace nlsr::util::boost_info_editor
