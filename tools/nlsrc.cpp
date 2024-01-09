/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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

#include "config.hpp"
#include "version.hpp"
#include "src/publisher/dataset-interest-handler.hpp"

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>
#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/security/interest-signer.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/security/validator-null.hpp>
#include <ndn-cxx/util/segment-fetcher.hpp>

#include <boost/algorithm/string/replace.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <iostream>

namespace nlsrc {

const ndn::Name LOCALHOST_PREFIX("/localhost");
const ndn::PartialName LSDB_SUFFIX("nlsr/lsdb");
const ndn::PartialName NAME_UPDATE_SUFFIX("nlsr/prefix-update");
const ndn::PartialName RT_SUFFIX("nlsr/routing-table");

const uint32_t ERROR_CODE_TIMEOUT = 10060;
const uint32_t RESPONSE_CODE_SUCCESS = 200;
const uint32_t RESPONSE_CODE_NO_EFFECT = 204;
const uint32_t RESPONSE_CODE_SAVE_OR_DELETE = 205;

Nlsrc::Nlsrc(std::string programName, ndn::Face& face)
  : m_programName(std::move(programName))
  , m_routerPrefix(LOCALHOST_PREFIX)
  , m_face(face)
{
  disableValidator();
}

void
Nlsrc::printUsage() const
{
  const std::string help(R"EOT(Usage:
@NLSRC@ [-h | -V]
@NLSRC@ [-R <router prefix> [-c <nlsr.conf path> | -k]] COMMAND [<Command Options>]
       -h print usage and exit
       -V print version and exit
       -R target a remote NLSR instance
       -c verify response with nlsr.conf security.validator policy
       -k do not verify response (insecure)

   COMMAND can be one of the following:
       lsdb
           display NLSR lsdb status
       routing
           display routing table status
       status
           display all NLSR status (lsdb & routingtable)
       advertise <name>
           advertise a name prefix through NLSR
       advertise <name> save
           advertise and save the name prefix to the conf file
       withdraw <name>
           remove a name prefix advertised through NLSR
       withdraw <name> delete
           withdraw and delete the name prefix from the conf file
)EOT");
  boost::algorithm::replace_all_copy(std::ostream_iterator<char>(std::cout),
                                     help, "@NLSRC@", m_programName);
}

void
Nlsrc::setRouterPrefix(ndn::Name prefix)
{
  m_routerPrefix = std::move(prefix);
}

void
Nlsrc::disableValidator()
{
  m_validator.reset(new ndn::security::ValidatorNull());
}

bool
Nlsrc::enableValidator(const std::string& filename)
{
  using namespace boost::property_tree;
  ptree validatorConfig;
  try {
    ptree config;
    read_info(filename, config);
    validatorConfig = config.get_child("security.validator");
  }
  catch (const ptree_error& e) {
    std::cerr << "Failed to parse configuration file '" << filename
              << "': " << e.what() << std::endl;
    return false;
  }

  auto validator = std::make_unique<ndn::security::ValidatorConfig>(m_face);
  try {
    validator->load(validatorConfig, filename);
  }
  catch (const ndn::security::validator_config::Error& e) {
    std::cerr << "Failed to load validator config from '" << filename
              << "' security.validator section: " << e.what() << std::endl;
    return false;
  }

  m_validator = std::move(validator);
  return true;
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
  else if (command == "status") {
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchAdjacencyLsas, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchCoordinateLsas, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchNameLsas, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::fetchRtables, this));
    m_fetchSteps.push_back(std::bind(&Nlsrc::printAll, this));
  }
  runNextStep();
}

bool
Nlsrc::dispatch(ndn::span<std::string> subcommand)
{
  if (subcommand.size() == 0) {
    return false;
  }

  if (subcommand[0] == "advertise") {
    switch (subcommand.size()) {
      case 2:
        advertiseName(subcommand[1], false);
        return true;
      case 3:
        if (subcommand[2] != "save") {
          return false;
        }
        advertiseName(subcommand[1], true);
        return true;
    }
    return false;
  }

  if (subcommand[0] == "withdraw") {
    switch (subcommand.size()) {
      case 2:
        withdrawName(subcommand[1], false);
        return true;
      case 3:
        if (subcommand[2] != "delete") {
          return false;
        }
        withdrawName(subcommand[1], true);
        return true;
    }
    return false;
  }

  if (subcommand[0] == "lsdb" || subcommand[0] == "routing" || subcommand[0] == "status") {
    if (subcommand.size() != 1) {
      return false;
    }
    getStatus(subcommand[0]);
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
Nlsrc::advertiseName(ndn::Name name, bool wantSave)
{
  std::string info = (wantSave ? "(Save: " : "(Advertise: ") + name.toUri() + ")";
  ndn::Name::Component verb("advertise");
  sendNamePrefixUpdate(name, verb, info, wantSave);
}

void
Nlsrc::withdrawName(ndn::Name name, bool wantDelete)
{
  std::string info = (wantDelete ? "(Delete: " : "(Withdraw: ") + name.toUri() + ")";
  ndn::Name::Component verb("withdraw");
  sendNamePrefixUpdate(name, verb, info, wantDelete);
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

  auto paramWire = parameters.wireEncode();
  ndn::Name commandName = m_routerPrefix;
  commandName.append(NAME_UPDATE_SUFFIX);
  commandName.append(verb);
  commandName.append(paramWire.begin(), paramWire.end());

  ndn::security::InterestSigner signer(m_keyChain);
  auto commandInterest = signer.makeCommandInterest(commandName,
                           ndn::security::signingByIdentity(m_keyChain.getPib().getDefaultIdentity()));
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
    m_exitCode = 1;
    return;
  }

  uint32_t code = response.getCode();

  if (code != RESPONSE_CODE_SUCCESS && code != RESPONSE_CODE_SAVE_OR_DELETE) {
    std::cerr << response.getText() << std::endl;
    std::cerr << "Name prefix update error (code: " << code << ")" << std::endl;
    m_exitCode = code == RESPONSE_CODE_NO_EFFECT ? 0 : 1;
    return;
  }

  std::cout << "Applied Name prefix update successfully: " << info << std::endl;
  m_exitCode = 0;
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

template<class T>
void
Nlsrc::fetchFromLsdb(const ndn::Name::Component& datasetType,
                     const std::function<void(const T&)>& recordLsa)
{
  auto name = m_routerPrefix;
  name.append(LSDB_SUFFIX);
  name.append(datasetType);
  ndn::Interest interest(name);

  auto fetcher = ndn::SegmentFetcher::start(m_face, interest, *m_validator);
  fetcher->onComplete.connect(std::bind(&Nlsrc::onFetchSuccess<T>, this, _1, recordLsa));
  fetcher->onError.connect(std::bind(&Nlsrc::onTimeout, this, _1, _2));
}

void
Nlsrc::recordLsa(const nlsr::Lsa& lsa)
{
  Router& router = m_routers.emplace(lsa.getOriginRouter(), Router()).first->second;
  auto lsaString = boost::lexical_cast<std::string>(lsa);

  if (lsa.getType() == nlsr::Lsa::Type::ADJACENCY) {
    router.adjacencyLsaString = lsaString;
  }
  else if (lsa.getType() == nlsr::Lsa::Type::COORDINATE) {
    router.coordinateLsaString = lsaString;
  }
  else if (lsa.getType() == nlsr::Lsa::Type::NAME) {
    router.nameLsaString = lsaString;
  }
}

template<class T>
void
Nlsrc::fetchFromRt(const std::function<void(const T&)>& recordDataset)
{
  auto name = m_routerPrefix;
  name.append(RT_SUFFIX);
  ndn::Interest interest(name);

  auto fetcher = ndn::SegmentFetcher::start(m_face, interest, *m_validator);
  fetcher->onComplete.connect(std::bind(&Nlsrc::onFetchSuccess<T>, this, _1, recordDataset));
  fetcher->onError.connect(std::bind(&Nlsrc::onTimeout, this, _1, _2));
}

template<class T>
void
Nlsrc::onFetchSuccess(const ndn::ConstBufferPtr& buf,
                      const std::function<void(const T&)>& recordDataset)
{
  size_t offset = 0;
  while (offset < buf->size()) {
    auto [isOk, block] = ndn::Block::fromBuffer(buf, offset);

    if (!isOk) {
      std::cerr << "ERROR: cannot decode LSA TLV" << std::endl;
      break;
    }

    offset += block.size();

    T dataset(block);
    recordDataset(dataset);
  }

  runNextStep();
}

void
Nlsrc::onTimeout(uint32_t errorCode, const std::string& error)
{
  std::cerr << "Request timed out (code: " << errorCode
            << ", error: " << error << ")"  << std::endl;
  m_exitCode = 1;
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
  nlsrc::Nlsrc nlsrc(argv[0], face);

  if (argc < 2) {
    nlsrc.printUsage();
    return 2;
  }

  int opt;
  const char* confFile = DEFAULT_CONFIG_FILE;
  bool disableValidator = false;
  while ((opt = ::getopt(argc, argv, "hVR:c:k")) != -1) {
    switch (opt) {
    case 'h':
      nlsrc.printUsage();
      return 0;
    case 'V':
      std::cout << NLSR_VERSION_BUILD_STRING << std::endl;
      return 0;
    case 'R':
      nlsrc.setRouterPrefix(::optarg);
      break;
    case 'c':
      confFile = ::optarg;
      break;
    case 'k':
      disableValidator = true;
      break;
    default:
      nlsrc.printUsage();
      return 2;
    }
  }

  if (argc == ::optind) {
    nlsrc.printUsage();
    return 2;
  }

  if (nlsrc.getRouterPrefix() != nlsrc::LOCALHOST_PREFIX && !disableValidator) {
    if (!nlsrc.enableValidator(confFile)) {
      return 1;
    }
  }

  std::vector<std::string> subcommand(&argv[::optind], &argv[argc]);
  try {
    bool isValidSyntax = nlsrc.dispatch(subcommand);
    if (!isValidSyntax) {
      nlsrc.printUsage();
      return 2;
    }

    face.processEvents();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
  return nlsrc.getExitCode();
}
