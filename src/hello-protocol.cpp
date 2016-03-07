/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
 *                           Regents of the University of California
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
 **/

#include "hello-protocol.hpp"
#include "nlsr.hpp"
#include "lsdb.hpp"
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
                                       std::bind(&HelloProtocol::onContent,
                                                 this,
                                                 _1, _2),
                                       std::bind(&HelloProtocol::processInterestTimedOut, // Nack
                                                 this, _1),
                                       std::bind(&HelloProtocol::processInterestTimedOut,
                                                 this, _1));

  // increment SENT_HELLO_INTEREST
  hpIncrementSignal(Statistics::PacketType::SENT_HELLO_INTEREST);
}

void
HelloProtocol::sendScheduledInterest(uint32_t seconds)
{
  std::list<Adjacent> adjList = m_nlsr.getAdjacencyList().getAdjList();
  for (std::list<Adjacent>::iterator it = adjList.begin(); it != adjList.end();
       ++it) {
    // If this adjacency has a Face, just proceed as usual.
    if((*it).getFaceId() != 0) {
      // interest name: /<neighbor>/NLSR/INFO/<router>
      ndn::Name interestName = (*it).getName() ;
      interestName.append(NLSR_COMPONENT);
      interestName.append(INFO_COMPONENT);
      interestName.append(m_nlsr.getConfParameter().getRouterPrefix().wireEncode());
      expressInterest(interestName,
                      m_nlsr.getConfParameter().getInterestResendTime());
      _LOG_DEBUG("Sending scheduled interest: " << interestName);
    }
  }
  scheduleInterest(m_nlsr.getConfParameter().getInfoInterestInterval());
}

void
HelloProtocol::scheduleInterest(uint32_t seconds)
{
  _LOG_DEBUG("Scheduling HELLO Interests in " << ndn::time::seconds(seconds));

  m_scheduler.scheduleEvent(ndn::time::seconds(seconds),
                            std::bind(&HelloProtocol::sendScheduledInterest, this, seconds));
}

void
HelloProtocol::processInterest(const ndn::Name& name,
                               const ndn::Interest& interest)
{
  // interest name: /<neighbor>/NLSR/INFO/<router>
  const ndn::Name interestName = interest.getName();

  // increment RCV_HELLO_INTEREST
  hpIncrementSignal(Statistics::PacketType::RCV_HELLO_INTEREST);

  _LOG_DEBUG("Interest Received for Name: " << interestName);
  if (interestName.get(-2).toUri() != INFO_COMPONENT) {
    _LOG_DEBUG("INFO_COMPONENT not found or interestName: " << interestName
               << " does not match expression");
    return;
  }

  ndn::Name neighbor;
  neighbor.wireDecode(interestName.get(-1).blockFromValue());
  _LOG_DEBUG("Neighbor: " << neighbor);
  if (m_nlsr.getAdjacencyList().isNeighbor(neighbor)) {
    std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>();
    data->setName(ndn::Name(interest.getName()).appendVersion());
    data->setFreshnessPeriod(ndn::time::seconds(10)); // 10 sec
    data->setContent(reinterpret_cast<const uint8_t*>(INFO_COMPONENT.c_str()),
                    INFO_COMPONENT.size());
    m_nlsr.getKeyChain().sign(*data, m_nlsr.getDefaultCertName());
    _LOG_DEBUG("Sending out data for name: " << interest.getName());
    m_nlsr.getNlsrFace().put(*data);
    // increment SENT_HELLO_DATA
    hpIncrementSignal(Statistics::PacketType::SENT_HELLO_DATA);

    auto adjacent = m_nlsr.getAdjacencyList().findAdjacent(neighbor);
    // If this neighbor was previously inactive, send our own hello interest, too
    if (adjacent->getStatus() == Adjacent::STATUS_INACTIVE) {
      // We can only do that if the neighbor currently has a face.
      if(adjacent->getFaceId() != 0){
        // interest name: /<neighbor>/NLSR/INFO/<router>
        ndn::Name interestName(neighbor);
        interestName.append(NLSR_COMPONENT);
        interestName.append(INFO_COMPONENT);
        interestName.append(m_nlsr.getConfParameter().getRouterPrefix().wireEncode());
        expressInterest(interestName,
                        m_nlsr.getConfParameter().getInterestResendTime());
      }
    }
  }
}

void
HelloProtocol::processInterestTimedOut(const ndn::Interest& interest)
{
  // interest name: /<neighbor>/NLSR/INFO/<router>
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
    // interest name: /<neighbor>/NLSR/INFO/<router>
    ndn::Name interestName(neighbor);
    interestName.append(NLSR_COMPONENT);
    interestName.append(INFO_COMPONENT);
    interestName.append(m_nlsr.getConfParameter().getRouterPrefix().wireEncode());
    _LOG_DEBUG("Resending interest: " << interestName);
    expressInterest(interestName,
                    m_nlsr.getConfParameter().getInterestResendTime());
  }
  else if ((status == Adjacent::STATUS_ACTIVE) &&
           (infoIntTimedOutCount == m_nlsr.getConfParameter().getInterestRetryNumber())) {
    m_nlsr.getAdjacencyList().setStatusOfNeighbor(neighbor, Adjacent::STATUS_INACTIVE);

    _LOG_DEBUG("Neighbor: " << neighbor << " status changed to INACTIVE");

    m_nlsr.getLsdb().scheduleAdjLsaBuild();
  }
}

  // This is the first function that incoming Hello data will
  // see. This checks if the data appears to be signed, and passes it
  // on to validate the content of the data.
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
                                 std::bind(&HelloProtocol::onContentValidated, this, _1),
                                 std::bind(&HelloProtocol::onContentValidationFailed,
                                           this, _1, _2));
}

  // A validator is called on the incoming data, and if the data
  // passes the validator's description/definitions, this function is
  // called. Set the neighbor's status to active and refresh its
  // LSA. If there was a change in status, we schedule an adjacency
  // LSA build.
void
HelloProtocol::onContentValidated(const std::shared_ptr<const ndn::Data>& data)
{
  // data name: /<neighbor>/NLSR/INFO/<router>/<version>
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
      if (m_nlsr.getConfParameter().getHyperbolicState() == HYPERBOLIC_STATE_ON) {
        m_nlsr.getRoutingTable().scheduleRoutingTableCalculation(m_nlsr);
      }
      else {
        m_nlsr.getLsdb().scheduleAdjLsaBuild();
      }
    }
  }
  // increment RCV_HELLO_DATA
  hpIncrementSignal(Statistics::PacketType::RCV_HELLO_DATA);
}

  // Simply logs a debug message that the content could not be
  // validated (and is implicitly being discarded as a result).
void
HelloProtocol::onContentValidationFailed(const std::shared_ptr<const ndn::Data>& data,
                                         const std::string& msg)
{
  _LOG_DEBUG("Validation Error: " << msg);
}

} // namespace nlsr
