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

#ifndef NLSR_LOGGER_HPP
#define NLSR_LOGGER_HPP

#include <log4cxx/logger.h>

#define INIT_LOGGER(name) \
  static log4cxx::LoggerPtr staticModuleLogger = log4cxx::Logger::getLogger(name)

#define _LOG_TRACE(x) \
  LOG4CXX_TRACE(staticModuleLogger, x)

#define _LOG_DEBUG(x) \
  LOG4CXX_DEBUG(staticModuleLogger, x)

#define _LOG_INFO(x) \
  LOG4CXX_INFO(staticModuleLogger, x)

#define _LOG_WARN(x) \
  LOG4CXX_WARN(staticModuleLogger, x)

#define _LOG_ERROR(x) \
  LOG4CXX_ERROR(staticModuleLogger, x)

#define _LOG_FATAL(x) \
  LOG4CXX_FATAL(staticModuleLogger, x);

void
INIT_LOGGERS(const std::string& logDir, const std::string& logLevel);

void
INIT_LOG4CXX(const std::string& log4cxxConfPath);

bool
isValidLogLevel(const std::string& logLevel);

#endif // NLSR_LOGGER_HPP
