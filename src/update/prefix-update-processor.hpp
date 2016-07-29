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

#ifndef UPDATE_PREFIX_UPDATE_PROCESSOR_HPP
#define UPDATE_PREFIX_UPDATE_PROCESSOR_HPP

#include "name-prefix-list.hpp"
#include "test-access-control.hpp"
#include "validator.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/management/nfd-control-command.hpp>
#include <ndn-cxx/management/nfd-control-parameters.hpp>
#include <ndn-cxx/security/certificate-cache-ttl.hpp>
#include <ndn-cxx/security/key-chain.hpp>

#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>

namespace nlsr {

class Lsdb;
class SyncLogicHandler;

namespace security {
  class CertificateStore;
}

namespace update {

typedef boost::property_tree::ptree ConfigSection;

class PrefixUpdateProcessor : boost::noncopyable
{
public:
  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

public:
  PrefixUpdateProcessor(ndn::Face& face,
                        NamePrefixList& namePrefixList,
                        Lsdb& lsdb,
                        SyncLogicHandler& sync,
                        const ndn::Name broadcastPrefix,
                        ndn::KeyChain& keyChain,
                        ndn::shared_ptr<ndn::CertificateCacheTtl> certificateCache,
                        security::CertificateStore& certStore);

  void
  loadValidator(ConfigSection section, const std::string& filename);

  void
  startListening();

  void
  enable()
  {
    m_isEnabled = true;
  }

private:
  void
  onInterest(const ndn::Interest& request);

  void
  sendNack(const ndn::Interest& request);

  void
  sendResponse(const std::shared_ptr<const ndn::Interest>& request,
               uint32_t code,
               const std::string& text);

  /** \brief adds desired name prefix to the advertised name prefix list
   */
  void
  advertise(const std::shared_ptr<const ndn::Interest>& request,
            const ndn::nfd::ControlParameters& parameters);

  /** \brief removes desired name prefix from the advertised name prefix list
   */
  void
  withdraw(const std::shared_ptr<const ndn::Interest>& request,
           const ndn::nfd::ControlParameters& parameters);

  void
  onCommandValidated(const std::shared_ptr<const ndn::Interest>& request);

  void
  onCommandValidationFailed(const std::shared_ptr<const ndn::Interest>& request,
                            const std::string& failureInfo);

  static bool
  extractParameters(const ndn::name::Component& parameterComponent,
                    ndn::nfd::ControlParameters& extractedParameters);

  bool
  validateParameters(const ndn::nfd::ControlCommand& command,
                     const ndn::nfd::ControlParameters& parameters);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  Validator&
  getValidator()
  {
    return m_validator;
  }

private:
  ndn::Face& m_face;
  NamePrefixList& m_namePrefixList;
  Lsdb& m_lsdb;
  SyncLogicHandler& m_sync;
  ndn::KeyChain& m_keyChain;
  Validator m_validator;
  bool m_isEnabled;

  const ndn::Name COMMAND_PREFIX; // /localhost/nlsr/prefix-update

  static const ndn::Name::Component MODULE_COMPONENT;
  static const ndn::Name::Component ADVERTISE_VERB;
  static const ndn::Name::Component WITHDRAW_VERB;
};

} // namespace update
} // namespace nlsr

#endif // UPDATE_PREFIX_UPDATE_PROCESSOR_HPP
