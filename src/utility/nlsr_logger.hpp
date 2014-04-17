#ifndef NLSR_LOGGER_HPP
#define NLSR_LOGGER_HPP

#define BOOST_LOG_DYN_LINK 1
#include "boost-log.hpp"

#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <pwd.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <boost/format.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>




namespace nlsr
{

  namespace logging = boost::log;
  namespace attrs = boost::log::attributes;
  namespace src = boost::log::sources;
  namespace sinks = boost::log::sinks;
  namespace expr = boost::log::expressions;
  namespace keywords = boost::log::keywords;

  using boost::shared_ptr;
  using namespace std;


  class NlsrLogger
  {
  public:
    NlsrLogger()
    {
    }

    void initNlsrLogger(std::string dirPath);

    src::logger& getLogger()
    {
      return mLogger;
    }

  private:
    string getEpochTime();
    string getUserHomeDirectory();

  private:
    src::logger mLogger;
  };

}//namespace nlsr
#endif
