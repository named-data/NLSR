#ifndef NLSR_HPP
#define NLSR_HPP

#include <boost/cstdint.hpp>
#include <stdexcept>

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

class Nlsr
{
  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

public:
  Nlsr()
    : m_scheduler(m_nlsrFace.getIoService())
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
    , m_fib(m_nlsrFace)
    , m_syncLogicHandler(m_nlsrFace.getIoService())
  {}

  void
  registrationFailed(const ndn::Name& name);

  void
  setInterestFilter(const std::string& name);

  void
  startEventLoop();

  void
  usage(const std::string& progname);

  std::string
  getConfFileName() const
  {
    return m_configFileName;
  }

  void
  setConfFileName(const std::string& fileName)
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

  ndn::Scheduler&
  getScheduler()
  {
    return m_scheduler;
  }

  ndn::Face&
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
  setAdjBuildCount(int64_t abc)
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
  setApiPort(int32_t ap)
  {
    m_apiPort = ap;
  }

  int32_t
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
  ndn::Face m_nlsrFace;
  ndn::Scheduler m_scheduler;
  ConfParameter m_confParam;
  AdjacencyList m_adjacencyList;
  NamePrefixList m_namePrefixList;
  InterestManager m_interestManager;
  DataManager m_dataManager;
  SequencingManager m_sequencingManager;
  // KeyManager m_km;
  bool m_isDaemonProcess;
  std::string m_configFileName;
  Lsdb m_nlsrLsdb;
  int64_t m_adjBuildCount;
  bool m_isBuildAdjLsaSheduled;
  bool m_isRouteCalculationScheduled;
  bool m_isRoutingTableCalculating;
  RoutingTable m_routingTable;
  NamePrefixTable m_namePrefixTable;
  Fib m_fib;
  SyncLogicHandler m_syncLogicHandler;
  int32_t m_apiPort;
};

} //namespace nlsr

#endif //NLSR_HPP
