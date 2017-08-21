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

#ifndef NLSR_UPDATE_PREFIX_UPDATE_PROCESSOR_HPP
#define NLSR_UPDATE_PREFIX_UPDATE_PROCESSOR_HPP

#include "manager-base.hpp"
#include "prefix-update-commands.hpp"
#include "test-access-control.hpp"

#include <ndn-cxx/util/io.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/v2/certificate-storage.hpp>
#include <ndn-cxx/util/io.hpp>

#include <boost/property_tree/ptree.hpp>
#include <memory>

namespace nlsr {

namespace security {
  class CertificateStore;
} // namespace security

namespace update {

typedef boost::property_tree::ptree ConfigSection;

class PrefixUpdateProcessor : public CommandManagerBase
{
public:
  PrefixUpdateProcessor(ndn::mgmt::Dispatcher& dispatcher,
                        ndn::Face& face,
                        NamePrefixList& namePrefixList,
                        Lsdb& lsdb);

  /*! \brief Load the validator's configuration from a section of a
   * configuration file.
   * \sa ConfFileProcessor::processConfFile
   * \sa ConfFileProcessor::processConfSectionSecurity
   *
   * Loads the state of the validator for prefix update commands by
   * reading a section from a configuration file. This function is
   * expecting the section to be from a Boost property tree object.
   *
   * \throws PrefixUpdateProcessor::Error If configuration fails to load successfully
   */
  void
  loadValidator(ConfigSection section, const std::string& filename);

  ndn::security::ValidatorConfig&
  getValidator()
  {
    return m_validator;
  }

private:
  /*! \brief an authorization function for prefix-update module and verb(advertise/withdraw)
   *  accept if the verb is advertise/withdraw
   *  reject if the verb is not advertise/withdraw
      \retval an Authorization function
      \sa Nlsr::getDispatcher()
   */
  ndn::mgmt::Authorization
  makeAuthorization();

private:
  ndn::security::ValidatorConfig m_validator;
};

} // namespace update
} // namespace nlsr

#endif // NLSR_UPDATE_PREFIX_UPDATE_PROCESSOR_HPP
