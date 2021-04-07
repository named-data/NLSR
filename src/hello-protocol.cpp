/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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
 */

#include "hello-protocol.hpp"
#include "nlsr.hpp"
#include "lsdb.hpp"
#include "utility/name-helper.hpp"
#include "logger.hpp"

#include <ndn-cxx/encoding/nfd-constants.hpp>

namespace nlsr {

INIT_LOGGER(HelloProtocol);

const std::string HelloProtocol::INFO_COMPONENT = "INFO";
const std::string HelloProtocol::NLSR_COMPONENT = "nlsr";

HelloProtocol::HelloProtocol(ndn::Face& face, ndn::KeyChain& keyChain,
                             ConfParameter& confParam, RoutingTable& routingTable,
                             Lsdb& lsdb)
  : m_face(face)
  , m_scheduler(m_face.getIoService())
  , m_keyChain(keyChain)
  , m_signingInfo(confParam.getSigningInfo())
  , m_confParam(confParam)
  , m_routingTable(routingTable)
  , m_lsdb(lsdb)
  , m_adjacencyList(m_confParam.getAdjacencyList())
{
  ndn::Name name(m_confParam.getRouterPrefix());
  name.append(NLSR_COMPONENT);
  name.append(INFO_COMPONENT);

  NLSR_LOG_DEBUG("Setting interest filter for Hello interest: " << name);

  m_face.setInterestFilter(ndn::InterestFilter(name).allowLoopback(false),
    [this] (const auto& name, const auto& interest) {
      processInterest(name, interest);
    },
    [] (const auto& name) {
      NLSR_LOG_DEBUG("Successfully registered prefix: " << name);
    },
    [] (const auto& name, const auto& resp) {
      NLSR_LOG_ERROR("Failed to register prefix " << name);
      NDN_THROW(std::runtime_error("Failed to register hello prefix: " + resp));
    },
    m_signingInfo, ndn::nfd::ROUTE_FLAG_CAPTURE);
}

void
HelloProtocol::expressInterest(const ndn::Name& interestName, uint32_t seconds)
{
  NLSR_LOG_DEBUG("Expressing Interest: " << interestName);
  ndn::Interest interest(interestName);
  interest.setInterestLifetime(ndn::time::seconds(seconds));
  interest.setMustBeFresh(true);
  interest.setCanBePrefix(true);
  m_face.expressInterest(interest,
    std::bind(&HelloProtocol::onContent, this, _1, _2),
    [this, seconds] (const ndn::Interest& interest, const ndn::lp::Nack& nack)
    {
      NDN_LOG_TRACE("Received Nack with reason: " << nack.getReason());
      NDN_LOG_TRACE("Will treat as timeout in " << 2 * seconds << " seconds");
      m_scheduler.schedule(ndn::time::seconds(2 * seconds),
        [this, interest] { processInterestTimedOut(interest); });
    },
    std::bind(&HelloProtocol::processInterestTimedOut, this, _1));

  // increment SENT_HELLO_INTEREST
  hpIncrementSignal(Statistics::PacketType::SENT_HELLO_INTEREST);
}

void
HelloProtocol::sendHelloInterest(const ndn::Name& neighbor)
{
  auto adjacent = m_adjacencyList.findAdjacent(neighbor);

  if (adjacent == m_adjacencyList.end()) {
    return;
  }

  // If this adjacency has a Face, just proceed as usual.
  if(adjacent->getFaceId() != 0) {
    // interest name: /<neighbor>/NLSR/INFO/<router>
    ndn::Name interestName = adjacent->getName() ;
    interestName.append(NLSR_COMPONENT);
    interestName.append(INFO_COMPONENT);
    interestName.append(m_confParam.getRouterPrefix().wireEncode());
    expressInterest(interestName, m_confParam.getInterestResendTime());
    NLSR_LOG_DEBUG("Sending HELLO interest: " << interestName);
  }

  m_scheduler.schedule(ndn::time::seconds(m_confParam.getInfoInterestInterval()),
                       [this, neighbor] { sendHelloInterest(neighbor); });
}

void
HelloProtocol::processInterest(const ndn::Name& name,
                               const ndn::Interest& interest)
{
  // interest name: /<neighbor>/NLSR/INFO/<router>
  const ndn::Name interestName = interest.getName();

  // increment RCV_HELLO_INTEREST
  hpIncrementSignal(Statistics::PacketType::RCV_HELLO_INTEREST);

  NLSR_LOG_DEBUG("Interest Received for Name: " << interestName);
  if (interestName.get(-2).toUri() != INFO_COMPONENT) {
    NLSR_LOG_DEBUG("INFO_COMPONENT not found or interestName: " << interestName
               << " does not match expression");
    return;
  }

  ndn::Name neighbor;
  neighbor.wireDecode(interestName.get(-1).blockFromValue());
  NLSR_LOG_DEBUG("Neighbor: " << neighbor);
  if (m_adjacencyList.isNeighbor(neighbor)) {
    std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>();
    data->setName(ndn::Name(interest.getName()).appendVersion());
    data->setFreshnessPeriod(ndn::time::seconds(10)); // 10 sec
    data->setContent(reinterpret_cast<const uint8_t*>(INFO_COMPONENT.c_str()),
                                                      INFO_COMPONENT.size());

    m_keyChain.sign(*data, m_signingInfo);

    NLSR_LOG_DEBUG("Sending out data for name: " << interest.getName());

    m_face.put(*data);
    // increment SENT_HELLO_DATA
    hpIncrementSignal(Statistics::PacketType::SENT_HELLO_DATA);

    auto adjacent = m_adjacencyList.findAdjacent(neighbor);
    // If this neighbor was previously inactive, send our own hello interest, too
    if (adjacent->getStatus() == Adjacent::STATUS_INACTIVE) {
      // We can only do that if the neighbor currently has a face.
      if(adjacent->getFaceId() != 0){
        // interest name: /<neighbor>/NLSR/INFO/<router>
        ndn::Name interestName(neighbor);
        interestName.append(NLSR_COMPONENT);
        interestName.append(INFO_COMPONENT);
        interestName.append(m_confParam.getRouterPrefix().wireEncode());
        expressInterest(interestName, m_confParam.getInterestResendTime());
      }
    }
  }
}

void
HelloProtocol::processInterestTimedOut(const ndn::Interest& interest)
{
  // interest name: /<neighbor>/NLSR/INFO/<router>
  const ndn::Name interestName(interest.getName());
  NLSR_LOG_DEBUG("Interest timed out for Name: " << interestName);
  if (interestName.get(-2).toUri() != INFO_COMPONENT) {
    return;
  }
  ndn::Name neighbor = interestName.getPrefix(-3);
  NLSR_LOG_DEBUG("Neighbor: " << neighbor);
  m_adjacencyList.incrementTimedOutInterestCount(neighbor);

  Adjacent::Status status = m_adjacencyList.getStatusOfNeighbor(neighbor);

  uint32_t infoIntTimedOutCount = m_adjacencyList.getTimedOutInterestCount(neighbor);
  NLSR_LOG_DEBUG("Status: " << status);
  NLSR_LOG_DEBUG("Info Interest Timed out: " << infoIntTimedOutCount);
  if (infoIntTimedOutCount < m_confParam.getInterestRetryNumber()) {
    // interest name: /<neighbor>/NLSR/INFO/<router>
    ndn::Name interestName(neighbor);
    interestName.append(NLSR_COMPONENT);
    interestName.append(INFO_COMPONENT);
    interestName.append(m_confParam.getRouterPrefix().wireEncode());
    NLSR_LOG_DEBUG("Resending interest: " << interestName);
    expressInterest(interestName, m_confParam.getInterestResendTime());
  }
  else if (status == Adjacent::STATUS_ACTIVE) {
    m_adjacencyList.setStatusOfNeighbor(neighbor, Adjacent::STATUS_INACTIVE);

    NLSR_LOG_DEBUG("Neighbor: " << neighbor << " status changed to INACTIVE");

    if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
      m_routingTable.scheduleRoutingTableCalculation();
    }
    else {
      m_lsdb.scheduleAdjLsaBuild();
    }
  }
}

  // This is the first function that incoming Hello data will
  // see. This checks if the data appears to be signed, and passes it
  // on to validate the content of the data.
void
HelloProtocol::onContent(const ndn::Interest& interest, const ndn::Data& data)
{
  NLSR_LOG_DEBUG("Received data for INFO(name): " << data.getName());
  auto kl = data.getKeyLocator();
  if (kl && kl->getType() == ndn::tlv::Name) {
    NLSR_LOG_DEBUG("Data signed with: " << kl->getName());
  }
  m_confParam.getValidator().validate(data,
                                      std::bind(&HelloProtocol::onContentValidated, this, _1),
                                      std::bind(&HelloProtocol::onContentValidationFailed,
                                                this, _1, _2));
}

void
HelloProtocol::onContentValidated(const ndn::Data& data)
{
  // data name: /<neighbor>/NLSR/INFO/<router>/<version>
  ndn::Name dataName = data.getName();
  NLSR_LOG_DEBUG("Data validation successful for INFO(name): " << dataName);

  if (dataName.get(-3).toUri() == INFO_COMPONENT) {
    ndn::Name neighbor = dataName.getPrefix(-4);

    Adjacent::Status oldStatus = m_adjacencyList.getStatusOfNeighbor(neighbor);
    m_adjacencyList.setStatusOfNeighbor(neighbor, Adjacent::STATUS_ACTIVE);
    m_adjacencyList.setTimedOutInterestCount(neighbor, 0);
    Adjacent::Status newStatus = m_adjacencyList.getStatusOfNeighbor(neighbor);

    NLSR_LOG_DEBUG("Neighbor : " << neighbor);
    NLSR_LOG_DEBUG("Old Status: " << oldStatus << " New Status: " << newStatus);
    // change in Adjacency list
    if ((oldStatus - newStatus) != 0) {
      if (m_confParam.getHyperbolicState() == HYPERBOLIC_STATE_ON) {
        m_routingTable.scheduleRoutingTableCalculation();
      }
      else {
        m_lsdb.scheduleAdjLsaBuild();
      }
      onInitialHelloDataValidated(neighbor);
    }
  }
  // increment RCV_HELLO_DATA
  hpIncrementSignal(Statistics::PacketType::RCV_HELLO_DATA);
}

void
HelloProtocol::onContentValidationFailed(const ndn::Data& data,
                                         const ndn::security::ValidationError& ve)
{
  NLSR_LOG_DEBUG("Validation Error: " << ve);
}

} // namespace nlsr
