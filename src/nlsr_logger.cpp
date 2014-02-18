#include "nlsr_logger.hpp"


string 
NlsrLogger::getEpochTime()
{
	std::stringstream ss;
	boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1970,1,1)); 
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration diff = now - time_t_epoch;
	ss<<diff.total_seconds()<<"."<<boost::format("%06i")%(diff.total_microseconds()%1000000);
	return ss.str();
}

string
NlsrLogger::getUserHomeDirectory()
{
	string homeDirPath(getpwuid(getuid())->pw_dir);
	if( homeDirPath.empty() )
	{
		homeDirPath = getenv("HOME");
	}

	return homeDirPath;
}

void 
NlsrLogger::initNlsrLogger(std::string dirPath)
{
	string logDirPath(dirPath);
	if( dirPath.empty() )
	{
		logDirPath=getUserHomeDirectory();
	}
	 
	typedef sinks::synchronous_sink< sinks::text_file_backend > file_sink;
	shared_ptr< file_sink > sink(new file_sink(
		keywords::file_name = "NLSR%Y%m%d%H%M%S_%3N.log",  // file name pattern
		keywords::rotation_size = 128 * 1024 * 1024 ,// rotation size, in characters
		keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0)
		));
		
	sink->locked_backend()->set_file_collector(sinks::file::make_collector(
		keywords::target = logDirPath,                // where to store rotated files
		keywords::max_size = 64 * 1024 * 1024 * 1024, // maximum total size of the stored files, in bytes
		keywords::min_free_space = 128 * 1024 * 1024  // minimum free space on the drive, in bytes
 		));

	sink->set_formatter
  (
		expr::format("%1%: %2%")
				% getEpochTime()
				% expr::smessage
	);

  // Add it to the core
	logging::core::get()->add_sink(sink);
}
