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
 **/

/*! \file logger.hpp
 * \brief Define macros and auxiliary functions for logging.
 *
 * This file defines the macros that NLSR uses for logging
 * messages. An intrepid hacker could replace this system cleanly by
 * providing a system that redefines all of the _LOG_* macros with an
 * arbitrary system, as long as the underlying system accepts strings.
 */

#ifndef NLSR_LOGGER_HPP
#define NLSR_LOGGER_HPP

#include <ndn-cxx/util/logger.hpp>

#define INIT_LOGGER(name) NDN_LOG_INIT(nlsr.name)

#define NLSR_LOG_TRACE(x) NDN_LOG_TRACE(x)
#define NLSR_LOG_DEBUG(x) NDN_LOG_DEBUG(x)
#define NLSR_LOG_INFO(x) NDN_LOG_INFO(x)
#define NLSR_LOG_WARN(x) NDN_LOG_WARN(x)
#define NLSR_LOG_ERROR(x) NDN_LOG_ERROR(x)
#define NLSR_LOG_FATAL(x) NDN_LOG_FATAL(x)

#endif // NLSR_LOGGER_HPP
