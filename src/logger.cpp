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
