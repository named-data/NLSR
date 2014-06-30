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
                                  ndn::bind(&Nlsr::registrationFailed, this, _1));
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
                                  ndn::bind(&Nlsr::registrationFailed, this, _1));
}

void
Nlsr::registerPrefixes()
{
  std::string strategy("ndn:/localhost/nfd/strategy/broadcast");
  ndn::Name broadcastKeyPrefix = DEFAULT_BROADCAST_PREFIX;
  broadcastKeyPrefix.append("KEYS");
  std::list<Adjacent>& adjacents = m_adjacencyList.getAdjList();
  for (std::list<Adjacent>::iterator it = adjacents.begin();
       it != adjacents.end(); it++) {
    m_fib.registerPrefix((*it).getName(), (*it).getConnectingFaceUri(),
                         (*it).getLinkCost(), 31536000); /* One Year in seconds */
    m_fib.registerPrefix(m_confParam.getChronosyncPrefix(),
                         (*it).getConnectingFaceUri(), (*it).getLinkCost(), 31536000);
    m_fib.registerPrefix(m_confParam.getLsaPrefix(),
                         (*it).getConnectingFaceUri(), (*it).getLinkCost(), 31536000);
    m_fib.registerPrefix(broadcastKeyPrefix,
                         (*it).getConnectingFaceUri(), (*it).getLinkCost(), 31536000);
    m_fib.setStrategy((*it).getName(), strategy);
  }

  m_fib.setStrategy(m_confParam.getChronosyncPrefix(), strategy);
  m_fib.setStrategy(m_confParam.getLsaPrefix(), strategy);
  m_fib.setStrategy(broadcastKeyPrefix, strategy);
}

void
Nlsr::initialize()
{
  _LOG_DEBUG("Initializing Nlsr");
  m_confParam.buildRouterPrefix();
  m_nlsrLsdb.setLsaRefreshTime(m_confParam.getLsaRefreshTime());
  m_nlsrLsdb.setThisRouterPrefix(m_confParam.getRouterPrefix().toUri());
  m_fib.setEntryRefreshTime(2 * m_confParam.getLsaRefreshTime());
  m_sequencingManager.setSeqFileName(m_confParam.getSeqFileDir());
  m_sequencingManager.initiateSeqNoFromFile();
  m_syncLogicHandler.setSyncPrefix(m_confParam.getChronosyncPrefix().toUri());
  intializeKey();
  /* Logging start */
  m_confParam.writeLog();
  m_adjacencyList.writeLog();
  m_namePrefixList.writeLog();
  /* Logging end */

  createFaces();
}

void
Nlsr::start()
{
  registerPrefixes();
  setInfoInterestFilter();
  setLsaInterestFilter();
  m_nlsrLsdb.buildAndInstallOwnNameLsa();
  m_nlsrLsdb.buildAndInstallOwnCoordinateLsa();
  m_syncLogicHandler.createSyncSocket(boost::ref(*this));
  registerKeyPrefix();
  m_helloProtocol.scheduleInterest(10);
}

void
Nlsr::intializeKey()
{
  m_defaultIdentity = m_confParam.getRouterPrefix();
  m_defaultIdentity.append("NLSR");

  ndn::Name keyName = m_keyChain.generateRsaKeyPairAsDefault(m_defaultIdentity, true);

  ndn::shared_ptr<ndn::IdentityCertificate> certificate = m_keyChain.selfSign(keyName);
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
                               ndn::bind(&Nlsr::registrationFailed, this, _1));

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

  Data data(interestName);
  data.setContent(cert->wireEncode());
  m_keyChain.signWithSha256(data);

  m_nlsrFace.put(data);
}

void
Nlsr::onKeyPrefixRegSuccess(const ndn::Name& name)
{
}

void
Nlsr::createFace(const std::string& faceUri,
                 const CommandSucceedCallback& onSuccess,
                 const CommandFailCallback& onFailure)
{
  ndn::nfd::ControlParameters faceParameters;
  faceParameters
    .setUri(faceUri);
  m_controller.start<ndn::nfd::FaceCreateCommand>(faceParameters,
                                                  onSuccess,
                                                  onFailure);
}

void
Nlsr::onCreateFaceSuccess(const ndn::nfd::ControlParameters& commandSuccessResult)
{
  m_nFacesCreated++;
  if (m_nFacesToCreate == m_nFacesCreated)
  {
    start();
  }
}

void
Nlsr::onCreateFaceFailure(int32_t code, const std::string& error)
{
  _LOG_DEBUG(error << " (code: " << code << ")");
  destroyFaces();
  throw Error("Error: Face creation failed");
}

void
Nlsr::createFaces()
{
  std::list<Adjacent>& adjacents = m_adjacencyList.getAdjList();
  for (std::list<Adjacent>::iterator it = adjacents.begin();
       it != adjacents.end(); it++) {
    createFace((*it).getConnectingFaceUri(),
               ndn::bind(&Nlsr::onCreateFaceSuccess, this, _1),
               ndn::bind(&Nlsr::onCreateFaceFailure, this, _1, _2));
    m_nFacesToCreate++;
  }
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
    destroyFace((*it).getConnectingFaceUri());
  }
}

void
Nlsr::destroyFace(const std::string& faceUri)
{
  ndn::nfd::ControlParameters faceParameters;
  faceParameters
    .setUri(faceUri);
  m_controller.start<ndn::nfd::FaceCreateCommand>(faceParameters,
                                                  ndn::bind(&Nlsr::destroyFaceInNfd,
                                                            this, _1),
                                                  ndn::bind(&Nlsr::onDestroyFaceFailure,
                                                            this, _1, _2));
}

void
Nlsr::destroyFaceInNfd(const ndn::nfd::ControlParameters& faceDestroyResult)
{
  ndn::nfd::ControlParameters faceParameters;
  faceParameters
    .setFaceId(faceDestroyResult.getFaceId());
  m_controller.start<ndn::nfd::FaceDestroyCommand>(faceParameters,
                                                   ndn::bind(&Nlsr::onDestroyFaceSuccess,
                                                             this, _1),
                                                   ndn::bind(&Nlsr::onDestroyFaceFailure,
                                                             this, _1, _2));
}


void
Nlsr::startEventLoop()
{
  m_nlsrFace.processEvents();
}

void
Nlsr::usage(const string& progname)
{
  cout << "Usage: " << progname << " [OPTIONS...]" << endl;
  cout << "   NDN routing...." << endl;
  cout << "       -d, --daemon        Run in daemon mode" << endl;
  cout << "       -f, --config_file   Specify configuration file name" << endl;
  cout << "       -p, --api_port      port where api client will connect" << endl;
  cout << "       -h, --help          Display this help message" << endl;
}


} // namespace nlsr
