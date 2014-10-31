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

using namespace ndn;
using namespace std;

void
Nlsr::registrationFailed(const ndn::Name& name)
{
  std::cerr << "ERROR: Failed to register prefix in local hub's daemon" << endl;
  throw Error("Error: Prefix registration failed");
}

void
Nlsr::onRegistrationSuccess(const ndn::Name& name)
{
}

void
Nlsr::setInfoInterestFilter()
{
  ndn::Name name(m_confParam.getRouterPrefix());
  _LOG_DEBUG("Setting interest filter for name: " << name);
  getNlsrFace().setInterestFilter(name,
                                  ndn::bind(&HelloProtocol::processInterest,
                                            &m_helloProtocol, _1, _2),
                                  ndn::bind(&Nlsr::onRegistrationSuccess, this, _1),
                                  ndn::bind(&Nlsr::registrationFailed, this, _1),
                                  m_defaultIdentity,
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
                                  ndn::bind(&Lsdb::processInterest,
                                            &m_nlsrLsdb, _1, _2),
                                  ndn::bind(&Nlsr::onRegistrationSuccess, this, _1),
                                  ndn::bind(&Nlsr::registrationFailed, this, _1),
                                  m_defaultIdentity,
                                  ndn::nfd::ROUTE_FLAG_CAPTURE);
}

void
Nlsr::setStrategies()
{
  std::string strategy("ndn:/localhost/nfd/strategy/broadcast");
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
  m_syncLogicHandler.setSyncPrefix(m_confParam.getChronosyncPrefix().toUri());
  /* Logging start */
  m_confParam.writeLog();
  m_adjacencyList.writeLog();
  m_namePrefixList.writeLog();
  /* Logging end */
  initializeKey();
  setStrategies();
  _LOG_DEBUG("Default NLSR identity: " << m_defaultIdentity);
  setInfoInterestFilter();
  setLsaInterestFilter();
  m_nlsrLsdb.buildAndInstallOwnNameLsa();
  m_nlsrLsdb.buildAndInstallOwnCoordinateLsa();

  registerKeyPrefix();

  // Set event intervals
  setFirstHelloInterval(m_confParam.getFirstHelloInterval());
  m_helloProtocol.setAdjLsaBuildInterval(m_confParam.getAdjLsaBuildInterval());
  m_routingTable.setRoutingCalcInterval(m_confParam.getRoutingCalcInterval());

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
  m_defaultIdentity = m_confParam.getRouterPrefix();
  m_defaultIdentity.append("NLSR");

  try
  {
    m_keyChain.deleteIdentity(m_defaultIdentity);
  }
  catch (std::exception& e)
  {
  }

  ndn::Name keyName = m_keyChain.generateRsaKeyPairAsDefault(m_defaultIdentity, true);

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
                               ndn::bind(&Nlsr::onKeyInterest,
                                         this, _1, _2),
                               ndn::bind(&Nlsr::onKeyPrefixRegSuccess, this, _1),
                               ndn::bind(&Nlsr::registrationFailed, this, _1),
                               m_defaultIdentity,
                               ndn::nfd::ROUTE_FLAG_CAPTURE);

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
                      ndn::bind(&Nlsr::onDestroyFaceSuccess, this, _1),
                      ndn::bind(&Nlsr::onDestroyFaceFailure, this, _1, _2));
  }
}




void
Nlsr::onFaceEventNotification(const ndn::nfd::FaceEventNotification& faceEventNotification)
{
  ndn::nfd::FaceEventKind kind = faceEventNotification.getKind();
  _LOG_DEBUG("Nlsr::onFaceEventNotification called");
  if (kind == ndn::nfd::FACE_EVENT_DESTROYED) {
    uint64_t faceId = faceEventNotification.getFaceId();
    Adjacent *adjacent = m_adjacencyList.findAdjacent(faceId);
    if (adjacent != 0) {
      _LOG_DEBUG("Face to " << adjacent->getName() << " with face id: "
                 << faceId << " deleted");
      adjacent->setFaceId(0);
    }
  }
}


void
Nlsr::startEventLoop()
{
  m_nlsrFace.processEvents();
}

void
Nlsr::usage(const string& progname)
{
  std::cout << "Usage: " << progname << " [OPTIONS...]" << std::endl;
  std::cout << "   NDN routing...." << std::endl;
  std::cout << "       -d          Run in daemon mode" << std::endl;
  std::cout << "       -f <FILE>   Specify configuration file name" << std::endl;
  std::cout << "       -V          Display version information" << std::endl;
  std::cout << "       -h          Display this help message" << std::endl;
}


} // namespace nlsr
