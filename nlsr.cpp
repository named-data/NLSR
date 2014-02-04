#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include <cstdlib>
#include<string>
#include <sstream>

#include "nlsr.hpp"
#include "nlsr_conf_param.hpp"
#include "nlsr_conf_processor.hpp"
#include "nlsr_lsdb.hpp"
//test purpose of NLSR
#include "nlsr_test.hpp" 

using namespace ndn;
using namespace std;

void
nlsr::nlsrRegistrationFailed(const ptr_lib::shared_ptr<const Name>&)
{
  cerr << "ERROR: Failed to register prefix in local hub's daemon" << endl;
  getNlsrFace().shutdown();
}


void
nlsr::setInterestFilterNlsr(const string& name)
{
  getNlsrFace().setInterestFilter(name,
                        func_lib::bind(&interestManager::processInterest, &im, 
                        boost::ref(*this), _1, _2),
                        func_lib::bind(&nlsr::nlsrRegistrationFailed, this, _1));
}


void
nlsr::startEventLoop()
{
	getNlsrFace().processEvents();
}

int 
nlsr::usage(const string& progname)
{

        cout << "Usage: " << progname << " [OPTIONS...]"<<endl;
        cout << "   NDN routing...." << endl;
        cout << "       -d, --daemon        Run in daemon mode" << endl;
        cout << "       -f, --config_file   Specify configuration file name" <<endl;
        cout << "       -p, --api_port      port where api client will connect" <<endl;
        cout << "       -h, --help          Display this help message" << endl;

        exit(EXIT_FAILURE);
}

int 
main(int argc, char **argv){

	nlsr nlsr;
	string programName(argv[0]);
	nlsr.setConfFileName("nlsr.conf");

	int opt;
  while ((opt = getopt(argc, argv, "df:p:h")) != -1) {
    switch (opt) {
    case 'f':
      nlsr.setConfFileName(optarg);
      break;
    case 'd':
      nlsr.setIsDaemonProcess(optarg);
      break;
    case 'p':
    		{
    			stringstream sst(optarg);
    			int ap;
    			sst>>ap;
    			nlsr.setApiPort(ap);
    		}
      break;
    case 'h':
    default:
      nlsr.usage(programName);
      return EXIT_FAILURE;
    }
  }
	
	
	ConfFileProcessor cfp(nlsr.getConfFileName());
	int res=cfp.processConfFile(nlsr);
	if ( res < 0 )
	{
		return EXIT_FAILURE;
	}
	nlsr.getConfParameter().buildRouterPrefix();

	/* debugging purpose start */
	cout <<	nlsr.getConfParameter();
	nlsr.getAdl().printAdl();
	nlsr.getNpl().printNpl();
  /* debugging purpose end */

	nlsr.getLsdb().buildAndInstallOwnNameLsa(nlsr);
	nlsr.getLsdb().buildAndInstallOwnCorLsa(nlsr);

	//testing purpose
	nlsr.getNlsrTesting().schedlueAddingLsas(nlsr);

	nlsr.setInterestFilterNlsr(nlsr.getConfParameter().getRouterPrefix());
	nlsr.getIm().scheduleInfoInterest(nlsr,1);

	
	

	try{
		nlsr.startEventLoop();
	}catch(std::exception &e) {
    		std::cerr << "ERROR: " << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}
