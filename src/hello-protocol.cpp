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

const std::string HelloProtocol::INFO_COMPONENT="info";

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
    ndn::Name interestName = (*it).getName() ;
    interestName.append(INFO_COMPONENT);
    interestName.append(m_nlsr.getConfParameter().getRouterPrefix().wireEncode());
    expressInterest(interestName,
                    m_nlsr.getConfParameter().getInterestResendTime());
  }
  scheduleInterest(m_nlsr.getConfParameter().getInfoInterestInterval());
}

void
HelloProtocol::scheduleInterest(uint32_t seconds)
{
  m_nlsr.getScheduler().scheduleEvent(ndn::time::seconds(seconds),
                                      ndn::bind(&HelloProtocol::sendScheduledInterest,
                                                this, seconds));
}

void
HelloProtocol::processInterest(const ndn::Name& name,
                               const ndn::Interest& interest)
{
  const ndn::Name interestName = interest.getName();
  _LOG_DEBUG("Interest Received for Name: " << interestName);
  if (interestName.get(-2).toUri() != INFO_COMPONENT) {
    return;
  }
  ndn::Name neighbor;
  neighbor.wireDecode(interestName.get(-1).blockFromValue());
  _LOG_DEBUG("Neighbor: " << neighbor);
  if (m_nlsr.getAdjacencyList().isNeighbor(neighbor)) {
    ndn::Data data(ndn::Name(interest.getName()).appendVersion());
    data.setFreshnessPeriod(ndn::time::seconds(10)); // 10 sec
    data.setContent(reinterpret_cast<const uint8_t*>(INFO_COMPONENT.c_str()),
                    INFO_COMPONENT.size());
    m_nlsr.getKeyChain().sign(data, m_nlsr.getDefaultCertName());
    _LOG_DEBUG("Sending out data for name: " << data.getName());
    m_nlsr.getNlsrFace().put(data);
    int status = m_nlsr.getAdjacencyList().getStatusOfNeighbor(neighbor);
    if (status == 0) {
      ndn::Name interestName(neighbor);
      interestName.append(INFO_COMPONENT);
      interestName.append(m_nlsr.getConfParameter().getRouterPrefix());
      expressInterest(interestName,
                      m_nlsr.getConfParameter().getInterestResendTime());
    }
  }
}

void
HelloProtocol::processInterestTimedOut(const ndn::Interest& interest)
{
  const ndn::Name interestName(interest.getName());
  _LOG_DEBUG("Interest timed out for Name: " << interestName);
  if (interestName.get(-2).toUri() != INFO_COMPONENT) {
    return;
  }
  ndn::Name neighbor = interestName.getPrefix(-2);
  _LOG_DEBUG("Neighbor: " << neighbor);
  m_nlsr.getAdjacencyList().incrementTimedOutInterestCount(neighbor);
  int status = m_nlsr.getAdjacencyList().getStatusOfNeighbor(neighbor);
  uint32_t infoIntTimedOutCount =
    m_nlsr.getAdjacencyList().getTimedOutInterestCount(neighbor);
  _LOG_DEBUG("Status: " << status);
  _LOG_DEBUG("Info Interest Timed out: " << infoIntTimedOutCount);
  if ((infoIntTimedOutCount < m_nlsr.getConfParameter().getInterestRetryNumber())) {
    ndn::Name interestName(neighbor);
    interestName.append(INFO_COMPONENT);
    interestName.append(m_nlsr.getConfParameter().getRouterPrefix().wireEncode());
    expressInterest(interestName,
                    m_nlsr.getConfParameter().getInterestResendTime());
  }
  else if ((status == 1) &&
           (infoIntTimedOutCount == m_nlsr.getConfParameter().getInterestRetryNumber())) {
    m_nlsr.getAdjacencyList().setStatusOfNeighbor(neighbor, 0);
    m_nlsr.incrementAdjBuildCount();
    if (m_nlsr.getIsBuildAdjLsaSheduled() == false) {
      _LOG_DEBUG("Scheduling scheduledAdjLsaBuild");
      m_nlsr.setIsBuildAdjLsaSheduled(true);
      // event here
      m_nlsr.getScheduler().scheduleEvent(ndn::time::seconds(5),
                                          ndn::bind(&Lsdb::scheduledAdjLsaBuild,
                                                    &m_nlsr.getLsdb()));
    }
  }
}

void
HelloProtocol::onContent(const ndn::Interest& interest, const ndn::Data& data)
{
  m_nlsr.getValidator().validate(data,
                                 ndn::bind(&HelloProtocol::onContentValidated, this, _1),
                                 ndn::bind(&HelloProtocol::onContentValidationFailed,
                                           this, _1, _2));
}

void
HelloProtocol::onContentValidated(const ndn::shared_ptr<const ndn::Data>& data)
{
  ndn::Name dataName = data->getName();
  _LOG_DEBUG("Data received for name: " << dataName);
  if (dataName.get(-3).toUri() == INFO_COMPONENT) {
    ndn::Name neighbor = dataName.getPrefix(-3);
    int oldStatus = m_nlsr.getAdjacencyList().getStatusOfNeighbor(neighbor);
    m_nlsr.getAdjacencyList().setStatusOfNeighbor(neighbor, 1);
    m_nlsr.getAdjacencyList().setTimedOutInterestCount(neighbor, 0);
    int newStatus = m_nlsr.getAdjacencyList().getStatusOfNeighbor(neighbor);
    _LOG_DEBUG("Neighbor : " << neighbor);
    _LOG_DEBUG("Old Status: " << oldStatus << " New Status: " << newStatus);
    // change in Adjacency list
    if ((oldStatus - newStatus) != 0) {
      m_nlsr.incrementAdjBuildCount();
      // Need to schedule event for Adjacency LSA building
      if (m_nlsr.getIsBuildAdjLsaSheduled() == false) {
        _LOG_DEBUG("Scheduling scheduledAdjLsaBuild");
        m_nlsr.setIsBuildAdjLsaSheduled(true);
        // event here
        m_nlsr.getScheduler().scheduleEvent(ndn::time::seconds(5),
                                            ndn::bind(&Lsdb::scheduledAdjLsaBuild,
                                                      ndn::ref(m_nlsr.getLsdb())));
      }
    }
  }
}

void
HelloProtocol::onContentValidationFailed(const ndn::shared_ptr<const ndn::Data>& data,
                                         const std::string& msg)
{
  _LOG_DEBUG("Validation Error: " << msg);
}

} //namespace nlsr
