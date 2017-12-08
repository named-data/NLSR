/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  The University of Memphis,
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

#ifndef NLSR_NLSR_RUNNER_HPP
#define NLSR_NLSR_RUNNER_HPP

#include "nlsr.hpp"
#include "conf-parameter.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/util/scheduler.hpp>

namespace nlsr {

/*! \brief A wrapper class to instantiate and configure an NLSR object.
 *
 * As its name suggests, this class is responsible for running
 * NLSR. It creates an nlsr::ConfFileProcessor to read a configuration
 * file and uses that to configure and then start an NLSR process.
 * This class only exists to provide this functionality, and there is
 * no special reliance of NLSR on this class.
 */
class NlsrRunner
{
public:
  explicit
  NlsrRunner(ndn::Face& face, ConfParameter& confParam);

  /*! \brief Instantiate, configure, and start the NLSR process.
   *
   * Each NlsrRunner is uniquely paired to the Face it's instantiated
   * with. This is *not* a factory-type class, but a one-to-one
   * relationship. If one wants to create multiple NLSR classes,
   * multiple NLSR runners need to be created, too.
   *
   * \throws ConfFileError The configuration file cannot be processed.
   *                       NLSR is not started.
   * \sa Nlsr::canonizeNeighborUris
   */
  void
  run();

private:
  ndn::Face& m_face;
  ndn::KeyChain m_keyChain;
  ConfParameter& m_confParam;
  Nlsr m_nlsr;
};

} // namespace nlsr

#endif // NLSR_NLSR_RUNNER_HPP
