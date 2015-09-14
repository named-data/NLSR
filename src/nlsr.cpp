/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
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

#include <cstdlib>
#include <string>
#include <sstream>
#include <cstdio>
#include <unistd.h>

#include "nlsr.hpp"
#include "adjacent.hpp"
#include "logger.hpp"


namespace nlsr {

INIT_LOGGER("nlsr");

const ndn::Name Nlsr::LOCALHOST_PREFIX = ndn::Name("/localhost/nlsr");

using namespace ndn;
using namespace std;

Nlsr::Nlsr(boost::asio::io_service& ioService, ndn::Scheduler& scheduler, ndn::Face& face)
  : m_nlsrFace(face)
  , m_scheduler(scheduler)
  , m_confParam()
  , m_adjacencyList()
  , m_namePrefixList()
  , m_sequencingManager()
  , m_isDaemonProcess(false)
  , m_configFileName("nlsr.conf")
  , m_nlsrLsdb(*this, scheduler, m_syncLogicHandler)
  , m_adjBuildCount(0)
  , m_isBuildAdjLsaSheduled(false)
  , m_isRouteCalculationScheduled(false)
  , m_isRoutingTableCalculating(false)
  , m_routingTable(scheduler)
  , m_fib(m_nlsrFace, scheduler, m_adjacencyList, m_confParam, m_keyChain)
  , m_namePrefixTable(*this)
  , m_syncLogicHandler(m_nlsrFace, m_nlsrLsdb, m_confParam, m_sequencingManager)
  , m_helloProtocol(*this, scheduler)
  , m_lsdbDatasetHandler(m_nlsrLsdb,
                         m_nlsrFace,
                         m_confParam.getRouterPrefix(),
                         m_keyChain)
  , m_certificateCache(new ndn::CertificateCacheTtl(ioService))
  , m_validator(m_nlsrFace, DEFAULT_BROADCAST_PREFIX, m_certificateCache, m_certStore)
  , m_prefixUpdateProcessor(m_nlsrFace,
                            m_namePrefixList,
                            m_nlsrLsdb,
                            m_syncLogicHandler,
                            DEFAULT_BROADCAST_PREFIX,
                            m_keyChain,
                            m_certificateCache,
                            m_certStore)
  , m_faceMonitor(m_nlsrFace)
  , m_firstHelloInterval(FIRST_HELLO_INTERVAL_DEFAULT)
{
  m_faceMonitor.onNotification.connect(bind(&Nlsr::onFaceEventNotification, this, _1));
  m_faceMonitor.start();
}

void
Nlsr::registrationFailed(const ndn::Name& name)
{
  std::cerr << "ERROR: Failed to register prefix in local hub's daemon" << endl;
  throw Error("Error: Prefix registration failed");
}

void
Nlsr::onRegistrationSuccess(const ndn::Name& name)
{
  _LOG_DEBUG("Successfully registered prefix: " << name);

  if (name.equals(m_confParam.getRouterPrefix())) {
    m_lsdbDatasetHandler.startListeningOnRouterPrefix();
  }
}

void
Nlsr::onLocalhostRegistrationSuccess(const ndn::Name& name)
{
  _LOG_DEBUG("Successfully registered prefix: " << name);

  m_prefixUpdateProcessor.startListening();
  m_lsdbDatasetHandler.startListeningOnLocalhost();
}

void
Nlsr::setInfoInterestFilter()
{
  ndn::Name name(m_confParam.getRouterPrefix());
  _LOG_DEBUG("Setting interest filter for name: " << name);
  getNlsrFace().setInterestFilter(name,
                                  std::bind(&HelloProtocol::processInterest,
                                            &m_helloProtocol, _1, _2),
                                  std::bind(&Nlsr::onRegistrationSuccess, this, _1),
                                  std::bind(&Nlsr::registrationFailed, this, _1),
                                  m_signingInfo,
                                  ndn::nfd::ROUTE_FLAG_CAPTURE);
}

void
Nlsr::setLsaInterestFilter()
{
  ndn::Name name = m_confParam.getLsaPrefix();
  name.append(m_confParam.getSiteName());
  name.append(m_confParam.getRouterName());
  _LOG_DEBUG("Setting interest filter for name: " << name);
  getNlsrFace().setInterestFilter(name,
                                  std::bind(&Lsdb::processInterest,
                                            &m_nlsrLsdb, _1, _2),
                                  std::bind(&Nlsr::onRegistrationSuccess, this, _1),
                                  std::bind(&Nlsr::registrationFailed, this, _1),
                                  m_signingInfo,
                                  ndn::nfd::ROUTE_FLAG_CAPTURE);
}

void
Nlsr::setStrategies()
{
  const std::string strategy("ndn:/localhost/nfd/strategy/multicast");

  ndn::Name broadcastKeyPrefix = DEFAULT_BROADCAST_PREFIX;
  broadcastKeyPrefix.append("KEYS");

  m_fib.setStrategy(m_confParam.getLsaPrefix(), strategy, 0);
  m_fib.setStrategy(broadcastKeyPrefix, strategy, 0);
  m_fib.setStrategy(m_confParam.getChronosyncPrefix(), strategy, 0);
}

void
Nlsr::daemonize()
{
  pid_t process_id = 0;
  pid_t sid = 0;
  process_id = fork();
  if (process_id < 0){
    std::cerr << "Daemonization failed!" << std::endl;
    throw Error("Error: Daemonization process- fork failed!");
  }
  if (process_id > 0) {
    _LOG_DEBUG("Process daemonized. Process id: " << process_id);
    exit(0);
  }

  umask(0);
  sid = setsid();
  if(sid < 0) {
    throw Error("Error: Daemonization process- setting id failed!");
  }

  if (chdir("/") < 0) {
    throw Error("Error: Daemonization process-chdir failed!");
  }
}

void
Nlsr::initialize()
{
  _LOG_DEBUG("Initializing Nlsr");
  m_confParam.buildRouterPrefix();
  m_nlsrLsdb.setLsaRefreshTime(ndn::time::seconds(m_confParam.getLsaRefreshTime()));
  m_nlsrLsdb.setThisRouterPrefix(m_confParam.getRouterPrefix().toUri());
  m_fib.setEntryRefreshTime(2 * m_confParam.getLsaRefreshTime());
  m_sequencingManager.setSeqFileName(m_confParam.getSeqFileDir());
  m_sequencingManager.initiateSeqNoFromFile();

  m_syncLogicHandler.createSyncSocket(m_confParam.getChronosyncPrefix());

  /* Logging start */
  m_confParam.writeLog();
  m_adjacencyList.writeLog();
  m_namePrefixList.writeLog();
  /* Logging end */
  initializeKey();
  setStrategies();
  _LOG_DEBUG("Default NLSR identity: " << m_signingInfo.getSignerName());
  setInfoInterestFilter();
  setLsaInterestFilter();

  // Set event intervals
  setFirstHelloInterval(m_confParam.getFirstHelloInterval());
  m_nlsrLsdb.setAdjLsaBuildInterval(m_confParam.getAdjLsaBuildInterval());
  m_routingTable.setRoutingCalcInterval(m_confParam.getRoutingCalcInterval());

  m_nlsrLsdb.buildAndInstallOwnNameLsa();
  m_nlsrLsdb.buildAndInstallOwnCoordinateLsa();

  registerKeyPrefix();
  registerLocalhostPrefix();

  m_helloProtocol.scheduleInterest(m_firstHelloInterval);

  // Need to set direct neighbors' costs to 0 for hyperbolic routing
  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {

    std::list<Adjacent>& neighbors = m_adjacencyList.getAdjList();

    for (std::list<Adjacent>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
      it->setLinkCost(0);
    }
  }
}

void
Nlsr::initializeKey()
{
  ndn::Name defaultIdentity = m_confParam.getRouterPrefix();
  defaultIdentity.append("NLSR");

  try {
    m_keyChain.deleteIdentity(defaultIdentity);
  }
  catch (std::exception& e) {
  }
  m_signingInfo = ndn::security::SigningInfo(ndn::security::SigningInfo::SIGNER_TYPE_ID, defaultIdentity);

  ndn::Name keyName = m_keyChain.generateRsaKeyPairAsDefault(defaultIdentity, true);

  ndn::shared_ptr<ndn::IdentityCertificate> certificate =
    ndn::make_shared<ndn::IdentityCertificate>();
  ndn::shared_ptr<ndn::PublicKey> pubKey = m_keyChain.getPublicKey(keyName);
  Name certificateName = keyName.getPrefix(-1);
  certificateName.append("KEY").append(keyName.get(-1)).append("ID-CERT").appendVersion();
  certificate->setName(certificateName);
  certificate->setNotBefore(time::system_clock::now() - time::days(1));
  certificate->setNotAfter(time::system_clock::now() + time::days(7300)); // ~20 years
  certificate->setPublicKeyInfo(*pubKey);
  certificate->addSubjectDescription(CertificateSubjectDescription(ndn::oid::ATTRIBUTE_NAME,
                                                                   keyName.toUri()));
  certificate->encode();
  m_keyChain.signByIdentity(*certificate, m_confParam.getRouterPrefix());

  m_keyChain.addCertificateAsIdentityDefault(*certificate);
  loadCertToPublish(certificate);

  m_defaultCertName = certificate->getName();
}

void
Nlsr::registerKeyPrefix()
{
  ndn::Name keyPrefix = DEFAULT_BROADCAST_PREFIX;
  keyPrefix.append("KEYS");
  m_nlsrFace.setInterestFilter(keyPrefix,
                               std::bind(&Nlsr::onKeyInterest,
                                         this, _1, _2),
                               std::bind(&Nlsr::onKeyPrefixRegSuccess, this, _1),
                               std::bind(&Nlsr::registrationFailed, this, _1),
                               m_signingInfo,
                               ndn::nfd::ROUTE_FLAG_CAPTURE);

}

void
Nlsr::registerLocalhostPrefix()
{
  _LOG_TRACE("Registering prefix: " << LOCALHOST_PREFIX);

  m_nlsrFace.registerPrefix(LOCALHOST_PREFIX,
                            std::bind(&Nlsr::onLocalhostRegistrationSuccess, this, _1),
                            std::bind(&Nlsr::registrationFailed, this, _1));
}

void
Nlsr::onKeyInterest(const ndn::Name& name, const ndn::Interest& interest)
{
  const ndn::Name& interestName = interest.getName();

  ndn::Name certName = interestName.getSubName(name.size());

  if (certName[-2].toUri() == "ID-CERT")
    {
      certName = certName.getPrefix(-1);
    }
  else if (certName[-1].toUri() != "ID-CERT")
    return; //Wrong key interest.

  ndn::shared_ptr<const ndn::IdentityCertificate> cert = getCertificate(certName);

  if (!static_cast<bool>(cert))
    return; // cert is not found

  ndn::shared_ptr<ndn::Data> data = ndn::make_shared<ndn::Data>();
  data->setName(interestName);
  data->setContent(cert->wireEncode());
  m_keyChain.signWithSha256(*data);

  m_nlsrFace.put(*data);
}

void
Nlsr::onKeyPrefixRegSuccess(const ndn::Name& name)
{
}

void
Nlsr::onDestroyFaceSuccess(const ndn::nfd::ControlParameters& commandSuccessResult)
{
}

void
Nlsr::onDestroyFaceFailure(int32_t code, const std::string& error)
{
  std::cerr << error << " (code: " << code << ")";
  throw Error("Error: Face destruction failed");
}

void
Nlsr::destroyFaces()
{
  std::list<Adjacent>& adjacents = m_adjacencyList.getAdjList();
  for (std::list<Adjacent>::iterator it = adjacents.begin();
       it != adjacents.end(); it++) {
    m_fib.destroyFace((*it).getConnectingFaceUri(),
                      std::bind(&Nlsr::onDestroyFaceSuccess, this, _1),
                      std::bind(&Nlsr::onDestroyFaceFailure, this, _1, _2));
  }
}




void
Nlsr::onFaceEventNotification(const ndn::nfd::FaceEventNotification& faceEventNotification)
{
  _LOG_TRACE("Nlsr::onFaceEventNotification called");
  ndn::nfd::FaceEventKind kind = faceEventNotification.getKind();

  if (kind == ndn::nfd::FACE_EVENT_DESTROYED) {
    uint64_t faceId = faceEventNotification.getFaceId();

    Adjacent* adjacent = m_adjacencyList.findAdjacent(faceId);

    if (adjacent != nullptr) {
      _LOG_DEBUG("Face to " << adjacent->getName() << " with face id: " << faceId << " destroyed");

      adjacent->setFaceId(0);

      // Only trigger an Adjacency LSA build if this node is changing from ACTIVE to INACTIVE
      // since this rebuild will effectively cancel the previous Adjacency LSA refresh event
      // and schedule a new one further in the future.
      //
      // Continuously scheduling the refresh in the future will block the router from refreshing
      // its Adjacency LSA. Since other routers' Name prefixes' expiration times are updated
      // when this router refreshes its Adjacency LSA, the other routers' prefixes will expire
      // and be removed from the RIB.
      //
      // This check is required to fix Bug #2733 for now. This check would be unnecessary
      // to fix Bug #2733 when Issue #2732 is completed, but the check also helps with
      // optimization so it can remain even when Issue #2732 is implemented.
      if (adjacent->getStatus() == Adjacent::STATUS_ACTIVE) {
        adjacent->setStatus(Adjacent::STATUS_INACTIVE);

        // A new adjacency LSA cannot be built until the neighbor is marked INACTIVE and
        // has met the HELLO retry threshold
        adjacent->setInterestTimedOutNo(m_confParam.getInterestRetryNumber());

        m_nlsrLsdb.scheduleAdjLsaBuild();
      }
    }
  }
}


void
Nlsr::startEventLoop()
{
  m_nlsrFace.processEvents();
}

} // namespace nlsr
