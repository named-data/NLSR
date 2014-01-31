#ifndef NLSR_HPP
#define NLSR_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "conf_param.hpp"
#include "adl.hpp"
#include "npl.hpp"
#include "nlsr_im.hpp"
#include "nlsr_dm.hpp"


using namespace ndn;
using namespace std;

class nlsr
{
public:
	nlsr()
		: io(ndn::make_shared<boost::asio::io_service>())
		, nlsrFace(io)
		, scheduler(*io)
		, configFileName()	
		, confParam()
		, adl()
		, npl()
    , im()
    , dm()
	{
		isDaemonProcess=false;
		configFileName="nlsr.conf";	
	}

	void nlsrRegistrationFailed(const ptr_lib::shared_ptr<const Name>&);

	void setInterestFilterNlsr(const string& name);
	void startEventLoop();
	
	int usage(const string& progname);

	string getConfFileName()
  {
		return configFileName;
	}

  void setConfFileName(const string& fileName)
  {
    configFileName=fileName;
  }

  bool isSetDaemonProcess()
  {
    return isDaemonProcess;
  }

  void setIsDaemonProcess(bool value)
  {
    isDaemonProcess=value;
  }

	ConfParameter confParam;
	Adl adl;
	Npl npl;
	ndn::shared_ptr<boost::asio::io_service> io;
	ndn::Scheduler scheduler;
	ndn::Face nlsrFace;
	ndn::KeyChain kChain;
	interestManager im;
	DataManager dm;

private:
	
	bool isDaemonProcess;
	string configFileName;

};

#endif
