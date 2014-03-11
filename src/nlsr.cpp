#include <cstdlib>
#include <string>
#include <sstream>
#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>


#include "nlsr.hpp"
#include "nlsr_conf_processor.hpp"
#include "utility/nlsr_logger.hpp"
#include "security/nlsr_km.hpp"
#include "security/nlsr_cert_store.hpp"
#include "security/nlsr_cse.hpp"


namespace nlsr
{

    using namespace ndn;
    using namespace std;

    void
    Nlsr::nlsrRegistrationFailed(const ndn::Name& name)
    {
        cerr << "ERROR: Failed to register prefix in local hub's daemon" << endl;
        getNlsrFace()->shutdown();
    }


    void
    Nlsr::setInterestFilterNlsr(const string& name)
    {
        getNlsrFace()->setInterestFilter(name,
                                         func_lib::bind(&interestManager::processInterest, &im,
                                                 boost::ref(*this), _1, _2),
                                         func_lib::bind(&Nlsr::nlsrRegistrationFailed, this, _1));
    }

    void
    Nlsr::initNlsr()
    {
        confParam.buildRouterPrefix();
        nlsrLogger.initNlsrLogger(confParam.getLogDir());
        nlsrLsdb.setLsaRefreshTime(confParam.getLsaRefreshTime());
        nlsrLsdb.setThisRouterPrefix(confParam.getRouterPrefix());
        fib.setFibEntryRefreshTime(2*confParam.getLsaRefreshTime());
        if( ! km.initKeyManager(confParam) )
        {
            std::cerr<<"Can not initiate certificate"<<endl;
        }
        
        sm.setSeqFileName(confParam.getSeqFileDir());
        sm.initiateSeqNoFromFile();
        
        /* debugging purpose start */
        cout <<	confParam;
        adl.printAdl();
        npl.printNpl();
        /* debugging purpose end */
        
        nlsrLsdb.buildAndInstallOwnNameLsa(boost::ref(*this));
        nlsrLsdb.buildAndInstallOwnCorLsa(boost::ref(*this));
        setInterestFilterNlsr(confParam.getRouterPrefix());
        setInterestFilterNlsr(confParam.getChronosyncLsaPrefix()+
                                                   confParam.getRouterPrefix());
        setInterestFilterNlsr(confParam.getRootKeyPrefix());
        slh.setSyncPrefix(confParam.getChronosyncSyncPrefix());
        slh.createSyncSocket(boost::ref(*this));
        slh.publishKeyUpdate(km);
    
        im.scheduleInfoInterest(boost::ref(*this),10);
    }

    void
    Nlsr::startEventLoop()
    {
        io->run();
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
    nlsr_.initNlsr();
    
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
