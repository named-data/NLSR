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

#include "nlsrc.hpp"

#include "version.hpp"
#include "src/publisher/dataset-interest-handler.hpp"

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/security/command-interest-signer.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/util/segment-fetcher.hpp>

#include <iostream>

namespace nlsrc {

const ndn::Name Nlsrc::LOCALHOST_PREFIX = ndn::Name("/localhost/nlsr");
const ndn::Name Nlsrc::LSDB_PREFIX = ndn::Name(Nlsrc::LOCALHOST_PREFIX).append("lsdb");
const ndn::Name Nlsrc::NAME_UPDATE_PREFIX = ndn::Name(Nlsrc::LOCALHOST_PREFIX).append("prefix-update");

const ndn::Name Nlsrc::RT_PREFIX = ndn::Name(Nlsrc::LOCALHOST_PREFIX).append("routing-table");

const uint32_t Nlsrc::ERROR_CODE_TIMEOUT = 10060;
const uint32_t Nlsrc::RESPONSE_CODE_SUCCESS = 200;
const uint32_t Nlsrc::RESPONSE_CODE_SAVE_OR_DELETE = 205;

Nlsrc::Nlsrc(ndn::Face& face)
  : m_face(face)
{
}

void
Nlsrc::printUsage()
{
  std::cout << "Usage:\n" << programName  << " [-h] [-V] COMMAND [<Command Options>]\n"
    "       -h print usage and exit\n"
    "       -V print version and exit\n"
    "\n"
    "   COMMAND can be one of the following:\n"
    "       lsdb\n"
    "           display NLSR lsdb status\n"
    "       routing\n"
    "           display routing table status\n"
    "       status\n"
    "           display all NLSR status (lsdb & routingtable)\n"
    "       advertise name\n"
    "           advertise a name prefix through NLSR\n"
    "       advertise name save\n"
    "           advertise and save the name prefix to the conf file\n"
    "       withdraw name\n"
    "           remove a name prefix advertised through NLSR\n"
    "       withdraw name delete\n"
    "           withdraw and delete the name prefix from the conf file"
    << std::endl;
}

void
Nlsrc::getStatus(const std::string& command)
{
  if (command == "lsdb") {
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchAdjacencyLsas, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchCoordinateLsas, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchNameLsas, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::printLsdb, this));
  }
  else if (command == "routing") {
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchRtables, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::printRT, this));
  }
  else if(command == "status") {
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchAdjacencyLsas, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchCoordinateLsas, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchNameLsas, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchRtables, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::printAll, this));
  }
  runNextStep();
}

bool
Nlsrc::dispatch(const std::string& command)
{
  if (command == "advertise") {
    if (nOptions < 0) {
      return false;
    }
    else if (nOptions == 1) {
      std::string saveFlag = commandLineArguments[0];
      if (saveFlag != "save") {
        return false;
      }
    }

    advertiseName();
    return true;
  }
  else if (command == "withdraw") {
    if (nOptions < 0) {
      return false;
    }
    else if (nOptions == 1) {
      std::string saveFlag = commandLineArguments[0];
      if (saveFlag != "delete") {
        return false;
      }
    }

    withdrawName();
    return true;
  }
  else if ((command == "lsdb") || (command == "routing") || (command == "status")) {
    if (nOptions != -1) {
      return false;
    }
    commandString = command;

    getStatus(command);
    return true;
  }

  return false;
}

void
Nlsrc::runNextStep()
{
  if (m_fetchSteps.empty()) {
    return;
  }

  std::function<void()> nextStep = m_fetchSteps.front();
  m_fetchSteps.pop_front();

  nextStep();
}

void
Nlsrc::advertiseName()
{
  ndn::Name name = commandLineArguments[-1];

  bool saveFlag = false;
  std::string info = "(Advertise: " + name.toUri() + ")";
  if (commandLineArguments[0]) {
    saveFlag = true;
    info = "(Save: " + name.toUri() + ")";
  }
  ndn::Name::Component verb("advertise");
  sendNamePrefixUpdate(name, verb, info, saveFlag);
}

void
Nlsrc::withdrawName()
{
  ndn::Name name = commandLineArguments[-1];

  bool deleteFlag = false;
  std::string info = "(Withdraw: " + name.toUri() + ")";
  if (commandLineArguments[0]) {
    deleteFlag = true;
    info = "(Delete: " + name.toUri() + ")";
  }
  ndn::Name::Component verb("withdraw");
  sendNamePrefixUpdate(name, verb, info, deleteFlag);
}

void
Nlsrc::sendNamePrefixUpdate(const ndn::Name& name,
                            const ndn::Name::Component& verb,
                            const std::string& info,
                            bool flag)
{
  ndn::nfd::ControlParameters parameters;
  parameters.setName(name);
  if (flag) {
    parameters.setFlags(1);
  }

  ndn::Name commandName = NAME_UPDATE_PREFIX;
  commandName.append(verb);
  commandName.append(parameters.wireEncode());

  ndn::security::CommandInterestSigner cis(m_keyChain);

  ndn::Interest commandInterest =
    cis.makeCommandInterest(commandName,
                            ndn::security::signingByIdentity(m_keyChain.getPib().
                                                             getDefaultIdentity()));

  commandInterest.setMustBeFresh(true);

  m_face.expressInterest(commandInterest,
                         std::bind(&Nlsrc::onControlResponse, this, info, _2),
                         std::bind(&Nlsrc::onTimeout, this, ERROR_CODE_TIMEOUT, "Nack"),
                         std::bind(&Nlsrc::onTimeout, this, ERROR_CODE_TIMEOUT, "Timeout"));
}

void
Nlsrc::onControlResponse(const std::string& info, const ndn::Data& data)
{
  if (data.getMetaInfo().getType() == ndn::tlv::ContentType_Nack) {
    std::cerr << "ERROR: Run-time advertise/withdraw disabled" << std::endl;
    return;
  }

  ndn::nfd::ControlResponse response;

  try {
    response.wireDecode(data.getContent().blockFromValue());
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: Control response decoding error" << std::endl;
    return;
  }

  uint32_t code = response.getCode();

  if (code != RESPONSE_CODE_SUCCESS && code != RESPONSE_CODE_SAVE_OR_DELETE) {

    std::cerr << response.getText() << std::endl;
    std::cerr << "Name prefix update error (code: " << code << ")" << std::endl;
    return;
  }

  std::cout << "Applied Name prefix update successfully: " << info << std::endl;
}

void
Nlsrc::fetchAdjacencyLsas()
{
  fetchFromLsdb<nlsr::AdjLsa>(nlsr::dataset::ADJACENCY_COMPONENT,
                              std::bind(&Nlsrc::recordLsa, this, _1));
}

void
Nlsrc::fetchCoordinateLsas()
{
  fetchFromLsdb<nlsr::CoordinateLsa>(nlsr::dataset::COORDINATE_COMPONENT,
                                     std::bind(&Nlsrc::recordLsa, this, _1));
}

void
Nlsrc::fetchNameLsas()
{
  fetchFromLsdb<nlsr::NameLsa>(nlsr::dataset::NAME_COMPONENT,
                               std::bind(&Nlsrc::recordLsa, this, _1));
}

void
Nlsrc::fetchRtables()
{
  fetchFromRt<nlsr::RoutingTableStatus>([this] (const auto& rts) { this->recordRtable(rts); });
}

template <class T>
void
Nlsrc::fetchFromLsdb(const ndn::Name::Component& datasetType,
                     const std::function<void(const T&)>& recordLsa)
{
  ndn::Interest interest(ndn::Name(LSDB_PREFIX).append(datasetType));

  auto fetcher = ndn::util::SegmentFetcher::start(m_face, interest, m_validator);
  fetcher->onComplete.connect(std::bind(&Nlsrc::onFetchSuccess<T>, this, _1, recordLsa));
  fetcher->onError.connect(std::bind(&Nlsrc::onTimeout, this, _1, _2));
}

void
Nlsrc::recordLsa(const nlsr::Lsa& lsa)
{
  Router& router = m_routers.emplace(lsa.getOriginRouter(), Router()).first->second;

  if (lsa.getType() == nlsr::Lsa::Type::ADJACENCY) {
    router.adjacencyLsaString = lsa.toString();
  }
  else if (lsa.getType() == nlsr::Lsa::Type::COORDINATE) {
    router.coordinateLsaString = lsa.toString();
  }
  else if (lsa.getType() == nlsr::Lsa::Type::NAME) {
    router.nameLsaString = lsa.toString();
  }
}

template <class T>
void
Nlsrc::fetchFromRt(const std::function<void(const T&)>& recordDataset)
{
  ndn::Interest interest(RT_PREFIX);

  auto fetcher = ndn::util::SegmentFetcher::start(m_face, interest, m_validator);
  fetcher->onComplete.connect(std::bind(&Nlsrc::onFetchSuccess<T>, this, _1, recordDataset));
  fetcher->onError.connect(std::bind(&Nlsrc::onTimeout, this, _1, _2));
}

template <class T>
void
Nlsrc::onFetchSuccess(const ndn::ConstBufferPtr& data,
                      const std::function<void(const T&)>& recordDataset)
{
  ndn::Block block;
  size_t offset = 0;

  while (offset < data->size()) {
    bool isOk = false;
    std::tie(isOk, block) = ndn::Block::fromBuffer(data, offset);

    if (!isOk) {
      std::cerr << "ERROR: cannot decode LSA TLV" << std::endl;
      break;
    }

    offset += block.size();

    T data(block);
    recordDataset(data);
  }

  runNextStep();
}

void
Nlsrc::onTimeout(uint32_t errorCode, const std::string& error)
{
  std::cerr << "Request timed out (code: " << errorCode
            << ", error: " << error << ")"  << std::endl;
}

void
Nlsrc::recordRtable(const nlsr::RoutingTableStatus& rts)
{
  std::ostringstream os;
  os << rts;
  m_rtString = os.str();
}

void
Nlsrc::printLsdb()
{
  std::cout << "LSDB:" << std::endl;

  for (const auto& item : m_routers) {
    std::cout << "  OriginRouter: " << item.first << std::endl;
    std::cout << std::endl;

    const Router& router = item.second;

    if (!router.adjacencyLsaString.empty()) {
      std::cout << router.adjacencyLsaString << std::endl;
    }

    if (!router.coordinateLsaString.empty()) {
      std::cout << router.coordinateLsaString << std::endl;
    }

    if (!router.nameLsaString.empty()) {
      std::cout << router.nameLsaString << std::endl;
    }
  }
}

void
Nlsrc::printRT()
{
  if (!m_rtString.empty()) {
    std::cout << m_rtString;
  }
  else {
    std::cout << "Routing Table is not calculated yet" << std::endl;
  }
}

void
Nlsrc::printAll()
{
  std::cout << "NLSR Status" << std::endl;
  printLsdb();
  printRT();
}

} // namespace nlsrc

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int
main(int argc, char** argv)
{
  ndn::Face face;
  nlsrc::Nlsrc nlsrc(face);

  nlsrc.programName = argv[0];

  if (argc < 2) {
    nlsrc.printUsage();
    return 0;
  }

  int opt;
  while ((opt = ::getopt(argc, argv, "hV")) != -1) {
    switch (opt) {
    case 'h':
      nlsrc.printUsage();
      return 0;
    case 'V':
      std::cout << NLSR_VERSION_BUILD_STRING << std::endl;
      return 0;
    default:
      nlsrc.printUsage();
      return 1;
    }
  }

  if (argc == ::optind) {
    nlsrc.printUsage();
    return 1;
  }

  try {
    ::optind = 3; // Set ::optind to the command's index

    nlsrc.commandLineArguments = argv + ::optind;
    nlsrc.nOptions = argc - ::optind;

    // argv[1] points to the command, so pass it to the dispatch
    bool isOk = nlsrc.dispatch(argv[1]);
    if (!isOk) {
      nlsrc.printUsage();
      return 1;
    }

    face.processEvents();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }
  return 0;
}
