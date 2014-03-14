#ifndef NLSR_HPP
#define NLSR_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "nlsr_conf_param.hpp"
#include "nlsr_adl.hpp"
#include "nlsr_npl.hpp"
#include "communication/nlsr_im.hpp"
#include "communication/nlsr_dm.hpp"
#include "nlsr_lsdb.hpp"
#include "nlsr_sm.hpp"
#include "route/nlsr_rt.hpp"
#include "route/nlsr_npt.hpp"
#include "route/nlsr_fib.hpp"
#include "utility/nlsr_logger.hpp"
#include "security/nlsr_km.hpp"
#include "communication/nlsr_slh.hpp"


namespace nlsr
{

  using namespace ndn;
  using namespace std;

  class Nlsr
  {
  public:
    Nlsr()
      : io(ndn::make_shared<boost::asio::io_service>())
      , nlsrFace(make_shared<ndn::Face>(io))
      , scheduler(*io)
      , confParam()
      , adl()
      , npl()
      , im()
      , dm()
      , sm()
      , km()
      , isDaemonProcess(false)
      , configFileName("nlsr.conf")
      , nlsrLsdb()
      , adjBuildCount(0)
      , isBuildAdjLsaSheduled(0)
      , isRouteCalculationScheduled(0)
      , isRoutingTableCalculating(0)
      , routingTable()
      , npt()
      , fib()
      , slh(io)
      , nlsrLogger()
    {}

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

    ConfParameter& getConfParameter()
    {
      return confParam;
    }

    Adl& getAdl()
    {
      return adl;
    }

    Npl& getNpl()
    {
      return npl;
    }

    ndn::shared_ptr<boost::asio::io_service>& getIo()
    {
      return io;
    }

    ndn::Scheduler& getScheduler()
    {
      return scheduler;
    }

    ndn::shared_ptr<ndn::Face> getNlsrFace()
    {
      return nlsrFace;
    }

    KeyManager& getKeyManager()
    {
      return km;
    }


    interestManager& getIm()
    {
      return im;
    }

    DataManager& getDm()
    {
      return dm;
    }

    SequencingManager& getSm()
    {
      return sm;
    }

    Lsdb& getLsdb()
    {
      return nlsrLsdb;
    }

    RoutingTable& getRoutingTable()
    {
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

    SyncLogicHandler& getSlh()
    {
      return slh;
    }

    NlsrLogger& getNlsrLogger()
    {
      return nlsrLogger;
    }

    void initNlsr();

  private:
    ConfParameter confParam;
    Adl adl;
    Npl npl;
    ndn::shared_ptr<boost::asio::io_service> io;
    ndn::Scheduler scheduler;
    ndn::shared_ptr<ndn::Face> nlsrFace;
    interestManager im;
    DataManager dm;
    SequencingManager sm;
    KeyManager km;
    bool isDaemonProcess;
    string configFileName;
    int apiPort;

    Lsdb nlsrLsdb;
    RoutingTable routingTable;
    Npt npt;
    Fib fib;
    SyncLogicHandler slh;
    NlsrLogger nlsrLogger;

    long int adjBuildCount;
    int isBuildAdjLsaSheduled;
    int isRouteCalculationScheduled;
    int isRoutingTableCalculating;



  };

} //namespace nlsr

#endif
