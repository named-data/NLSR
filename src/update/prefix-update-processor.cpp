/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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

#include "prefix-update-processor.hpp"

#include "lsdb.hpp"
#include "nlsr.hpp"
#include "prefix-update-commands.hpp"
#include "communication/sync-logic-handler.hpp"

#include <ndn-cxx/management/nfd-control-response.hpp>

namespace nlsr {
namespace update {

INIT_LOGGER("PrefixUpdateProcessor");

const ndn::Name::Component PrefixUpdateProcessor::MODULE_COMPONENT = ndn::Name::Component("prefix-update");
const ndn::Name::Component PrefixUpdateProcessor::ADVERTISE_VERB = ndn::Name::Component("advertise");
const ndn::Name::Component PrefixUpdateProcessor::WITHDRAW_VERB  = ndn::Name::Component("withdraw");

PrefixUpdateProcessor::PrefixUpdateProcessor(ndn::Face& face,
                                             NamePrefixList& namePrefixList,
                                             Lsdb& lsdb,
                                             SyncLogicHandler& sync,
                                             const ndn::Name broadcastPrefix,
                                             ndn::KeyChain& keyChain,
                                             ndn::shared_ptr<ndn::CertificateCacheTtl> certificateCache,
                                             security::CertificateStore& certStore)
  : m_face(face)
  , m_namePrefixList(namePrefixList)
  , m_lsdb(lsdb)
  , m_sync(sync)
  , m_keyChain(keyChain)
  , m_validator(m_face, broadcastPrefix, certificateCache, certStore)
  , m_isEnabled(false)
  , COMMAND_PREFIX(ndn::Name(Nlsr::LOCALHOST_PREFIX).append(MODULE_COMPONENT))
{
}

void
PrefixUpdateProcessor::startListening()
{
  _LOG_DEBUG("Setting Interest filter for: " << COMMAND_PREFIX);

  m_face.setInterestFilter(COMMAND_PREFIX, bind(&PrefixUpdateProcessor::onInterest, this, _2));
}

void
PrefixUpdateProcessor::onInterest(const ndn::Interest& request)
{
  _LOG_TRACE("Received Interest: " << request);

  if (!m_isEnabled) {
    sendNack(request);
    return;
  }

  m_validator.validate(request,
                       bind(&PrefixUpdateProcessor::onCommandValidated, this, _1),
                       bind(&PrefixUpdateProcessor::onCommandValidationFailed, this, _1, _2));
}

void
PrefixUpdateProcessor::loadValidator(boost::property_tree::ptree section,
                                     const std::string& filename)
{
  m_validator.load(section, filename);
}

void
PrefixUpdateProcessor::onCommandValidated(const std::shared_ptr<const ndn::Interest>& request)
{
  const ndn::Name& command = request->getName();
  const ndn::Name::Component& verb = command[COMMAND_PREFIX.size()];
  const ndn::Name::Component& parameterComponent = command[COMMAND_PREFIX.size() + 1];

  if (verb == ADVERTISE_VERB || verb == WITHDRAW_VERB) {
    ndn::nfd::ControlParameters parameters;

    if (!extractParameters(parameterComponent, parameters)) {
      sendResponse(request, 400, "Malformed command");
      return;
    }

    if (verb == ADVERTISE_VERB) {
      advertise(request, parameters);
    }
    else if (verb == WITHDRAW_VERB) {
      withdraw(request, parameters);
    }

    sendResponse(request, 200, "Success");
  }
  else {
    sendResponse(request, 501, "Unsupported command");
  }
}

void
PrefixUpdateProcessor::onCommandValidationFailed(const std::shared_ptr<const ndn::Interest>& request,
                                                 const std::string& failureInfo)
{
  sendResponse(request, 403, failureInfo);
}

bool
PrefixUpdateProcessor::extractParameters(const ndn::Name::Component& parameterComponent,
                                         ndn::nfd::ControlParameters& extractedParameters)
{
  try {
    ndn::Block rawParameters = parameterComponent.blockFromValue();
    extractedParameters.wireDecode(rawParameters);
  }
  catch (const ndn::tlv::Error&) {
    return false;
  }

  return true;
}

void
PrefixUpdateProcessor::advertise(const std::shared_ptr<const ndn::Interest>& request,
                                 const ndn::nfd::ControlParameters& parameters)
{
  AdvertisePrefixCommand command;

  if (!validateParameters(command, parameters)) {
    sendResponse(request, 400, "Malformed command");
    return;
  }

  _LOG_INFO("Advertising name: " << parameters.getName());

  if (m_namePrefixList.insert(parameters.getName())) {
    // Only build a Name LSA if the added name is new
    m_lsdb.buildAndInstallOwnNameLsa();
    m_sync.publishRoutingUpdate();
  }
}

void
PrefixUpdateProcessor::withdraw(const std::shared_ptr<const ndn::Interest>& request,
                                const ndn::nfd::ControlParameters& parameters)
{
  WithdrawPrefixCommand command;

  if (!validateParameters(command, parameters)) {
    sendResponse(request, 400, "Malformed command");
    return;
  }

  _LOG_INFO("Withdrawing name: " << parameters.getName());

  if (m_namePrefixList.remove(parameters.getName())) {
    // Only build a Name LSA if a name was actually removed
    m_lsdb.buildAndInstallOwnNameLsa();
    m_sync.publishRoutingUpdate();
  }
}

bool
PrefixUpdateProcessor::validateParameters(const ndn::nfd::ControlCommand& command,
                                          const ndn::nfd::ControlParameters& parameters)
{
  try {
    command.validateRequest(parameters);
  }
  catch (const ndn::nfd::ControlCommand::ArgumentError&) {
    return false;
  }

  return true;
}

void
PrefixUpdateProcessor::sendNack(const ndn::Interest& request)
{
  ndn::MetaInfo metaInfo;
  metaInfo.setType(ndn::tlv::ContentType_Nack);

  shared_ptr<ndn::Data> responseData = std::make_shared<ndn::Data>(request.getName());
  responseData->setMetaInfo(metaInfo);

  m_keyChain.sign(*responseData);
  m_face.put(*responseData);
}

void
PrefixUpdateProcessor::sendResponse(const std::shared_ptr<const ndn::Interest>& request,
                                    uint32_t code,
                                    const std::string& text)
{
  if (request == nullptr) {
    return;
  }

  ndn::nfd::ControlResponse response(code, text);
  const ndn::Block& encodedControl = response.wireEncode();

  std::shared_ptr<ndn::Data> responseData = std::make_shared<ndn::Data>(request->getName());
  responseData->setContent(encodedControl);

  m_keyChain.sign(*responseData);
  m_face.put(*responseData);
}

} // namespace update
} // namespace nlsr
