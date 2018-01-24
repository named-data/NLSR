/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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

#include "nlsr.hpp"
#include "adjacent.hpp"
#include "logger.hpp"

#include <cstdlib>
#include <string>
#include <sstream>
#include <cstdio>
#include <unistd.h>

#include <ndn-cxx/net/face-uri.hpp>
#include <ndn-cxx/signature.hpp>

namespace nlsr {

INIT_LOGGER(Nlsr);

const ndn::Name Nlsr::LOCALHOST_PREFIX = ndn::Name("/localhost/nlsr");

Nlsr::Nlsr(boost::asio::io_service& ioService, ndn::Scheduler& scheduler, ndn::Face& face, ndn::KeyChain& keyChain)
  : m_nlsrFace(face)
  , m_scheduler(scheduler)
  , m_keyChain(keyChain)
  , m_confParam()
  , m_adjacencyList()
  , m_namePrefixList()
  , m_configFileName("nlsr.conf")
  , m_nlsrLsdb(*this, scheduler)
  , m_adjBuildCount(0)
  , m_isBuildAdjLsaSheduled(false)
  , m_isRouteCalculationScheduled(false)
  , m_isRoutingTableCalculating(false)
  , m_routingTable(scheduler)
  , m_fib(m_nlsrFace, scheduler, m_adjacencyList, m_confParam, m_keyChain)
  , m_namePrefixTable(*this, m_routingTable.afterRoutingChange)
  , m_dispatcher(m_nlsrFace, m_keyChain)
  , m_datasetHandler(m_nlsrLsdb,
                     m_routingTable,
                     m_dispatcher,
                     m_nlsrFace,
                     m_keyChain)
  , m_helloProtocol(*this, scheduler)
  , m_validator(ndn::make_unique<ndn::security::v2::CertificateFetcherDirectFetch>(m_nlsrFace))
  , m_controller(m_nlsrFace, m_keyChain)
  , m_faceDatasetController(m_nlsrFace, m_keyChain)
  , m_prefixUpdateProcessor(m_dispatcher,
                            m_nlsrFace,
                            m_namePrefixList,
                            m_nlsrLsdb)
  , m_nfdRibCommandProcessor(m_dispatcher,
                             m_namePrefixList,
                             m_nlsrLsdb)
  , m_statsCollector(m_nlsrLsdb, m_helloProtocol)
  , m_faceMonitor(m_nlsrFace)
  , m_firstHelloInterval(FIRST_HELLO_INTERVAL_DEFAULT)
{
  m_faceMonitor.onNotification.connect(std::bind(&Nlsr::onFaceEventNotification, this, _1));
  m_faceMonitor.start();
}

void
Nlsr::registrationFailed(const ndn::Name& name)
{
  NLSR_LOG_ERROR("ERROR: Failed to register prefix in local hub's daemon");
  BOOST_THROW_EXCEPTION(Error("Error: Prefix registration failed"));
}

void
Nlsr::onRegistrationSuccess(const ndn::Name& name)
{
  NLSR_LOG_DEBUG("Successfully registered prefix: " << name);
}

void
Nlsr::setInfoInterestFilter()
{
  ndn::Name name(m_confParam.getRouterPrefix());
  name.append("NLSR");
  name.append("INFO");

  NLSR_LOG_DEBUG("Setting interest filter for Hello interest: " << name);

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

  NLSR_LOG_DEBUG("Setting interest filter for LsaPrefix: " << name);

  getNlsrFace().setInterestFilter(name,
                                  std::bind(&Lsdb::processInterest,
                                            &m_nlsrLsdb, _1, _2),
                                  std::bind(&Nlsr::onRegistrationSuccess, this, _1),
                                  std::bind(&Nlsr::registrationFailed, this, _1),
                                  m_signingInfo,
                                  ndn::nfd::ROUTE_FLAG_CAPTURE);
}


void
Nlsr::addDispatcherTopPrefix(const ndn::Name& topPrefix)
{
  try {
    m_dispatcher.addTopPrefix(topPrefix, false, m_signingInfo);
  }
  catch (const std::exception& e) {
    NLSR_LOG_ERROR("Error setting top-level prefix in dispatcher: " << e.what() << "\n");
  }
}

void
Nlsr::setStrategies()
{
  const std::string strategy("ndn:/localhost/nfd/strategy/multicast");

  m_fib.setStrategy(m_confParam.getLsaPrefix(), strategy, 0);
  m_fib.setStrategy(m_confParam.getChronosyncPrefix(), strategy, 0);
}

void
Nlsr::canonizeContinuation(std::list<Adjacent>::iterator iterator,
                           std::function<void(void)> finally)
{
  canonizeNeighborUris(iterator, [this, finally] (std::list<Adjacent>::iterator iterator) {
      canonizeContinuation(iterator, finally);
    },
    finally);
}

void
Nlsr::canonizeNeighborUris(std::list<Adjacent>::iterator currentNeighbor,
                           std::function<void(std::list<Adjacent>::iterator)> then,
                           std::function<void(void)> finally)
{
  if (currentNeighbor != m_adjacencyList.getAdjList().end()) {
    ndn::FaceUri uri(currentNeighbor->getFaceUri());
    uri.canonize([this, then, currentNeighbor] (ndn::FaceUri canonicalUri) {
        NLSR_LOG_DEBUG("Canonized URI: " << currentNeighbor->getFaceUri()
                   << " to: " << canonicalUri);
        currentNeighbor->setFaceUri(canonicalUri);
        then(std::next(currentNeighbor));
      },
      [this, then, currentNeighbor] (const std::string& reason) {
        NLSR_LOG_ERROR("Could not canonize URI: " << currentNeighbor->getFaceUri()
                   << " because: " << reason);
        then(std::next(currentNeighbor));
      },
      m_nlsrFace.getIoService(),
      TIME_ALLOWED_FOR_CANONIZATION);
  }
  // We have finished canonizing all neighbors, so call finally()
  else {
    finally();
  }
}


void
Nlsr::loadCertToPublish(const ndn::security::v2::Certificate& certificate)
{
  m_certStore.insert(certificate);
  m_validator.loadAnchor("Authoritative-Certificate",
                          ndn::security::v2::Certificate(certificate));
  m_prefixUpdateProcessor.getValidator().
                          loadAnchor("Authoritative-Certificate",
                                      ndn::security::v2::Certificate(certificate));
}

void
Nlsr::initialize()
{
  NLSR_LOG_DEBUG("Initializing Nlsr");
  m_confParam.buildRouterPrefix();
  m_datasetHandler.setRouterNameCommandPrefix(m_confParam.getRouterPrefix());
  m_nlsrLsdb.setLsaRefreshTime(ndn::time::seconds(m_confParam.getLsaRefreshTime()));
  m_nlsrLsdb.setThisRouterPrefix(m_confParam.getRouterPrefix().toUri());
  m_fib.setEntryRefreshTime(2 * m_confParam.getLsaRefreshTime());

  m_nlsrLsdb.getSequencingManager().setSeqFileDirectory(m_confParam.getSeqFileDir());
  m_nlsrLsdb.getSequencingManager().initiateSeqNoFromFile(m_confParam.getHyperbolicState());

  m_nlsrLsdb.getSyncLogicHandler().createSyncSocket(m_confParam.getChronosyncPrefix());

  // Logging start
  m_confParam.writeLog();
  m_adjacencyList.writeLog();
  NLSR_LOG_DEBUG(m_namePrefixList);
  // Logging end

  initializeKey();
  setStrategies();

  NLSR_LOG_DEBUG("Default NLSR identity: " << m_signingInfo.getSignerName());

  setInfoInterestFilter();
  setLsaInterestFilter();

  // add top-level prefixes: router and localhost prefix
  addDispatcherTopPrefix(m_confParam.getRouterPrefix());
  addDispatcherTopPrefix(LOCALHOST_PREFIX);

  initializeFaces(std::bind(&Nlsr::processFaceDataset, this, _1),
                  std::bind(&Nlsr::onFaceDatasetFetchTimeout, this, _1, _2, 0));

  enableIncomingFaceIdIndication();

  // Set event intervals
  setFirstHelloInterval(m_confParam.getFirstHelloInterval());
  m_nlsrLsdb.setAdjLsaBuildInterval(m_confParam.getAdjLsaBuildInterval());
  m_routingTable.setRoutingCalcInterval(m_confParam.getRoutingCalcInterval());

  m_nlsrLsdb.buildAndInstallOwnNameLsa();

  // Install coordinate LSAs if using HR or dry-run HR.
  if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
    m_nlsrLsdb.buildAndInstallOwnCoordinateLsa();
  }

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
  NLSR_LOG_DEBUG("Initializing Key ...");

  ndn::Name nlsrInstanceName = m_confParam.getRouterPrefix();
  nlsrInstanceName.append("NLSR");

  try {
    m_keyChain.deleteIdentity(m_keyChain.getPib().getIdentity(nlsrInstanceName));
  } catch (const std::exception& e) {
    NLSR_LOG_WARN(e.what());
  }

  auto nlsrInstanceIdentity = m_keyChain.createIdentity(nlsrInstanceName);
  auto nlsrInstanceKey = nlsrInstanceIdentity.getDefaultKey();

  ndn::security::v2::Certificate certificate;

  ndn::Name certificateName = nlsrInstanceKey.getName();
  certificateName.append("NA");
  certificateName.appendVersion();
  certificate.setName(certificateName);

  // set metainfo
  certificate.setContentType(ndn::tlv::ContentType_Key);
  certificate.setFreshnessPeriod(ndn::time::days(365));

  // set content
  certificate.setContent(nlsrInstanceKey.getPublicKey().data(), nlsrInstanceKey.getPublicKey().size());

  // set signature-info
  ndn::SignatureInfo signatureInfo;
  signatureInfo.setValidityPeriod(ndn::security::ValidityPeriod(ndn::time::system_clock::TimePoint(),
                                                                ndn::time::system_clock::now()
                                                                + ndn::time::days(365)));
  try {
    m_keyChain.sign(certificate,
                    ndn::security::SigningInfo(m_keyChain.getPib().getIdentity(m_confParam.getRouterPrefix()))
                                               .setSignatureInfo(signatureInfo));
  }
  catch (const std::exception& e) {
    NLSR_LOG_WARN("ERROR: Router's " << e.what()
                  << "NLSR is running without security."
                  << " If security is enabled NLSR will not converge.");

    std::cerr << "Router's " << e.what() << "NLSR is running without security "
              << "(Only for testing, should not be used in production.)"
              << " If security is enabled NLSR will not converge." << std::endl;
  }

  m_signingInfo = ndn::security::SigningInfo(ndn::security::SigningInfo::SIGNER_TYPE_ID,
                                             nlsrInstanceName);

  loadCertToPublish(certificate);

  m_defaultCertName = certificate.getName();
}

void
Nlsr::registerKeyPrefix()
{
  // Start listening for the interest of this router's NLSR certificate
  ndn::Name nlsrKeyPrefix = getConfParameter().getRouterPrefix();
  nlsrKeyPrefix.append("NLSR");
  nlsrKeyPrefix.append("KEY");

  m_nlsrFace.setInterestFilter(nlsrKeyPrefix,
                               std::bind(&Nlsr::onKeyInterest,
                                         this, _1, _2),
                               std::bind(&Nlsr::onKeyPrefixRegSuccess, this, _1),
                               std::bind(&Nlsr::registrationFailed, this, _1),
                               m_signingInfo,
                               ndn::nfd::ROUTE_FLAG_CAPTURE);

  // Start listening for the interest of this router's certificate
  ndn::Name routerKeyPrefix = getConfParameter().getRouterPrefix();
  routerKeyPrefix.append("KEY");

  m_nlsrFace.setInterestFilter(routerKeyPrefix,
                                 std::bind(&Nlsr::onKeyInterest,
                                           this, _1, _2),
                                 std::bind(&Nlsr::onKeyPrefixRegSuccess, this, _1),
                                 std::bind(&Nlsr::registrationFailed, this, _1),
                                 m_signingInfo,
                                 ndn::nfd::ROUTE_FLAG_CAPTURE);

  // Start listening for the interest of this router's operator's certificate
  ndn::Name operatorKeyPrefix = getConfParameter().getNetwork();
  operatorKeyPrefix.append(getConfParameter().getSiteName());
  operatorKeyPrefix.append(std::string("%C1.Operator"));

  m_nlsrFace.setInterestFilter(operatorKeyPrefix,
                               std::bind(&Nlsr::onKeyInterest,
                                         this, _1, _2),
                               std::bind(&Nlsr::onKeyPrefixRegSuccess, this, _1),
                               std::bind(&Nlsr::registrationFailed, this, _1),
                               m_signingInfo,
                               ndn::nfd::ROUTE_FLAG_CAPTURE);

  // Start listening for the interest of this router's site's certificate
  ndn::Name siteKeyPrefix = getConfParameter().getNetwork();
  siteKeyPrefix.append(getConfParameter().getSiteName());
  siteKeyPrefix.append("KEY");

  m_nlsrFace.setInterestFilter(siteKeyPrefix,
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
  m_nlsrFace.registerPrefix(LOCALHOST_PREFIX,
                            std::bind(&Nlsr::onRegistrationSuccess, this, _1),
                            std::bind(&Nlsr::registrationFailed, this, _1));
}

void
Nlsr::onKeyInterest(const ndn::Name& name, const ndn::Interest& interest)
{
  NLSR_LOG_DEBUG("Got interest for certificate. Interest: " << interest.getName());

  const ndn::Name& interestName = interest.getName();
  const ndn::security::v2::Certificate* cert = getCertificate(interestName);

  if (cert == nullptr) {
      NLSR_LOG_DEBUG("Certificate is not found for: " << interest);
      return; // cert is not found
  }

  m_nlsrFace.put(*cert);
}

void
Nlsr::onKeyPrefixRegSuccess(const ndn::Name& name)
{
  NLSR_LOG_DEBUG("KEY prefix: " << name << " registration is successful.");
}

void
Nlsr::onFaceEventNotification(const ndn::nfd::FaceEventNotification& faceEventNotification)
{
  NLSR_LOG_TRACE("Nlsr::onFaceEventNotification called");

  switch (faceEventNotification.getKind()) {
    case ndn::nfd::FACE_EVENT_DESTROYED: {
      uint64_t faceId = faceEventNotification.getFaceId();

      auto adjacent = m_adjacencyList.findAdjacent(faceId);

      if (adjacent != m_adjacencyList.end()) {
        NLSR_LOG_DEBUG("Face to " << adjacent->getName() << " with face id: " << faceId << " destroyed");

        adjacent->setFaceId(0);

        // Only trigger an Adjacency LSA build if this node is changing
        // from ACTIVE to INACTIVE since this rebuild will effectively
        // cancel the previous Adjacency LSA refresh event and schedule
        // a new one further in the future.
        //
        // Continuously scheduling the refresh in the future will block
        // the router from refreshing its Adjacency LSA. Since other
        // routers' Name prefixes' expiration times are updated when
        // this router refreshes its Adjacency LSA, the other routers'
        // prefixes will expire and be removed from the RIB.
        //
        // This check is required to fix Bug #2733 for now. This check
        // would be unnecessary to fix Bug #2733 when Issue #2732 is
        // completed, but the check also helps with optimization so it
        // can remain even when Issue #2732 is implemented.
        if (adjacent->getStatus() == Adjacent::STATUS_ACTIVE) {
          adjacent->setStatus(Adjacent::STATUS_INACTIVE);

          // A new adjacency LSA cannot be built until the neighbor is marked INACTIVE and
          // has met the HELLO retry threshold
          adjacent->setInterestTimedOutNo(m_confParam.getInterestRetryNumber());

          if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
            getRoutingTable().scheduleRoutingTableCalculation(*this);
          }
          else {
            m_nlsrLsdb.scheduleAdjLsaBuild();
          }
        }
      }
      break;
    }
    case ndn::nfd::FACE_EVENT_CREATED: {
      // Find the neighbor in our adjacency list
      ndn::FaceUri faceUri;
      try {
        faceUri = ndn::FaceUri(faceEventNotification.getRemoteUri());
      }
      catch (const std::exception& e) {
        NLSR_LOG_WARN(e.what());
        return;
      }
      auto adjacent = m_adjacencyList.findAdjacent(faceUri);

      // If we have a neighbor by that FaceUri and it has no FaceId, we
      // have a match.
      if (adjacent != m_adjacencyList.end()) {
        NLSR_LOG_DEBUG("Face creation event matches neighbor: " << adjacent->getName()
                   << ". New Face ID: " << faceEventNotification.getFaceId()
                   << ". Registering prefixes.");
        adjacent->setFaceId(faceEventNotification.getFaceId());

        registerAdjacencyPrefixes(*adjacent, ndn::time::milliseconds::max());

        if (m_confParam.getHyperbolicState() != HYPERBOLIC_STATE_OFF) {
          getRoutingTable().scheduleRoutingTableCalculation(*this);
        }
        else {
         m_nlsrLsdb.scheduleAdjLsaBuild();
        }
      }
      break;
    }
    default:
      break;
  }
}

void
Nlsr::initializeFaces(const FetchDatasetCallback& onFetchSuccess,
                      const FetchDatasetTimeoutCallback& onFetchFailure)
{
  NLSR_LOG_TRACE("Initializing Faces...");

  m_faceDatasetController.fetch<ndn::nfd::FaceDataset>(onFetchSuccess, onFetchFailure);

}

void
Nlsr::processFaceDataset(const std::vector<ndn::nfd::FaceStatus>& faces)
{
  NLSR_LOG_DEBUG("Processing face dataset");

  // Iterate over each neighbor listed in nlsr.conf
  for (auto& adjacent : m_adjacencyList.getAdjList()) {

    const std::string faceUriString = adjacent.getFaceUri().toString();
    // Check the list of FaceStatus objects we got for a match
    for (const ndn::nfd::FaceStatus& faceStatus : faces) {
      // Set the adjacency FaceID if we find a URI match and it was
      // previously unset. Change the boolean to true.
      if (adjacent.getFaceId() == 0 && faceUriString == faceStatus.getRemoteUri()) {
        NLSR_LOG_DEBUG("FaceUri: " << faceStatus.getRemoteUri() <<
                   " FaceId: "<< faceStatus.getFaceId());
        adjacent.setFaceId(faceStatus.getFaceId());
        // Register the prefixes for each neighbor
        this->registerAdjacencyPrefixes(adjacent, ndn::time::milliseconds::max());
      }
    }
    // If this adjacency has no information in this dataset, then one
    // of two things is happening: 1. NFD is starting slowly and this
    // Face wasn't ready yet, or 2. NFD is configured
    // incorrectly and this Face isn't available.
    if (adjacent.getFaceId() == 0) {
      NLSR_LOG_WARN("The adjacency " << adjacent.getName() <<
                " has no Face information in this dataset.");
    }
  }

  scheduleDatasetFetch();
}

void
Nlsr::registerAdjacencyPrefixes(const Adjacent& adj,
                                const ndn::time::milliseconds& timeout)
{
  ndn::FaceUri faceUri = adj.getFaceUri();
  double linkCost = adj.getLinkCost();
  const ndn::Name& adjName = adj.getName();

  m_fib.registerPrefix(adjName, faceUri, linkCost,
                       timeout, ndn::nfd::ROUTE_FLAG_CAPTURE, 0);

  m_fib.registerPrefix(m_confParam.getChronosyncPrefix(),
                       faceUri, linkCost, timeout,
                       ndn::nfd::ROUTE_FLAG_CAPTURE, 0);

  m_fib.registerPrefix(m_confParam.getLsaPrefix(),
                       faceUri, linkCost, timeout,
                       ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
}

void
Nlsr::onFaceDatasetFetchTimeout(uint32_t code,
                                const std::string& msg,
                                uint32_t nRetriesSoFar)
{
  NLSR_LOG_DEBUG("onFaceDatasetFetchTimeout");
  // If we have exceeded the maximum attempt count, do not try again.
  if (nRetriesSoFar++ < m_confParam.getFaceDatasetFetchTries()) {
    NLSR_LOG_DEBUG("Failed to fetch dataset: " << msg << ". Attempting retry #" << nRetriesSoFar);
    m_faceDatasetController.fetch<ndn::nfd::FaceDataset>(std::bind(&Nlsr::processFaceDataset,
                                                        this, _1),
                                              std::bind(&Nlsr::onFaceDatasetFetchTimeout,
                                                        this, _1, _2, nRetriesSoFar));
  }
  else {
    NLSR_LOG_ERROR("Failed to fetch dataset: " << msg << ". Exceeded limit of " <<
               m_confParam.getFaceDatasetFetchTries() << ", so not trying again this time.");
    // If we fail to fetch it, just do nothing until the next
    // interval.  Since this is a backup mechanism, we aren't as
    // concerned with retrying.
    scheduleDatasetFetch();
  }
}

void
Nlsr::scheduleDatasetFetch()
{
  NLSR_LOG_DEBUG("Scheduling Dataset Fetch in " << m_confParam.getFaceDatasetFetchInterval());

  m_scheduler.scheduleEvent(m_confParam.getFaceDatasetFetchInterval(),
    [this] {
      this->initializeFaces(
        [this] (const std::vector<ndn::nfd::FaceStatus>& faces) {
         this->processFaceDataset(faces);
        },
        [this] (uint32_t code, const std::string& msg) {
         this->onFaceDatasetFetchTimeout(code, msg, 0);
        });
  });
}

void
Nlsr::enableIncomingFaceIdIndication()
{
  NLSR_LOG_DEBUG("Enabling incoming face id indication for local face.");

  m_controller.start<ndn::nfd::FaceUpdateCommand>(
    ndn::nfd::ControlParameters()
      .setFlagBit(ndn::nfd::FaceFlagBit::BIT_LOCAL_FIELDS_ENABLED, true),
    bind(&Nlsr::onFaceIdIndicationSuccess, this, _1),
    bind(&Nlsr::onFaceIdIndicationFailure, this, _1));
}

void
Nlsr::onFaceIdIndicationSuccess(const ndn::nfd::ControlParameters& cp)
{
  NLSR_LOG_DEBUG("Successfully enabled incoming face id indication"
                 << "for face id " << cp.getFaceId());
}

void
Nlsr::onFaceIdIndicationFailure(const ndn::nfd::ControlResponse& cr)
{
  std::ostringstream os;
  os << "Failed to enable incoming face id indication feature: " <<
        "(code: " << cr.getCode() << ", reason: " << cr.getText() << ")";

  NLSR_LOG_DEBUG(os.str());
}

void
Nlsr::startEventLoop()
{
  m_nlsrFace.processEvents();
}

} // namespace nlsr
