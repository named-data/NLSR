/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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
 */

#include "nlsr.hpp"
#include "adjacent.hpp"
#include "logger.hpp"

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include <ndn-cxx/net/face-uri.hpp>

namespace nlsr {

INIT_LOGGER(Nlsr);

const ndn::Name Nlsr::LOCALHOST_PREFIX = ndn::Name("/localhost/nlsr");

Nlsr::Nlsr(ndn::Face& face, ndn::KeyChain& keyChain, ConfParameter& confParam)
  : m_face(face)
  , m_scheduler(face.getIoService())
  , m_confParam(confParam)
  , m_adjacencyList(confParam.getAdjacencyList())
  , m_namePrefixList(confParam.getNamePrefixList())
  , m_fib(m_face, m_scheduler, m_adjacencyList, m_confParam, keyChain)
  , m_lsdb(m_face, keyChain, m_confParam)
  , m_routingTable(m_scheduler, m_lsdb, m_confParam)
  , m_namePrefixTable(confParam.getRouterPrefix(), m_fib, m_routingTable,
                      m_routingTable.afterRoutingChange, m_lsdb.onLsdbModified)
  , m_helloProtocol(m_face, keyChain, confParam, m_routingTable, m_lsdb)
  , m_onNewLsaConnection(m_lsdb.getSync().onNewLsa->connect(
      [this] (const ndn::Name& updateName, uint64_t sequenceNumber,
              const ndn::Name& originRouter) {
        registerStrategyForCerts(originRouter);
      }))
  , m_onPrefixRegistrationSuccess(m_fib.onPrefixRegistrationSuccess.connect(
      [this] (const ndn::Name& name) {
        m_helloProtocol.sendHelloInterest(name);
      }))
  , m_onInitialHelloDataValidated(m_helloProtocol.onInitialHelloDataValidated.connect(
      [this] (const ndn::Name& neighbor) {
        auto it = m_adjacencyList.findAdjacent(neighbor);
        if (it != m_adjacencyList.end()) {
          m_fib.registerPrefix(m_confParam.getSyncPrefix(), it->getFaceUri(), it->getLinkCost(),
                               ndn::time::milliseconds::max(), ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
        }
      }))
  , m_dispatcher(m_face, keyChain)
  , m_datasetHandler(m_dispatcher, m_lsdb, m_routingTable)
  , m_controller(m_face, keyChain)
  , m_faceDatasetController(m_face, keyChain)
  , m_prefixUpdateProcessor(m_dispatcher,
      m_confParam.getPrefixUpdateValidator(),
      m_namePrefixList,
      m_lsdb,
      m_confParam.getConfFileNameDynamic())
  , m_nfdRibCommandProcessor(m_dispatcher,
      m_namePrefixList,
      m_lsdb)
  , m_statsCollector(m_lsdb, m_helloProtocol)
  , m_faceMonitor(m_face)
{
  NLSR_LOG_DEBUG("Initializing Nlsr");

  m_faceMonitor.onNotification.connect(std::bind(&Nlsr::onFaceEventNotification, this, _1));
  m_faceMonitor.start();

  m_fib.setStrategy(m_confParam.getLsaPrefix(), Fib::MULTICAST_STRATEGY, 0);
  m_fib.setStrategy(m_confParam.getSyncPrefix(), Fib::MULTICAST_STRATEGY, 0);

  NLSR_LOG_DEBUG("Default NLSR identity: " << m_confParam.getSigningInfo().getSignerName());

  // Add top-level prefixes: router and localhost prefix
  addDispatcherTopPrefix(ndn::Name(m_confParam.getRouterPrefix()).append("nlsr"));
  addDispatcherTopPrefix(LOCALHOST_PREFIX);

  enableIncomingFaceIdIndication();

  initializeFaces(std::bind(&Nlsr::processFaceDataset, this, _1),
                  std::bind(&Nlsr::onFaceDatasetFetchTimeout, this, _1, _2, 0));

  m_adjacencyList.writeLog();
  NLSR_LOG_DEBUG(m_namePrefixList);

  // Need to set direct neighbors' costs to 0 for hyperbolic routing
  if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
    for (auto&& neighbor : m_adjacencyList.getAdjList()) {
      neighbor.setLinkCost(0);
    }
  }
}

void
Nlsr::registerStrategyForCerts(const ndn::Name& originRouter)
{
  for (const ndn::Name& router : m_strategySetOnRouters) {
    if (router == originRouter) {
      // Have already set strategy for this router's certs once
      return;
    }
  }

  m_strategySetOnRouters.push_back(originRouter);

  ndn::Name routerKey(originRouter);
  routerKey.append(ndn::security::Certificate::KEY_COMPONENT);
  ndn::Name instanceKey(originRouter);
  instanceKey.append("nlsr").append(ndn::security::Certificate::KEY_COMPONENT);

  m_fib.setStrategy(routerKey, Fib::BEST_ROUTE_V2_STRATEGY, 0);
  m_fib.setStrategy(instanceKey, Fib::BEST_ROUTE_V2_STRATEGY, 0);

  ndn::Name siteKey;
  for (size_t i = 0; i < originRouter.size(); ++i) {
    if (originRouter[i].toUri() == "%C1.Router") {
      break;
    }
    siteKey.append(originRouter[i]);
  }
  ndn::Name opPrefix(siteKey);
  siteKey.append(ndn::security::Certificate::KEY_COMPONENT);
  m_fib.setStrategy(siteKey, Fib::BEST_ROUTE_V2_STRATEGY, 0);

  opPrefix.append(std::string("%C1.Operator"));
  m_fib.setStrategy(opPrefix, Fib::BEST_ROUTE_V2_STRATEGY, 0);
}

void
Nlsr::addDispatcherTopPrefix(const ndn::Name& topPrefix)
{
  registerPrefix(topPrefix);
  try {
    // false since we want to have control over the registration process
    m_dispatcher.addTopPrefix(topPrefix, false, m_confParam.getSigningInfo());
  }
  catch (const std::exception& e) {
    NLSR_LOG_ERROR("Error setting top-level prefix in dispatcher: " << e.what());
  }
}

void
Nlsr::registerPrefix(const ndn::Name& prefix)
{
  m_face.registerPrefix(prefix,
    [] (const ndn::Name& name) {
      NLSR_LOG_DEBUG("Successfully registered prefix: " << name);
    },
    [] (const ndn::Name& name, const std::string& reason) {
      NLSR_LOG_ERROR("ERROR: Failed to register prefix " << name << " in local hub's daemon");
      NDN_THROW(Error("Error: Prefix registration failed: " + reason));
    });
}

void
Nlsr::onFaceEventNotification(const ndn::nfd::FaceEventNotification& faceEventNotification)
{
  NLSR_LOG_TRACE("onFaceEventNotification called");

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

          if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
            m_routingTable.scheduleRoutingTableCalculation();
          }
          else {
            // Will call scheduleRoutingTableCalculation internally
            // if needed in case of LS or DRY_RUN
            m_lsdb.scheduleAdjLsaBuild();
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
      uint64_t faceId = faceEventNotification.getFaceId();

      // If we have a neighbor by that FaceUri and it has no FaceId or
      // the FaceId is different from ours, we have a match.
      if (adjacent != m_adjacencyList.end() &&
          (adjacent->getFaceId() == 0 || adjacent->getFaceId() != faceId))
      {
        NLSR_LOG_DEBUG("Face creation event matches neighbor: " << adjacent->getName()
                        << ". New Face ID: " << faceId << ". Registering prefixes.");
        adjacent->setFaceId(faceId);

        registerAdjacencyPrefixes(*adjacent, ndn::time::milliseconds::max());

        // We should not do scheduleRoutingTableCalculation or scheduleAdjLsaBuild here
        // because once the prefixes are registered, we send a HelloInterest
        // to the prefix (see NLSR ctor). HelloProtocol will call these functions
        // once HelloData is received and validated.
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
  for (auto&& adjacent : m_adjacencyList.getAdjList()) {

    const std::string& faceUriString = adjacent.getFaceUri().toString();
    // Check the list of FaceStatus objects we got for a match
    for (const auto& faceStatus : faces) {
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
Nlsr::registerAdjacencyPrefixes(const Adjacent& adj, ndn::time::milliseconds timeout)
{
  ndn::FaceUri faceUri = adj.getFaceUri();
  double linkCost = adj.getLinkCost();
  const ndn::Name& adjName = adj.getName();

  m_fib.registerPrefix(adjName, faceUri, linkCost,
                       timeout, ndn::nfd::ROUTE_FLAG_CAPTURE, 0);

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

  m_scheduler.schedule(m_confParam.getFaceDatasetFetchInterval(),
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
    [] (const ndn::nfd::ControlParameters& cp) {
      NLSR_LOG_DEBUG("Successfully enabled incoming face id indication"
                     << "for face id " << cp.getFaceId());
    },
    [] (const ndn::nfd::ControlResponse& cr) {
      NLSR_LOG_WARN("Failed to enable incoming face id indication feature: " <<
                    "(code: " << cr.getCode() << ", reason: " << cr.getText() << ")");
    });
}

} // namespace nlsr
