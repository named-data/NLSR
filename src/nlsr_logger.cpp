#include "nlsr_logger.hpp"

namespace nlsr
{

    string
    NlsrLogger::getEpochTime()
    {
        std::stringstream ss;
        boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1970,1,1));
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration diff = now - time_t_epoch;
        ss<<diff.total_seconds()<<"."<<boost::format("%06i")%(diff.total_microseconds()
                %1000000);
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
            logDirPath=getUserHomeDirectory()+"/nlsrLog";
        }
        cout<<"Log Dir Path: "<< logDirPath<<endl;
        typedef sinks::synchronous_sink< sinks::text_file_backend > file_sink;
        shared_ptr< file_sink > sink(new file_sink(
                                         keywords::file_name = logDirPath
                                                 +"/NLSR%Y%m%d%H%M%S_%3N.log",
                                         keywords::rotation_size = 128 * 1024 * 1024,
                                         keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0),
                                         keywords::auto_flush = true
                                     ));
        sink->locked_backend()->set_file_collector(sinks::file::make_collector(
                    keywords::target = logDirPath,
                    keywords::max_size = 16 * 1024 * 1024 * 1024,
                    keywords::min_free_space = 128 * 1024 * 1024
                ));
        sink->set_formatter(
            expr::format("%1%: %2%")
            % getEpochTime()
            % expr::smessage
        );
        logging::core::get()->add_sink(sink);
    }

}//namespace nlsr
