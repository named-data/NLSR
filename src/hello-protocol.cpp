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
#include "nlsr.hpp"
#include "lsdb.hpp"
#include "hello-protocol.hpp"
#include "utility/name-helper.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("HelloProtocol");

const std::string HelloProtocol::INFO_COMPONENT = "INFO";
const std::string HelloProtocol::NLSR_COMPONENT = "NLSR";

void
HelloProtocol::expressInterest(const ndn::Name& interestName, uint32_t seconds)
{
  _LOG_DEBUG("Expressing Interest :" << interestName);
  ndn::Interest i(interestName);
  i.setInterestLifetime(ndn::time::seconds(seconds));
  i.setMustBeFresh(true);
  m_nlsr.getNlsrFace().expressInterest(i,
                                       ndn::bind(&HelloProtocol::onContent,
                                                 this,
                                                 _1, _2),
                                       ndn::bind(&HelloProtocol::processInterestTimedOut,
                                                 this, _1));
}

void
HelloProtocol::sendScheduledInterest(uint32_t seconds)
{
  std::list<Adjacent> adjList = m_nlsr.getAdjacencyList().getAdjList();
  for (std::list<Adjacent>::iterator it = adjList.begin(); it != adjList.end();
       ++it) {
    if((*it).getFaceId() != 0) {
      /* interest name: /<neighbor>/NLSR/INFO/<router> */
      ndn::Name interestName = (*it).getName() ;
      interestName.append(NLSR_COMPONENT);
      interestName.append(INFO_COMPONENT);
      interestName.append(m_nlsr.getConfParameter().getRouterPrefix().wireEncode());
      expressInterest(interestName,
                      m_nlsr.getConfParameter().getInterestResendTime());
    }
    else {
      registerPrefixes((*it).getName(), (*it).getConnectingFaceUri(),
                       (*it).getLinkCost(), ndn::time::milliseconds::max());
    }
  }
  scheduleInterest(m_nlsr.getConfParameter().getInfoInterestInterval());
}

void
HelloProtocol::scheduleInterest(uint32_t seconds)
{
  _LOG_DEBUG("Scheduling HELLO Interests in " << ndn::time::seconds(seconds));

  m_scheduler.scheduleEvent(ndn::time::seconds(seconds),
                            ndn::bind(&HelloProtocol::sendScheduledInterest, this, seconds));
}

void
HelloProtocol::processInterest(const ndn::Name& name,
                               const ndn::Interest& interest)
{
  /* interest name: /<neighbor>/NLSR/INFO/<router> */
  const ndn::Name interestName = interest.getName();
  _LOG_DEBUG("Interest Received for Name: " << interestName);
  if (interestName.get(-2).toUri() != INFO_COMPONENT) {
    return;
  }
  ndn::Name neighbor;
  neighbor.wireDecode(interestName.get(-1).blockFromValue());
  _LOG_DEBUG("Neighbor: " << neighbor);
  if (m_nlsr.getAdjacencyList().isNeighbor(neighbor)) {
    ndn::shared_ptr<ndn::Data> data = ndn::make_shared<ndn::Data>();
    data->setName(ndn::Name(interest.getName()).appendVersion());
    data->setFreshnessPeriod(ndn::time::seconds(10)); // 10 sec
    data->setContent(reinterpret_cast<const uint8_t*>(INFO_COMPONENT.c_str()),
                    INFO_COMPONENT.size());
    m_nlsr.getKeyChain().sign(*data, m_nlsr.getDefaultCertName());
    _LOG_DEBUG("Sending out data for name: " << interest.getName());
    m_nlsr.getNlsrFace().put(*data);
    Adjacent *adjacent = m_nlsr.getAdjacencyList().findAdjacent(neighbor);
    if (adjacent->getStatus() == Adjacent::STATUS_INACTIVE) {
      if(adjacent->getFaceId() != 0){
        /* interest name: /<neighbor>/NLSR/INFO/<router> */
        ndn::Name interestName(neighbor);
        interestName.append(NLSR_COMPONENT);
        interestName.append(INFO_COMPONENT);
        interestName.append(m_nlsr.getConfParameter().getRouterPrefix().wireEncode());
        expressInterest(interestName,
                        m_nlsr.getConfParameter().getInterestResendTime());
      }
      else {
        registerPrefixes(adjacent->getName(), adjacent->getConnectingFaceUri(),
                         adjacent->getLinkCost(), ndn::time::milliseconds::max());
      }
    }
  }
}

void
HelloProtocol::processInterestTimedOut(const ndn::Interest& interest)
{
  /* interest name: /<neighbor>/NLSR/INFO/<router> */
  const ndn::Name interestName(interest.getName());
  _LOG_DEBUG("Interest timed out for Name: " << interestName);
  if (interestName.get(-2).toUri() != INFO_COMPONENT) {
    return;
  }
  ndn::Name neighbor = interestName.getPrefix(-3);
  _LOG_DEBUG("Neighbor: " << neighbor);
  m_nlsr.getAdjacencyList().incrementTimedOutInterestCount(neighbor);

  Adjacent::Status status = m_nlsr.getAdjacencyList().getStatusOfNeighbor(neighbor);

  uint32_t infoIntTimedOutCount =
    m_nlsr.getAdjacencyList().getTimedOutInterestCount(neighbor);
  _LOG_DEBUG("Status: " << status);
  _LOG_DEBUG("Info Interest Timed out: " << infoIntTimedOutCount);
  if ((infoIntTimedOutCount < m_nlsr.getConfParameter().getInterestRetryNumber())) {
    /* interest name: /<neighbor>/NLSR/INFO/<router> */
    ndn::Name interestName(neighbor);
    interestName.append(NLSR_COMPONENT);
    interestName.append(INFO_COMPONENT);
    interestName.append(m_nlsr.getConfParameter().getRouterPrefix().wireEncode());
    expressInterest(interestName,
                    m_nlsr.getConfParameter().getInterestResendTime());
  }
  else if ((status == Adjacent::STATUS_ACTIVE) &&
           (infoIntTimedOutCount == m_nlsr.getConfParameter().getInterestRetryNumber())) {
    m_nlsr.getAdjacencyList().setStatusOfNeighbor(neighbor, Adjacent::STATUS_INACTIVE);

    m_nlsr.getLsdb().scheduleAdjLsaBuild();
  }
}

void
HelloProtocol::onContent(const ndn::Interest& interest, const ndn::Data& data)
{
  _LOG_DEBUG("Received data for INFO(name): " << data.getName());
  if (data.getSignature().hasKeyLocator()) {
    if (data.getSignature().getKeyLocator().getType() == ndn::KeyLocator::KeyLocator_Name) {
      _LOG_DEBUG("Data signed with: " << data.getSignature().getKeyLocator().getName());
    }
  }
  m_nlsr.getValidator().validate(data,
                                 ndn::bind(&HelloProtocol::onContentValidated, this, _1),
                                 ndn::bind(&HelloProtocol::onContentValidationFailed,
                                           this, _1, _2));
}

void
HelloProtocol::onContentValidated(const ndn::shared_ptr<const ndn::Data>& data)
{
  /* data name: /<neighbor>/NLSR/INFO/<router>/<version> */
  ndn::Name dataName = data->getName();
  _LOG_DEBUG("Data validation successful for INFO(name): " << dataName);
  if (dataName.get(-3).toUri() == INFO_COMPONENT) {
    ndn::Name neighbor = dataName.getPrefix(-4);

    Adjacent::Status oldStatus = m_nlsr.getAdjacencyList().getStatusOfNeighbor(neighbor);
    m_nlsr.getAdjacencyList().setStatusOfNeighbor(neighbor, Adjacent::STATUS_ACTIVE);
    m_nlsr.getAdjacencyList().setTimedOutInterestCount(neighbor, 0);
    Adjacent::Status newStatus = m_nlsr.getAdjacencyList().getStatusOfNeighbor(neighbor);

    _LOG_DEBUG("Neighbor : " << neighbor);
    _LOG_DEBUG("Old Status: " << oldStatus << " New Status: " << newStatus);
    // change in Adjacency list
    if ((oldStatus - newStatus) != 0) {
      m_nlsr.getLsdb().scheduleAdjLsaBuild();
    }
  }
}

void
HelloProtocol::onContentValidationFailed(const ndn::shared_ptr<const ndn::Data>& data,
                                         const std::string& msg)
{
  _LOG_DEBUG("Validation Error: " << msg);
}

void
HelloProtocol::registerPrefixes(const ndn::Name& adjName, const std::string& faceUri,
                               double linkCost, const ndn::time::milliseconds& timeout)
{
  m_nlsr.getFib().registerPrefix(adjName, faceUri, linkCost, timeout,
                                 ndn::nfd::ROUTE_FLAG_CAPTURE, 0,
                                 ndn::bind(&HelloProtocol::onRegistrationSuccess,
                                           this, _1, adjName,timeout),
                                 ndn::bind(&HelloProtocol::onRegistrationFailure,
                                           this, _1, _2, adjName));
}

void
HelloProtocol::onRegistrationSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                                     const ndn::Name& neighbor,const ndn::time::milliseconds& timeout)
{
  Adjacent *adjacent = m_nlsr.getAdjacencyList().findAdjacent(neighbor);
  if (adjacent != 0) {
    adjacent->setFaceId(commandSuccessResult.getFaceId());
    ndn::Name broadcastKeyPrefix = DEFAULT_BROADCAST_PREFIX;
    broadcastKeyPrefix.append("KEYS");
    std::string faceUri = adjacent->getConnectingFaceUri();
    double linkCost = adjacent->getLinkCost();
    m_nlsr.getFib().registerPrefix(m_nlsr.getConfParameter().getChronosyncPrefix(),
                                 faceUri, linkCost, timeout,
                                 ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
    m_nlsr.getFib().registerPrefix(m_nlsr.getConfParameter().getLsaPrefix(),
                                 faceUri, linkCost, timeout,
                                 ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
    m_nlsr.getFib().registerPrefix(broadcastKeyPrefix,
                                 faceUri, linkCost, timeout,
                                 ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
    m_nlsr.setStrategies();

    /* interest name: /<neighbor>/NLSR/INFO/<router> */
    ndn::Name interestName(neighbor);
    interestName.append(NLSR_COMPONENT);
    interestName.append(INFO_COMPONENT);
    interestName.append(m_nlsr.getConfParameter().getRouterPrefix().wireEncode());
    expressInterest(interestName,
                    m_nlsr.getConfParameter().getInterestResendTime());
  }
}

void
HelloProtocol::onRegistrationFailure(uint32_t code, const std::string& error,
                                     const ndn::Name& name)
{
  _LOG_DEBUG(error << " (code: " << code << ")");
  /*
  * If NLSR can not create face for given faceUri then it will treat this
  * failure as one INFO interest timed out. So that NLSR can move on with
  * building Adj Lsa and calculate routing table. NLSR does not build Adj
  * Lsa unless all the neighbors are ACTIVE or DEAD. For considering the
  * missconfigured(link) neighbour dead this is required.
  */
  Adjacent *adjacent = m_nlsr.getAdjacencyList().findAdjacent(name);
  if (adjacent != 0) {
    adjacent->setInterestTimedOutNo(adjacent->getInterestTimedOutNo() + 1);
    Adjacent::Status status = adjacent->getStatus();
    uint32_t infoIntTimedOutCount = adjacent->getInterestTimedOutNo();

    if (infoIntTimedOutCount == m_nlsr.getConfParameter().getInterestRetryNumber()) {
      if (status == Adjacent::STATUS_ACTIVE) {
        adjacent->setStatus(Adjacent::STATUS_INACTIVE);
      }

      m_nlsr.getLsdb().scheduleAdjLsaBuild();
    }
  }
}

} //namespace nlsr
