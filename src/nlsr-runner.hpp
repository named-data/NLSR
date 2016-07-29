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

#ifndef NLSR_NLSR_RUNNER_HPP
#define NLSR_NLSR_RUNNER_HPP

#include "nlsr.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/util/scheduler.hpp>

// boost needs to be included after ndn-cxx, otherwise there will be conflict with _1, _2, ...
#include <boost/asio.hpp>

namespace nlsr {

class NlsrRunner
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

  NlsrRunner(std::string& configFileName, bool isDaemonProcess);

  void
  run();

  static void
  printUsage(const std::string& programName);

private:
  boost::asio::io_service m_ioService;
  ndn::Scheduler m_scheduler;
  ndn::Face m_face;

  Nlsr m_nlsr;
};

} // namespace nlsr

#endif // NLSR_NLSR_RUNNER_HPP
