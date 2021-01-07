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

#include "prefix-update-processor.hpp"
#include "lsdb.hpp"
#include "nlsr.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/mgmt/nfd/control-response.hpp>

#include <boost/algorithm/string.hpp>
#include <algorithm>

namespace nlsr {
namespace update {

INIT_LOGGER(update.PrefixUpdateProcessor);

/** \brief an Interest tag to indicate command signer
 */
using SignerTag = ndn::SimpleTag<ndn::Name, 20>;

/** \brief obtain signer from SignerTag attached to Interest, if available
 */
static ndn::optional<std::string>
getSignerFromTag(const ndn::Interest& interest)
{
  auto signerTag = interest.getTag<SignerTag>();
  if (signerTag == nullptr) {
    return ndn::nullopt;
  }
  else {
    return signerTag->get().toUri();
  }
}

PrefixUpdateProcessor::PrefixUpdateProcessor(ndn::mgmt::Dispatcher& dispatcher,
                                             ndn::security::ValidatorConfig& validator,
                                             NamePrefixList& namePrefixList,
                                             Lsdb& lsdb, const std::string& configFileName)
  : CommandManagerBase(dispatcher, namePrefixList, lsdb, "prefix-update")
  , m_validator(validator)
  , m_confFileNameDynamic(configFileName)
{
  NLSR_LOG_DEBUG("Setting dispatcher to capture Interests for: "
    << ndn::Name(Nlsr::LOCALHOST_PREFIX).append("prefix-update"));

  m_dispatcher.addControlCommand<ndn::nfd::ControlParameters>(makeRelPrefix("advertise"),
    makeAuthorization(),
    std::bind(&PrefixUpdateProcessor::validateParameters<AdvertisePrefixCommand>,
                this, _1),
    std::bind(&PrefixUpdateProcessor::advertiseAndInsertPrefix, this, _1, _2, _3, _4));

  m_dispatcher.addControlCommand<ndn::nfd::ControlParameters>(makeRelPrefix("withdraw"),
    makeAuthorization(),
    std::bind(&PrefixUpdateProcessor::validateParameters<WithdrawPrefixCommand>,
                this, _1),
    std::bind(&PrefixUpdateProcessor::withdrawAndRemovePrefix, this, _1, _2, _3, _4));
}

ndn::mgmt::Authorization
PrefixUpdateProcessor::makeAuthorization()
{
  return [=] (const ndn::Name& prefix, const ndn::Interest& interest,
              const ndn::mgmt::ControlParameters* params,
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
  fp.close();
  return false;
}

bool
PrefixUpdateProcessor::addOrDeletePrefix(const ndn::Name& prefix, bool addPrefix)
{
  std::string value = " prefix " + prefix.toUri();
  std::string fileString;
  std::string line;
  std::string trimedLine;
  std::fstream input(m_confFileNameDynamic, input.in);
  if (!input.good() || !input.is_open()) {
    NLSR_LOG_ERROR("Failed to open configuration file for parsing");
    return false;
  }

  if (addPrefix) {
    //check if prefix already exist in the nlsr configuration file
    if (checkForPrefixInFile(value)) {
      NLSR_LOG_ERROR("Prefix already exists in the configuration file");
      return false;
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
      return false;
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
  return true;
}

ndn::optional<bool>
PrefixUpdateProcessor::afterAdvertise(const ndn::Name& prefix)
{
  return addOrDeletePrefix(prefix, true);
}

ndn::optional<bool>
PrefixUpdateProcessor::afterWithdraw(const ndn::Name& prefix)
{
  return addOrDeletePrefix(prefix, false);
}

} // namespace update
} // namespace nlsr
