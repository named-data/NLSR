/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
 *
 * This file is part of NLSR (Named-data Link State Routing).
 * See AUTHORS.md for complete list of NLSR authors and contributors.
 *
 * NLSR is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#ifndef NLSR_HPP
#define NLSR_HPP

#include <boost/cstdint.hpp>
#include <stdexcept>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/certificate-cache-ttl.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/management/nfd-face-event-notification.hpp>

#include "conf-parameter.hpp"
#include "adjacency-list.hpp"
#include "name-prefix-list.hpp"
#include "lsdb.hpp"
#include "sequencing-manager.hpp"
#include "route/routing-table.hpp"
#include "route/name-prefix-table.hpp"
#include "route/fib.hpp"
#include "communication/sync-logic-handler.hpp"
#include "hello-protocol.hpp"
#include "face-monitor.hpp"

#include "validator.hpp"


namespace nlsr {

static ndn::Name DEFAULT_BROADCAST_PREFIX("/ndn/broadcast");

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
    , m_sequencingManager()
    , m_isDaemonProcess(false)
    , m_configFileName("nlsr.conf")
    , m_nlsrLsdb(*this)
    , m_adjBuildCount(0)
    , m_isBuildAdjLsaSheduled(false)
    , m_isRouteCalculationScheduled(false)
    , m_isRoutingTableCalculating(false)
    , m_routingTable()
    , m_fib(*this, m_nlsrFace)
    , m_namePrefixTable(*this)
    , m_syncLogicHandler(m_nlsrFace.getIoService())
    , m_helloProtocol(*this)

    , m_certificateCache(new ndn::CertificateCacheTtl(m_nlsrFace.getIoService()))
    , m_validator(m_nlsrFace, DEFAULT_BROADCAST_PREFIX, m_certificateCache)

    , m_faceMonitor(m_nlsrFace.getIoService(),
                    ndn::bind(&Nlsr::onFaceEventNotification, this, _1))
  {}

  void
  registrationFailed(const ndn::Name& name);

  void
  onRegistrationSuccess(const ndn::Name& name);

  void
  setInfoInterestFilter();

  void
  setLsaInterestFilter();

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

  bool
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

  void
  intializeKey();

  void
  loadValidator(boost::property_tree::ptree section,
                const std::string& filename)
  {
    m_validator.load(section, filename);
  }

  Validator&
  getValidator()
  {
    return m_validator;
  }

  void
  loadCertToPublish(ndn::shared_ptr<ndn::IdentityCertificate> certificate)
  {
    if (static_cast<bool>(certificate))
      m_certToPublish[certificate->getName().getPrefix(-1)] = certificate; // key is cert name
                                                                           // without version
  }

  ndn::shared_ptr<const ndn::IdentityCertificate>
  getCertificate(const ndn::Name& certificateNameWithoutVersion)
  {
    CertMap::iterator it = m_certToPublish.find(certificateNameWithoutVersion);

    if (it != m_certToPublish.end())
      {
        return it->second;
      }

    return m_certificateCache->getCertificate(certificateNameWithoutVersion);
  }

  ndn::KeyChain&
  getKeyChain()
  {
    return m_keyChain;
  }

  const ndn::Name&
  getDefaultCertName()
  {
    return m_defaultCertName;
  }

  void
  createFace(const std::string& faceUri,
             const CommandSucceedCallback& onSuccess,
             const CommandFailCallback& onFailure);

  void
  destroyFaces();

  void
  setStrategies();

private:
  void
  registerKeyPrefix();

  void
  onKeyInterest(const ndn::Name& name, const ndn::Interest& interest);

  void
  onKeyPrefixRegSuccess(const ndn::Name& name);

  void
  onDestroyFaceSuccess(const ndn::nfd::ControlParameters& commandSuccessResult);

  void
  onDestroyFaceFailure(int32_t code, const std::string& error);

  void
  onFaceEventNotification(const ndn::nfd::FaceEventNotification& faceEventNotification);

private:
  typedef std::map<ndn::Name, ndn::shared_ptr<ndn::IdentityCertificate> > CertMap;

  ndn::Face m_nlsrFace;
  ndn::Scheduler m_scheduler;
  ConfParameter m_confParam;
  AdjacencyList m_adjacencyList;
  NamePrefixList m_namePrefixList;
  SequencingManager m_sequencingManager;
  bool m_isDaemonProcess;
  std::string m_configFileName;
  Lsdb m_nlsrLsdb;
  int64_t m_adjBuildCount;
  bool m_isBuildAdjLsaSheduled;
  bool m_isRouteCalculationScheduled;
  bool m_isRoutingTableCalculating;
  RoutingTable m_routingTable;
  Fib m_fib;
  NamePrefixTable m_namePrefixTable;
  SyncLogicHandler m_syncLogicHandler;
  int32_t m_apiPort;
  HelloProtocol m_helloProtocol;

  ndn::shared_ptr<ndn::CertificateCacheTtl> m_certificateCache;
  CertMap m_certToPublish;
  Validator m_validator;
  ndn::KeyChain m_keyChain;
  ndn::Name m_defaultIdentity;
  ndn::Name m_defaultCertName;

  FaceMonitor m_faceMonitor;
};

} //namespace nlsr

#endif //NLSR_HPP
