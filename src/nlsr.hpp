#ifndef NLSR_HPP
#define NLSR_HPP

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/scheduler.hpp>

#include "conf-parameter.hpp"
#include "adjacency-list.hpp"
#include "name-prefix-list.hpp"
#include "communication/interest-manager.hpp"
#include "communication/data-manager.hpp"
#include "lsdb.hpp"
#include "sequencing-manager.hpp"
#include "route/routing-table.hpp"
#include "route/name-prefix-table.hpp"
#include "route/fib.hpp"
// #include "security/key-manager.hpp"
#include "communication/sync-logic-handler.hpp"


namespace nlsr {

inline static void
NullDeleter(boost::asio::io_service* variable)
{
  // do nothing
}

class Nlsr
{
public:
  Nlsr()
    : m_io(new boost::asio::io_service)
    , m_nlsrFace(new Face(ndn::shared_ptr<boost::asio::io_service>(&*m_io,
                                                                   &NullDeleter)))
    , m_scheduler(*m_io)
    , m_confParam()
    , m_adjacencyList()
    , m_namePrefixList()
    , m_interestManager(*this)
    , m_dataManager(*this)
    , m_sequencingManager()
    // , m_km()
    , m_isDaemonProcess(false)
    , m_configFileName("nlsr.conf")
    , m_nlsrLsdb()
    , m_adjBuildCount(0)
    , m_isBuildAdjLsaSheduled(false)
    , m_isRouteCalculationScheduled(false)
    , m_isRoutingTableCalculating(false)
    , m_routingTable()
    , m_namePrefixTable()
    , m_fib()
    , m_syncLogicHandler(m_io)
  {}

  void
  registrationFailed(const ndn::Name& name);

  void
  setInterestFilterNlsr(const string& name);

  void
  startEventLoop();

  int
  usage(const string& progname);

  std::string
  getConfFileName()
  {
    return m_configFileName;
  }

  void
  setConfFileName(const string& fileName)
  {
    m_configFileName = fileName;
  }

  bool
  getIsSetDaemonProcess()
  {
    return m_isDaemonProcess;
  }

  void
  setIsDaemonProcess(bool value)
  {
    m_isDaemonProcess = value;
  }

  ConfParameter&
  getConfParameter()
  {
    return m_confParam;
  }

  AdjacencyList&
  getAdjacencyList()
  {
    return m_adjacencyList;
  }

  NamePrefixList&
  getNamePrefixList()
  {
    return m_namePrefixList;
  }

  ndn::shared_ptr<boost::asio::io_service>&
  getIo()
  {
    return m_io;
  }

  ndn::Scheduler&
  getScheduler()
  {
    return m_scheduler;
  }

  ndn::shared_ptr<ndn::Face>
  getNlsrFace()
  {
    return m_nlsrFace;
  }

  // KeyManager&
  // getKeyManager()
  // {
  //   return m_km;
  // }


  InterestManager&
  getInterestManager()
  {
    return m_interestManager;
  }

  DataManager&
  getDataManager()
  {
    return m_dataManager;
  }

  SequencingManager&
  getSequencingManager()
  {
    return m_sequencingManager;
  }

  Lsdb&
  getLsdb()
  {
    return m_nlsrLsdb;
  }

  RoutingTable&
  getRoutingTable()
  {
    return m_routingTable;
  }

  NamePrefixTable&
  getNamePrefixTable()
  {
    return m_namePrefixTable;
  }

  Fib&
  getFib()
  {
    return m_fib;
  }

  long int
  getAdjBuildCount()
  {
    return m_adjBuildCount;
  }

  void
  incrementAdjBuildCount()
  {
    m_adjBuildCount++;
  }

  void
  setAdjBuildCount(long int abc)
  {
    m_adjBuildCount = abc;
  }

  int
  getIsBuildAdjLsaSheduled()
  {
    return m_isBuildAdjLsaSheduled;
  }

  void
  setIsBuildAdjLsaSheduled(bool iabls)
  {
    m_isBuildAdjLsaSheduled = iabls;
  }


  void
  setApiPort(int ap)
  {
    m_apiPort = ap;
  }

  int
  getApiPort()
  {
    return m_apiPort;
  }

  bool
  getIsRoutingTableCalculating()
  {
    return m_isRoutingTableCalculating;
  }

  void
  setIsRoutingTableCalculating(bool irtc)
  {
    m_isRoutingTableCalculating = irtc;
  }

  bool
  getIsRouteCalculationScheduled()
  {
    return m_isRouteCalculationScheduled;
  }

  void
  setIsRouteCalculationScheduled(bool ircs)
  {
    m_isRouteCalculationScheduled = ircs;
  }

  SyncLogicHandler&
  getSyncLogicHandler()
  {
    return m_syncLogicHandler;
  }

  void
  initialize();

private:
  ndn::shared_ptr<boost::asio::io_service> m_io;
  ndn::shared_ptr<ndn::Face> m_nlsrFace;
  ndn::Scheduler m_scheduler;
  ConfParameter m_confParam;
  AdjacencyList m_adjacencyList;
  NamePrefixList m_namePrefixList;
  InterestManager m_interestManager;
  DataManager m_dataManager;
  SequencingManager m_sequencingManager;
  // KeyManager m_km;
  bool m_isDaemonProcess;
  string m_configFileName;


  Lsdb m_nlsrLsdb;


  long int m_adjBuildCount;
  bool m_isBuildAdjLsaSheduled;
  bool m_isRouteCalculationScheduled;
  bool m_isRoutingTableCalculating;

  RoutingTable m_routingTable;
  NamePrefixTable m_namePrefixTable;
  Fib m_fib;
  SyncLogicHandler m_syncLogicHandler;

  int m_apiPort;


};

} //namespace nlsr

#endif //NLSR_HPP
