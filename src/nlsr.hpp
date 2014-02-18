#ifndef NLSR_HPP
#define NLSR_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "nlsr_conf_param.hpp"
#include "nlsr_adl.hpp"
#include "nlsr_npl.hpp"
#include "nlsr_im.hpp"
#include "nlsr_dm.hpp"
#include "nlsr_lsdb.hpp"
#include "nlsr_sm.hpp"
#include "nlsr_rt.hpp"
#include "nlsr_npt.hpp"
#include "nlsr_fib.hpp"
#include "nlsr_logger.hpp"
//testing
#include "nlsr_test.hpp"



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
    , sm()
    , nlsrLsdb()
    , adjBuildCount(0)
    , isBuildAdjLsaSheduled(0)
    , isRouteCalculationScheduled(0)
    , isRoutingTableCalculating(0)
    , routingTable()
    , npt()
    , fib()
    , nlsrLogger()
    , nlsrTesting()
	{
		isDaemonProcess=false;
		configFileName="nlsr.conf";	
	}

	void nlsrRegistrationFailed(const ndn::Name& name);

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

	ConfParameter& getConfParameter(){
		return confParam;
	}

	Adl& getAdl(){
		return adl;
	}

	Npl& getNpl(){
		return npl;
	}

	ndn::shared_ptr<boost::asio::io_service>& getIo()
	{
		return io;
	}

	ndn::Scheduler& getScheduler(){
		return scheduler;
	}

	ndn::Face& getNlsrFace(){
		return nlsrFace;
	}

	ndn::KeyChain& getKeyChain(){
		return kChain;
	}

	interestManager& getIm(){
		return im;
	}

	DataManager& getDm(){
		return dm;
	}

	SequencingManager& getSm(){
	 return sm;
	}

	Lsdb& getLsdb(){
		return nlsrLsdb;
	}

	RoutingTable& getRoutingTable(){
		return routingTable;
	}

	Npt& getNpt()
	{
		return npt;
	}

	Fib& getFib()
	{
		return fib;
	}

	long int getAdjBuildCount()
	{
		return adjBuildCount;
	}

	void incrementAdjBuildCount()
	{
		adjBuildCount++;
	}

	void setAdjBuildCount(long int abc)
	{
		adjBuildCount=abc;
	}

	int getIsBuildAdjLsaSheduled()
	{
		return isBuildAdjLsaSheduled;
	}

	void setIsBuildAdjLsaSheduled(int iabls)
	{
		isBuildAdjLsaSheduled=iabls;
	}

	nlsrTest& getNlsrTesting()
	{
		return nlsrTesting;
	}

	void setApiPort(int ap)
	{
		apiPort=ap;
	}

	int getApiPort()
	{
		return apiPort;
	}

	int getIsRoutingTableCalculating()
	{
		return isRoutingTableCalculating;
	}

	void setIsRoutingTableCalculating(int irtc)
	{
		isRoutingTableCalculating=irtc;
	}

	int getIsRouteCalculationScheduled()
	{
		return isRouteCalculationScheduled;
	}

	void setIsRouteCalculationScheduled(int ircs)
	{
		isRouteCalculationScheduled=ircs;
	}

	NlsrLogger& getNlsrLogger()
	{
		return nlsrLogger;
	}
	
private:
	ConfParameter confParam;
	Adl adl;
	Npl npl;
	ndn::shared_ptr<boost::asio::io_service> io;
	ndn::Scheduler scheduler;
	ndn::Face nlsrFace;
	ndn::KeyChain kChain;
	interestManager im;
	DataManager dm;
	SequencingManager sm;
	bool isDaemonProcess;
	string configFileName;
	int apiPort;
	
	Lsdb nlsrLsdb;
	RoutingTable routingTable;
	Npt npt;
	Fib fib;
	NlsrLogger nlsrLogger;

	long int adjBuildCount;
	int isBuildAdjLsaSheduled;
	int isRouteCalculationScheduled;
	int isRoutingTableCalculating;

	nlsrTest nlsrTesting;
	

};

#endif
