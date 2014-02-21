#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>
#include <cstdlib>
#include <string>
#include <sstream>

#include "nlsr.hpp"
#include "nlsr_conf_param.hpp"
#include "nlsr_conf_processor.hpp"
#include "nlsr_lsdb.hpp"
#include "nlsr_logger.hpp"
//test purpose of NLSR
#include "nlsr_test.hpp"

namespace nlsr
{

    using namespace ndn;
    using namespace std;

    void
    Nlsr::nlsrRegistrationFailed(const ndn::Name& name)
    {
        cerr << "ERROR: Failed to register prefix in local hub's daemon" << endl;
        getNlsrFace().shutdown();
    }


    void
    Nlsr::setInterestFilterNlsr(const string& name)
    {
        getNlsrFace().setInterestFilter(name,
                                        func_lib::bind(&interestManager::processInterest, &im,
                                                boost::ref(*this), _1, _2),
                                        func_lib::bind(&Nlsr::nlsrRegistrationFailed, this, _1));
    }


    void
    Nlsr::startEventLoop()
    {
        getNlsrFace().processEvents();
    }

    int
    Nlsr::usage(const string& progname)
    {
        cout << "Usage: " << progname << " [OPTIONS...]"<<endl;
        cout << "   NDN routing...." << endl;
        cout << "       -d, --daemon        Run in daemon mode" << endl;
        cout << "       -f, --config_file   Specify configuration file name" <<endl;
        cout << "       -p, --api_port      port where api client will connect" <<endl;
        cout << "       -h, --help          Display this help message" << endl;
        exit(EXIT_FAILURE);
    }


} // namespace nlsr

using namespace nlsr;

int
main(int argc, char **argv)
{
    nlsr::Nlsr nlsr_;
    string programName(argv[0]);
    nlsr_.setConfFileName("nlsr.conf");
    int opt;
    while ((opt = getopt(argc, argv, "df:p:h")) != -1)
    {
        switch (opt)
        {
            case 'f':
                nlsr_.setConfFileName(optarg);
                break;
            case 'd':
                nlsr_.setIsDaemonProcess(optarg);
                break;
            case 'p':
                {
                    stringstream sst(optarg);
                    int ap;
                    sst>>ap;
                    nlsr_.setApiPort(ap);
                }
                break;
            case 'h':
            default:
                nlsr_.usage(programName);
                return EXIT_FAILURE;
        }
    }
    ConfFileProcessor cfp(nlsr_.getConfFileName());
    int res=cfp.processConfFile(nlsr_);
    if ( res < 0 )
    {
        return EXIT_FAILURE;
    }
    nlsr_.getConfParameter().buildRouterPrefix();
    nlsr_.getNlsrLogger().initNlsrLogger(nlsr_.getConfParameter().getLogDir());
    //src::logger lg;
    //BOOST_LOG(lg) << "Some log record from nlsr.cpp";
    //for(int j=0; j< 1000; j++)
    //{
    //	BOOST_LOG(lg) << "Some log record from nlsr.cpp "<<j;
    //}
    nlsr_.getLsdb().setLsaRefreshTime(nlsr_.getConfParameter().getLsaRefreshTime());
    nlsr_.getFib().setFibEntryRefreshTime(
                                    2*nlsr_.getConfParameter().getLsaRefreshTime());
    nlsr_.getLsdb().setThisRouterPrefix(nlsr_.getConfParameter().getRouterPrefix());
    nlsr_.getKeyManager().initKeyManager(nlsr_.getConfParameter());
    /* debugging purpose start */
    cout <<	nlsr_.getConfParameter();
    nlsr_.getAdl().printAdl();
    nlsr_.getNpl().printNpl();
    /* debugging purpose end */
    nlsr_.getLsdb().buildAndInstallOwnNameLsa(nlsr_);
    nlsr_.getLsdb().buildAndInstallOwnCorLsa(nlsr_);
    nlsr_.setInterestFilterNlsr(nlsr_.getConfParameter().getRouterPrefix());
    nlsr_.getIm().scheduleInfoInterest(nlsr_,1);
    //testing purpose
    nlsr_.getNlsrTesting().schedlueAddingLsas(nlsr_);
    try
    {
        nlsr_.startEventLoop();
    }
    catch(std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}
