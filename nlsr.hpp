#ifndef NLSR_HPP
#define NLSR_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "conf_param.hpp"
#include "adl.hpp"
#include "npl.hpp"


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
	{
		isDaemonProcess=false;
		configFileName="nlsr.conf";	
	}

	void processInterest(const ptr_lib::shared_ptr<const Name> &name, 
							const ptr_lib::shared_ptr<const Interest> &interest);
	void processContent(const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest,
								 const ndn::ptr_lib::shared_ptr<ndn::Data> &data);
	void nlsrRegistrationFailed(const ptr_lib::shared_ptr<const Name>&);
	void processInterestTimedOut(const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest);

	void setInterestFilterNlsr(const string& name);

	void expressInterest(const string& interestNamePrefix, int scope, int seconds);

	//void scheduleSomeInterest(const string& interestName);

	void sendScheduledInfoInterest(int seconds);
	void scheduleInfoInterest(int seconds);

	void startEventLoop();
	
	int usage(const string& progname);

	string getConfFileName(){
		return configFileName;
	}

	void setConfFileName(const string& fileName){
		configFileName=fileName;
	}

	bool isSetDaemonProcess(){
		return isDaemonProcess;
	}

	void setIsDaemonProcess(bool value){
		isDaemonProcess=value;
	}

	ConfParameter confParam;
	Adl adl;
	Npl npl;
	private:
	ndn::shared_ptr<boost::asio::io_service> io;
	ndn::Scheduler scheduler;
	ndn::Face nlsrFace;
	ndn::KeyChain kChain;
	bool isDaemonProcess;
	string configFileName;

};

#endif
