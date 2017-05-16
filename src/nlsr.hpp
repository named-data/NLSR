/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
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
 **/

#ifndef NLSR_NLSR_HPP
#define NLSR_NLSR_HPP

#include <boost/cstdint.hpp>
#include <stdexcept>
#include <boost/throw_exception.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/certificate-cache-ttl.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/mgmt/nfd/face-event-notification.hpp>
#include <ndn-cxx/mgmt/nfd/face-monitor.hpp>
#include <ndn-cxx/mgmt/dispatcher.hpp>

#include "adjacency-list.hpp"
#include "common.hpp"
#include "conf-parameter.hpp"
#include "hello-protocol.hpp"
#include "lsdb.hpp"
#include "name-prefix-list.hpp"
#include "test-access-control.hpp"
#include "validator.hpp"
#include "publisher/lsdb-dataset-interest-handler.hpp"
#include "route/fib.hpp"
#include "route/name-prefix-table.hpp"
#include "route/routing-table.hpp"
#include "security/certificate-store.hpp"
#include "update/prefix-update-processor.hpp"
#include "update/nfd-rib-command-processor.hpp"
#include "utility/name-helper.hpp"

namespace nlsr {

static ndn::Name DEFAULT_BROADCAST_PREFIX("/ndn/broadcast");

class Nlsr
{
  friend class NlsrRunner;

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
  Nlsr(boost::asio::io_service& ioService, ndn::Scheduler& scheduler, ndn::Face& face, ndn::KeyChain& keyChain);

  void
  registrationFailed(const ndn::Name& name);

  void
  onRegistrationSuccess(const ndn::Name& name);

  void
  onLocalhostRegistrationSuccess(const ndn::Name& name);

  void
  setInfoInterestFilter();

  void
  setLsaInterestFilter();

  void
  startEventLoop();

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

  ndn::Face&
  getNlsrFace()
  {
    return m_nlsrFace;
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

  LsdbDatasetInterestHandler&
  getLsdbDatasetHandler()
  {
    return m_lsdbDatasetHandler;
  }

  void
  initialize();

  void
  initializeKey();

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
  loadCertToPublish(std::shared_ptr<ndn::IdentityCertificate> certificate)
  {
    m_certStore.insert(certificate);
  }

  std::shared_ptr<const ndn::IdentityCertificate>
  getCertificate(const ndn::Name& certificateNameWithoutVersion)
  {
    shared_ptr<const ndn::IdentityCertificate> cert =
      m_certStore.find(certificateNameWithoutVersion);

    if (cert != nullptr) {
      return cert;
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

  update::PrefixUpdateProcessor&
  getPrefixUpdateProcessor()
  {
    return m_prefixUpdateProcessor;
  }

  update::NfdRibCommandProcessor&
  getNfdRibCommandProcessor()
  {
    return m_nfdRibCommandProcessor;
  }

  ndn::mgmt::Dispatcher&
  getDispatcher()
  {
    return m_dispatcher;
  }

  void
  createFace(const std::string& faceUri,
             const CommandSucceedCallback& onSuccess,
             const CommandFailCallback& onFailure);

  void
  destroyFaces();

  void
  setStrategies();

  void
  daemonize();

  uint32_t
  getFirstHelloInterval() const
  {
    return m_firstHelloInterval;
  }

  /**
   * \brief Canonize the URI for this and all proceeding neighbors in a list.
   *
   * This function canonizes the URI of the Adjacent object pointed to
   * by currentNeighbor. It then executes the then callback, providing
   * the next iterator in the list to the callback. A standard
   * invocation would be to pass the begin() iterator of NLSR's
   * adjacency list, and to provide Nlsr::canonizeContinuation as the
   * callback. Because every URI must be canonical before we begin
   * operations, the canonize function must call initialize itself.
   *
   * \sa Nlsr::canonizeContinuation
   * \sa Nlsr::initialize
   * \sa NlsrRunner::run
   */
  void
  canonizeNeighborUris(std::list<Adjacent>::iterator currentNeighbor,
                       std::function<void(std::list<Adjacent>::iterator)> then);


PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  void
  addCertificateToCache(std::shared_ptr<ndn::IdentityCertificate> certificate)
  {
    if (certificate != nullptr) {
      m_certificateCache->insertCertificate(certificate);
    }
  }

  security::CertificateStore&
  getCertificateStore()
  {
    return m_certStore;
  }

private:
  void
  registerKeyPrefix();

  void
  registerLocalhostPrefix();

  void
  onKeyInterest(const ndn::Name& name, const ndn::Interest& interest);

  void
  onKeyPrefixRegSuccess(const ndn::Name& name);

  void
  onDestroyFaceSuccess(const ndn::nfd::ControlParameters& commandSuccessResult);

  void
  onDestroyFaceFailure(const ndn::nfd::ControlResponse& response);

  void
  onFaceEventNotification(const ndn::nfd::FaceEventNotification& faceEventNotification);

  void
  setFirstHelloInterval(uint32_t interval)
  {
    m_firstHelloInterval = interval;
  }

  /**
   * \brief Continues canonizing neighbor URIs.
   *
   * For testability reasons, we want what each instance of
   * canonization does after completion to be controllable. The best
   * way to do this is to control that by simply passing a
   * continuation function.
   */
  void
  canonizeContinuation(std::list<Adjacent>::iterator iterator);

public:
  static const ndn::Name LOCALHOST_PREFIX;

private:
  ndn::Face& m_nlsrFace;
  ndn::Scheduler& m_scheduler;
  ndn::KeyChain& m_keyChain;
  ConfParameter m_confParam;
  AdjacencyList m_adjacencyList;
  NamePrefixList m_namePrefixList;
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
  LsdbDatasetInterestHandler m_lsdbDatasetHandler;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  HelloProtocol m_helloProtocol;

private:
  std::shared_ptr<ndn::CertificateCacheTtl> m_certificateCache;
  security::CertificateStore m_certStore;
  Validator m_validator;
  ndn::security::SigningInfo m_signingInfo;
  ndn::Name m_defaultCertName;
  update::PrefixUpdateProcessor m_prefixUpdateProcessor;
  ndn::mgmt::Dispatcher m_dispatcher;
  update::NfdRibCommandProcessor m_nfdRibCommandProcessor;

  ndn::nfd::FaceMonitor m_faceMonitor;

  uint32_t m_firstHelloInterval;
};

} // namespace nlsr

#endif // NLSR_NLSR_HPP
