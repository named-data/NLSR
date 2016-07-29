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

#include "nlsr-runner.hpp"
#include "version.hpp"

int
main(int32_t argc, char** argv)
{
  using namespace nlsr;

  std::string programName(argv[0]);

  std::string configFileName = "nlsr.conf";
  bool isDaemonProcess = false;

  int32_t opt;
  while ((opt = getopt(argc, argv, "df:hV")) != -1) {
    switch (opt) {
      case 'f':
        configFileName = optarg;
        break;
      case 'd':
        isDaemonProcess = true;
        break;
      case 'V':
        std::cout << NLSR_VERSION_BUILD_STRING << std::endl;
        return EXIT_SUCCESS;
        break;
      case 'h':
      default:
        NlsrRunner::printUsage(programName);
        return EXIT_FAILURE;
    }
  }

  NlsrRunner runner(configFileName, isDaemonProcess);

  try {
    runner.run();
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
