/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023,  The University of Memphis,
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

#include "lsa/adj-lsa.hpp"
#include "lsa/coordinate-lsa.hpp"
#include "lsa/name-lsa.hpp"
#include "route/routing-table.hpp"

#include <boost/noncopyable.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/validator.hpp>

#include <deque>
#include <map>
#include <stdexcept>

#ifndef NLSR_TOOLS_NLSRC_HPP
#define NLSR_TOOLS_NLSRC_HPP

namespace nlsrc {

class Nlsrc : boost::noncopyable
{
public:
  explicit
  Nlsrc(std::string programName, ndn::Face& face);

  void
  printUsage() const;

  const ndn::Name&
  getRouterPrefix() const
  {
    return m_routerPrefix;
  }

  void
  setRouterPrefix(ndn::Name prefix);

  void
  disableValidator();

  bool
  enableValidator(const std::string& filename);

  void
  getStatus(const std::string& command);

  bool
  dispatch(ndn::span<std::string> subcommand);

  int
  getExitCode() const
  {
    return m_exitCode;
  }

private:
  void
  runNextStep();

  /**
   * \brief Adds a name prefix to be advertised in NLSR's Name LSA
   *
   * cmd format:
   *   name
   *
   */
  void
  advertiseName(ndn::Name name, bool wantSave);

  /**
   * \brief Removes a name prefix from NLSR's Name LSA
   *
   * cmd format:
   *  name
   *
   */
  void
  withdrawName(ndn::Name name, bool wantDelete);

  void
  sendNamePrefixUpdate(const ndn::Name& name,
                       const ndn::Name::Component& verb,
                       const std::string& info,
                       bool saveFlag);

  void
  onControlResponse(const std::string& info, const ndn::Data& data);

private:
  void
  fetchAdjacencyLsas();

  void
  fetchCoordinateLsas();

  void
  fetchNameLsas();

  template<class T>
  void
  fetchFromLsdb(const ndn::Name::Component& datasetType,
                const std::function<void(const T&)>& recordLsa);

  void
  recordLsa(const nlsr::Lsa& lsa);

  void
  fetchRtables();

  template<class T>
  void
  fetchFromRt(const std::function<void(const T&)>& recordLsa);

  template<class T>
  void
  onFetchSuccess(const ndn::ConstBufferPtr& data,
                 const std::function<void(const T&)>& recordLsa);

  void
  onTimeout(uint32_t errorCode, const std::string& error);

  void
  recordRtable(const nlsr::RoutingTableStatus& rts);

  void
  printLsdb();

  void
  printRT();

  void
  printAll();

private:
  std::string m_programName;
  ndn::Name m_routerPrefix;
  std::unique_ptr<ndn::security::Validator> m_validator;
  ndn::KeyChain m_keyChain;
  ndn::Face& m_face;

  struct Router
  {
    std::string adjacencyLsaString;
    std::string coordinateLsaString;
    std::string nameLsaString;
  };
  std::map<ndn::Name, Router> m_routers;
  std::string m_rtString;
  std::deque<std::function<void()>> m_fetchSteps;

  int m_exitCode = 0;
};

} // namespace nlsrc

#endif // NLSR_TOOLS_NLSRC_HPP
