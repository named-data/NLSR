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
    , nlsrLsdb()
    , nameLsaSeq(0)
    , adjLsaSeq(0)
    , corLsaSeq(0)
	{
		isDaemonProcess=false;
		configFileName="nlsr.conf";	
	}

	nlsr(string confFile, uint32_t nlsn, uint32_t alsn, uint32_t clsn)
		: io(ndn::make_shared<boost::asio::io_service>())
		, nlsrFace(io)
		, scheduler(*io)
		, configFileName()	
		, confParam()
		, adl()
		, npl()
    , im()
    , dm()
    , nlsrLsdb()
	{
		isDaemonProcess=false;
		configFileName=confFile;
		nameLsaSeq=nlsn;
    adjLsaSeq=alsn;
    corLsaSeq=clsn;
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

	Lsdb& getLsdb(){
		return nlsrLsdb;
	}

	uint32_t getNameLsaSeq()
	{
		return nameLsaSeq;
	}

	void setNameLsaSeq(uint32_t nlsn){
		nameLsaSeq=nlsn;
	}

	uint32_t getAdjLsaSeq()
	{
		return adjLsaSeq;
	}

	void setAdjLsaSeq(uint32_t alsn){
		adjLsaSeq=alsn;
	}

	uint32_t getCorLsaSeq()
	{
		return corLsaSeq;
	}

	void setCorLsaSeq(uint32_t clsn){
		corLsaSeq=clsn;
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
	bool isDaemonProcess;
	string configFileName;
	Lsdb nlsrLsdb;
	uint32_t nameLsaSeq;
	uint32_t adjLsaSeq;
	uint32_t corLsaSeq;
	

};

#endif
