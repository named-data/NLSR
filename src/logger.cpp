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
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/rollingfileappender.h>

#include "logger.hpp"

void
INIT_LOGGERS(const std::string& logDir)
{
  static bool configured = false;
  if (configured) return;

  log4cxx::PatternLayoutPtr
           layout(new log4cxx::PatternLayout("%date{yyyyMMddHHmmssSSS} %c %L: %m%n"));
  log4cxx::RollingFileAppender* rollingFileAppender =
           new log4cxx::RollingFileAppender(layout, logDir+"/nlsr.log", true);
  rollingFileAppender->setMaxFileSize("10MB");
  rollingFileAppender->setMaxBackupIndex(10);
  log4cxx::helpers::Pool p;
  rollingFileAppender->activateOptions(p);
  log4cxx::BasicConfigurator::configure(log4cxx::AppenderPtr(rollingFileAppender));
  log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug());

  configured = true;
}
