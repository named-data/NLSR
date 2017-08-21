/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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
#include <ndn-cxx/mgmt/nfd/control-response.hpp>
#include <ndn-cxx/tag.hpp>
#include <ndn-cxx/face.hpp>

namespace nlsr {
namespace update {

INIT_LOGGER("PrefixUpdateProcessor");

/** \brief an Interest tag to indicate command signer
 */
using SignerTag = ndn::SimpleTag<ndn::Name, 20>;

/** \brief obtain signer from SignerTag attached to Interest, if available
 */
static ndn::optional<std::string>
getSignerFromTag(const ndn::Interest& interest)
{
  shared_ptr<SignerTag> signerTag = interest.getTag<SignerTag>();
  if (signerTag == nullptr) {
    return ndn::nullopt;
  }
  else {
    return signerTag->get().toUri();
  }
}

PrefixUpdateProcessor::PrefixUpdateProcessor(ndn::mgmt::Dispatcher& dispatcher,
                                             ndn::Face& face,
                                             NamePrefixList& namePrefixList,
                                             Lsdb& lsdb)
  : CommandManagerBase(dispatcher, namePrefixList, lsdb, "prefix-update")

  , m_validator(ndn::make_unique<ndn::security::v2::CertificateFetcherDirectFetch>(face))
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
      [reject] (const ndn::Interest& request, const ndn::security::v2::ValidationError& error) {
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

} // namespace update
} // namespace nlsr
