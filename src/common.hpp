/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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
 **/

/*! \file
 * Shared include file to provide a single point-of-entry, and
 * hopefully improve compile times.
 */

#ifndef NLSR_COMMON_HPP
#define NLSR_COMMON_HPP

#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/name.hpp>

namespace nlsr {

using std::bind;
using std::make_shared;
using std::shared_ptr;
using std::function;

const ndn::time::seconds TIME_ALLOWED_FOR_CANONIZATION = ndn::time::seconds(4);

template<typename T, typename = void>
struct is_iterator
{
   static constexpr bool value = false;
};

/*! Use C++11 iterator_traits to check if some type is an iterator
 */
template<typename T>
struct is_iterator<T, typename std::enable_if<!std::is_same<
  typename std::iterator_traits<T>::value_type,
  void>::value>::type>
{
   static constexpr bool value = true;
};

} // namespace nlsr

#endif // NLSR_COMMON_HPP
