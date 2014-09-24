/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 * \author Yingdi Yu <yingdi@cs.ucla.edu>
 *
 **/
#include "conf-file-processor.hpp"
#include "logger.hpp"
#include "nlsr.hpp"
#include "version.hpp"

#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

#include <ndn-cxx/util/scheduler.hpp>

namespace nlsr {

int
main(int32_t argc, char** argv)
{
  boost::asio::io_service ioService;
  ndn::Scheduler scheduler(ioService);
  ndn::Face face(ioService);

  Nlsr nlsr(ioService, scheduler, face);

  std::string programName(argv[0]);
  nlsr.setConfFileName("nlsr.conf");
  int32_t opt;
  while ((opt = getopt(argc, argv, "df:hV")) != -1) {
    switch (opt)
      {
      case 'f':
        nlsr.setConfFileName(optarg);
        break;
      case 'd':
        nlsr.setIsDaemonProcess(true);
        break;
      case 'V':
        std::cout << NLSR_VERSION_BUILD_STRING << std::endl;
        return EXIT_SUCCESS;
        break;
      case 'h':
      default:
        nlsr.usage(programName);
        return EXIT_FAILURE;
      }
  }
  ConfFileProcessor cfp(nlsr, nlsr.getConfFileName());
  if (!cfp.processConfFile()) {
    std::cerr << "Error in configuration file processing! Exiting from NLSR" << std::endl;
    return EXIT_FAILURE;
  }

  INIT_LOGGERS(nlsr.getConfParameter().getLogDir(), nlsr.getConfParameter().getLogLevel());
  INIT_LOGGER("Main");

  nlsr.initialize();
  if (nlsr.getIsSetDaemonProcess()) {
    nlsr.daemonize();
  }
  try {
    nlsr.startEventLoop();
  }
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    nlsr.getFib().clean();
    nlsr.destroyFaces();
  }
  return EXIT_SUCCESS;
}

} // namespace nlsr

int
main(int32_t argc, char** argv)
{
  return nlsr::main(argc, argv);
}
