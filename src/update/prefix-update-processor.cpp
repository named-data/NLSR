/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
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

#include "prefix-update-processor.hpp"
#include "logger.hpp"
#include "prefix-update-commands.hpp"

#include <boost/algorithm/string.hpp>
#include <fstream>

namespace nlsr::update {

INIT_LOGGER(update.PrefixUpdateProcessor);

/** \brief an Interest tag to indicate command signer
 */
using SignerTag = ndn::SimpleTag<ndn::Name, 20>;

/** \brief obtain signer from SignerTag attached to Interest, if available
 */
static std::optional<std::string>
getSignerFromTag(const ndn::Interest& interest)
{
  auto signerTag = interest.getTag<SignerTag>();
  if (signerTag == nullptr) {
    return std::nullopt;
  }
  else {
    return signerTag->get().toUri();
  }
}

PrefixUpdateProcessor::PrefixUpdateProcessor(ndn::mgmt::Dispatcher& dispatcher,
                                             ndn::security::ValidatorConfig& validator,
                                             NamePrefixList& namePrefixList,
                                             Lsdb& lsdb, const std::string& configFileName)
  : CommandProcessor(dispatcher, namePrefixList, lsdb)
  , m_validator(validator)
  , m_confFileNameDynamic(configFileName)
{
  m_dispatcher.addControlCommand<AdvertisePrefixCommand>(
    makeAuthorization(),
    // the first and second arguments are ignored since the handler does not need them
    std::bind(&PrefixUpdateProcessor::advertiseAndInsertPrefix, this, _3, _4));

  m_dispatcher.addControlCommand<WithdrawPrefixCommand>(
    makeAuthorization(),
    // the first and second arguments are ignored since the handler does not need them
    std::bind(&PrefixUpdateProcessor::withdrawAndRemovePrefix, this, _3, _4));
}

ndn::mgmt::Authorization
PrefixUpdateProcessor::makeAuthorization()
{
  return [=] (const ndn::Name& prefix, const ndn::Interest& interest,
              const ndn::mgmt::ControlParametersBase* params,
              const ndn::mgmt::AcceptContinuation& accept,
              const ndn::mgmt::RejectContinuation& reject) {
    m_validator.validate(interest,
      [accept] (const ndn::Interest& request) {
        auto signer1 = getSignerFromTag(request);
        std::string signer = signer1.value_or("*");
        NLSR_LOG_DEBUG("accept " << request.getName() << " signer=" << signer);
        accept(signer);
      },
      [reject] (const ndn::Interest& request, const ndn::security::ValidationError& error) {
        NLSR_LOG_DEBUG("reject " << request.getName() << " signer=" <<
                       getSignerFromTag(request).value_or("?") << ' ' << error);
        reject(ndn::mgmt::RejectReply::STATUS403);
      });
  };
}

void
PrefixUpdateProcessor::loadValidator(boost::property_tree::ptree section,
                                     const std::string& filename)
{
  m_validator.load(section, filename);
}

bool
PrefixUpdateProcessor::checkForPrefixInFile(const std::string prefix)
{
  std::string line;
  std::fstream fp(m_confFileNameDynamic);
  if (!fp.good() || !fp.is_open()) {
    NLSR_LOG_ERROR("Failed to open configuration file for parsing");
    return true;
  }
  while (!fp.eof()) {
    getline(fp, line);
    if (line == prefix) {
      return true;
    }
  }
  return false;
}

std::tuple<bool, std::string>
PrefixUpdateProcessor::addOrDeletePrefix(const ndn::Name& prefix, bool addPrefix)
{
  std::string value = " prefix " + prefix.toUri();
  std::string fileString;
  std::string line;
  std::string trimedLine;
  std::fstream input(m_confFileNameDynamic, input.in);
  if (!input.good() || !input.is_open()) {
    NLSR_LOG_ERROR("Failed to open configuration file for parsing");
    return {false, "Failed to open configuration file for parsing"};
  }

  if (addPrefix) {
    //check if prefix already exist in the nlsr configuration file
    if (checkForPrefixInFile(value)) {
      NLSR_LOG_ERROR("Prefix already exists in the configuration file");
      return {false, "Prefix already exists in the configuration file"};
    }
    while (!input.eof()) {
      getline(input, line);
      if (!line.empty()) {
        fileString.append(line + "\n");
        if (line == "advertising") {
          getline(input, line);
          fileString.append(line + "\n" + value + "\n");
        }
      }
    }
  }
  else {
    if (!checkForPrefixInFile(value)) {
      NLSR_LOG_ERROR("Prefix doesn't exists in the configuration file");
      return {false, "Prefix doesn't exists in the configuration file"};
    }
    boost::trim(value);
    while (!input.eof()) {
      getline(input, line);
      if (!line.empty()) {
        std::string trimLine = line;
        boost::trim(trimLine);
        if (trimLine != value) {
          fileString.append(line + "\n");
        }
      }
    }
  }
  input.close();
  std::ofstream output(m_confFileNameDynamic);
  output << fileString;
  output.close();
  return {true, "OK"};
}

std::tuple<bool, std::string>
PrefixUpdateProcessor::afterAdvertise(const ndn::Name& prefix)
{
  return addOrDeletePrefix(prefix, true);
}

std::tuple<bool, std::string>
PrefixUpdateProcessor::afterWithdraw(const ndn::Name& prefix)
{
  return addOrDeletePrefix(prefix, false);
}

} // namespace nlsr::update
