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

#include "nlsrc.hpp"

#include "version.hpp"
#include "src/publisher/lsa-publisher.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/management/nfd-control-parameters.hpp>
#include <ndn-cxx/management/nfd-control-response.hpp>
#include <ndn-cxx/util/segment-fetcher.hpp>

#include <iostream>

namespace nlsrc {

const ndn::Name Nlsrc::LOCALHOST_PREFIX = ndn::Name("/localhost/nlsr");
const ndn::Name Nlsrc::LSDB_PREFIX = ndn::Name(Nlsrc::LOCALHOST_PREFIX).append("lsdb");
const ndn::Name Nlsrc::NAME_UPDATE_PREFIX = ndn::Name(Nlsrc::LOCALHOST_PREFIX).append("prefix-update");

const uint32_t Nlsrc::ERROR_CODE_TIMEOUT = 10060;
const uint32_t Nlsrc::RESPONSE_CODE_SUCCESS = 200;

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
    "       status\n"
    "           display NLSR status\n"
    "       advertise name\n"
    "           advertise a name prefix through NLSR\n"
    "       withdraw name\n"
    "           remove a name prefix advertised through NLSR"
    << std::endl;
}

void
Nlsrc::getStatus()
{
  m_fetchSteps.push_back(std::bind(&Nlsrc::fetchAdjacencyLsas, this));
  m_fetchSteps.push_back(std::bind(&Nlsrc::fetchCoordinateLsas, this));
  m_fetchSteps.push_back(std::bind(&Nlsrc::fetchNameLsas, this));
  m_fetchSteps.push_back(std::bind(&Nlsrc::printLsdb, this));

  runNextStep();
}

bool
Nlsrc::dispatch(const std::string& command)
{
  if (command == "advertise") {
    if (nOptions != 1) {
      return false;
    }

    advertiseName();
    return true;
  }
  else if (command == "withdraw") {
    if (nOptions != 1) {
      return false;
    }

    withdrawName();
    return true;
  }
  else if (command == "status") {
    if (nOptions != 0) {
      return false;
    }

    getStatus();
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
  ndn::Name name = commandLineArguments[0];
  ndn::Name::Component verb("advertise");
  std::string info = "(Advertise: " + name.toUri() + ")";

  sendNamePrefixUpdate(name, verb, info);
}

void
Nlsrc::withdrawName()
{
  ndn::Name name = commandLineArguments[0];
  ndn::Name::Component verb("withdraw");
  std::string info = "(Withdraw: " + name.toUri() + ")";

  sendNamePrefixUpdate(name, verb, info);
}

void
Nlsrc::sendNamePrefixUpdate(const ndn::Name& name,
                            const ndn::Name::Component& verb,
                            const std::string& info)
{
  ndn::nfd::ControlParameters parameters;
  parameters.setName(name);

  ndn::Name commandName = NAME_UPDATE_PREFIX;
  commandName.append(verb);

  ndn::Interest interest(commandName.append(parameters.wireEncode()));
  interest.setMustBeFresh(true);
  m_keyChain.sign(interest);

  m_face.expressInterest(interest,
                         std::bind(&Nlsrc::onControlResponse, this, info, _2),
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

  if (code != RESPONSE_CODE_SUCCESS) {
    std::cerr << "Name prefix update error (code: " << code << ")" << std::endl;
    return;
  }

  std::cout << "Applied Name prefix update successfully: " << info << std::endl;
}

void
Nlsrc::fetchAdjacencyLsas()
{
  fetchFromLsdb<nlsr::tlv::AdjacencyLsa>(nlsr::AdjacencyLsaPublisher::DATASET_COMPONENT,
                                         std::bind(&Nlsrc::recordAdjacencyLsa, this, _1));
}

void
Nlsrc::fetchCoordinateLsas()
{
  fetchFromLsdb<nlsr::tlv::CoordinateLsa>(nlsr::CoordinateLsaPublisher::DATASET_COMPONENT,
                                          std::bind(&Nlsrc::recordCoordinateLsa, this, _1));
}

void
Nlsrc::fetchNameLsas()
{
  fetchFromLsdb<nlsr::tlv::NameLsa>(nlsr::NameLsaPublisher::DATASET_COMPONENT,
                                    std::bind(&Nlsrc::recordNameLsa, this, _1));
}

template <class T>
void
Nlsrc::fetchFromLsdb(const ndn::Name::Component& datasetType,
                     const std::function<void(const T&)>& recordLsa)
{
  ndn::Name command = LSDB_PREFIX;
  command.append(datasetType);

  ndn::Interest interest(command);

  ndn::util::SegmentFetcher::fetch(m_face,
                                   interest,
                                   m_validator,
                                   std::bind(&Nlsrc::onFetchSuccess<T>,
                                             this, _1, recordLsa),
                                   std::bind(&Nlsrc::onTimeout, this, _1, _2));
}

template <class T>
void
Nlsrc::onFetchSuccess(const ndn::ConstBufferPtr& data,
                      const std::function<void(const T&)>& recordLsa)
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

    T lsa(block);
    recordLsa(lsa);
  }

  runNextStep();
}

void
Nlsrc::onTimeout(uint32_t errorCode, const std::string& error)
{
  std::cerr << "Request timed out (code: " << errorCode
            << ", error: " << error << ")"  << std::endl;
}

std::string
Nlsrc::getLsaInfoString(const nlsr::tlv::LsaInfo& info)
{
  std::ostringstream os;
  os << "      info=" << info;

  return os.str();
}

void
Nlsrc::recordAdjacencyLsa(const nlsr::tlv::AdjacencyLsa& lsa)
{
  Router& router = getRouter(lsa.getLsaInfo());

  std::ostringstream os;
  os << "    AdjacencyLsa:" << std::endl;

  os << getLsaInfoString(lsa.getLsaInfo()) << std::endl;

  for (const auto& adjacency : lsa.getAdjacencies()) {
    os << "      adjacency=" << adjacency << std::endl;
  }

  router.adjacencyLsaString = os.str();
}

void
Nlsrc::recordCoordinateLsa(const nlsr::tlv::CoordinateLsa& lsa)
{
  Router& router = getRouter(lsa.getLsaInfo());

  std::ostringstream os;
  os << "    Coordinate LSA:" << std::endl;

  os << getLsaInfoString(lsa.getLsaInfo()) << std::endl;

  os << "      angle=" << lsa.getHyperbolicAngle() << std::endl;
  os << "      radius=" << lsa.getHyperbolicRadius() << std::endl;

  router.coordinateLsaString = os.str();
}

void
Nlsrc::recordNameLsa(const nlsr::tlv::NameLsa& lsa)
{
  Router& router = getRouter(lsa.getLsaInfo());

  std::ostringstream os;
  os << "    Name LSA:" << std::endl;

  os << getLsaInfoString(lsa.getLsaInfo()) << std::endl;

  for (const auto& name : lsa.getNames()) {
    os << "      name=" << name << std::endl;
  }

  router.nameLsaString = os.str();
}

void
Nlsrc::printLsdb()
{
  std::cout << "NLSR Status" << std::endl;
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

Nlsrc::Router&
Nlsrc::getRouter(const nlsr::tlv::LsaInfo& info)
{
  const ndn::Name& originRouterName = info.getOriginRouter();

  const auto& pair =
    m_routers.insert(std::make_pair(originRouterName, Router()));

  return pair.first->second;
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
    ::optind = 2; // Set ::optind to the command's index

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
