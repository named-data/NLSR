/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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

#ifndef NLSR_COMMON_HPP
#define NLSR_COMMON_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <utility>

#include <ndn-cxx/name.hpp>
#include <ndn-cxx/util/exception.hpp>
#include <ndn-cxx/util/time.hpp>

namespace nlsr {

using namespace ndn::time_literals;

const ndn::time::seconds TIME_ALLOWED_FOR_CANONIZATION = 4_s;

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
