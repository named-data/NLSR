/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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

#ifndef NLSR_NUMERIC_HPP
#define NLSR_NUMERIC_HPP

#include "common.hpp"

namespace nlsr::util {

/**
 * @brief Determine whether the difference between two numbers are within epsilon.
 * @param lhs first number.
 * @param rhs second number.
 * @returns true if their difference is within epsilon.
 */
inline bool
diffInEpsilon(double lhs, double rhs)
{
  return std::abs(lhs - rhs) < std::numeric_limits<double>::epsilon();
}

} // namespace nlsr::util

#endif // NLSR_NUMERIC_HPP
