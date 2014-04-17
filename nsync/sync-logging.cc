/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "sync-logging.h"

#ifdef HAVE_LOG4CXX

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/level.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/defaultconfigurator.h>
#include <log4cxx/helpers/exception.h>
using namespace log4cxx;
using namespace log4cxx::helpers;

#include <unistd.h>

void
INIT_LOGGERS ()
{
  static bool configured = false;

  if (configured) return;
  
  if (access ("log4cxx.properties", R_OK)==0)
    PropertyConfigurator::configureAndWatch ("log4cxx.properties");
  else
    {
      PatternLayoutPtr   layout   (new PatternLayout ("%d{HH:mm:ss} %p %c{1} - %m%n"));
      ConsoleAppenderPtr appender (new ConsoleAppender (layout));

      BasicConfigurator::configure( appender );
      Logger::getRootLogger()->setLevel (log4cxx::Level::getInfo ());
    }

  configured = true;
}

#endif
