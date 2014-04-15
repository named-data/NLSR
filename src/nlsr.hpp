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
      : m_io(ndn::make_shared<boost::asio::io_service>())
      , m_nlsrFace(make_shared<ndn::Face>(m_io))
      , m_scheduler(*m_io)
      , m_confParam()
      , m_adl()
      , m_npl()
      , m_im()
      , m_dm()
      , m_sm()
      , m_km()
      , isDaemonProcess(false)
      , m_configFileName("nlsr.conf")
      , m_nlsrLsdb()
      , m_adjBuildCount(0)
      , isBuildAdjLsaSheduled(0)
      , isRouteCalculationScheduled(0)
      , isRoutingTableCalculating(0)
      , m_routingTable()
      , m_npt()
      , m_fib()
      , m_slh(m_io)
      , m_nlsrLogger()
    {}

    void nlsrRegistrationFailed(const ndn::Name& name);

    void setInterestFilterNlsr(const string& name);
    void startEventLoop();

    int usage(const string& progname);

    string getConfFileName()
    {
      return m_configFileName;
    }

    void setConfFileName(const string& fileName)
    {
      m_configFileName=fileName;
    }

    bool getIsSetDaemonProcess()
    {
      return isDaemonProcess;
    }

    void setIsDaemonProcess(bool value)
    {
      isDaemonProcess=value;
    }

    ConfParameter& getConfParameter()
    {
      return m_confParam;
    }

    Adl& getAdl()
    {
      return m_adl;
    }

    Npl& getNpl()
    {
      return m_npl;
    }

    ndn::shared_ptr<boost::asio::io_service>& getIo()
    {
      return m_io;
    }

    ndn::Scheduler& getScheduler()
    {
      return m_scheduler;
    }

    ndn::shared_ptr<ndn::Face> getNlsrFace()
    {
      return m_nlsrFace;
    }

    KeyManager& getKeyManager()
    {
      return m_km;
    }


    InterestManager& getIm()
    {
      return m_im;
    }

    DataManager& getDm()
    {
      return m_dm;
    }

    SequencingManager& getSm()
    {
      return m_sm;
    }

    Lsdb& getLsdb()
    {
      return m_nlsrLsdb;
    }

    RoutingTable& getRoutingTable()
    {
      return m_routingTable;
    }

    Npt& getNpt()
    {
      return m_npt;
    }

    Fib& getFib()
    {
      return m_fib;
    }

    long int getAdjBuildCount()
    {
      return m_adjBuildCount;
    }

    void incrementAdjBuildCount()
    {
      m_adjBuildCount++;
    }

    void setAdjBuildCount(long int abc)
    {
      m_adjBuildCount=abc;
    }

    int getIsBuildAdjLsaSheduled()
    {
      return isBuildAdjLsaSheduled;
    }

    void setIsBuildAdjLsaSheduled(bool iabls)
    {
      isBuildAdjLsaSheduled=iabls;
    }


    void setApiPort(int ap)
    {
      m_apiPort=ap;
    }

    int getApiPort()
    {
      return m_apiPort;
    }

    bool getIsRoutingTableCalculating()
    {
      return isRoutingTableCalculating;
    }

    void setIsRoutingTableCalculating(bool irtc)
    {
      isRoutingTableCalculating=irtc;
    }

    bool getIsRouteCalculationScheduled()
    {
      return isRouteCalculationScheduled;
    }

    void setIsRouteCalculationScheduled(bool ircs)
    {
      isRouteCalculationScheduled=ircs;
    }

    SyncLogicHandler& getSlh()
    {
      return m_slh;
    }

    NlsrLogger& getNlsrLogger()
    {
      return m_nlsrLogger;
    }

    void initialize();

  private:
    ConfParameter m_confParam;
    Adl m_adl;
    Npl m_npl;
    ndn::shared_ptr<boost::asio::io_service> m_io;
    ndn::Scheduler m_scheduler;
    ndn::shared_ptr<ndn::Face> m_nlsrFace;
    InterestManager m_im;
    DataManager m_dm;
    SequencingManager m_sm;
    KeyManager m_km;
    bool isDaemonProcess;
    string m_configFileName;
    int m_apiPort;

    Lsdb m_nlsrLsdb;
    RoutingTable m_routingTable;
    Npt m_npt;
    Fib m_fib;
    SyncLogicHandler m_slh;
    NlsrLogger m_nlsrLogger;

    long int m_adjBuildCount;
    bool isBuildAdjLsaSheduled;
    bool isRouteCalculationScheduled;
    bool isRoutingTableCalculating;



  };

} //namespace nlsr

#endif
