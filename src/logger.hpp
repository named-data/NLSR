#ifndef NLSR_LOGGER_HPP
#define NLSR_LOGGER_HPP

#include <log4cxx/logger.h>


#define INIT_LOGGER(name) \
  static log4cxx::LoggerPtr staticModuleLogger = log4cxx::Logger::getLogger(name)

#define _LOG_DEBUG(x) \
  LOG4CXX_DEBUG(staticModuleLogger,x)

#define _LOG_INFO(x) \
  LOG4CXX_INFO(staticModuleLogger, x)

void
INIT_LOGGERS(const std::string& logDir);

#endif // NLSR_LOGGER_HPP
