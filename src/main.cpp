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
 **/

#include "conf-file-processor.hpp"
#include "nlsr.hpp"
#include "security/certificate-store.hpp"
#include "version.hpp"

#include <boost/exception/diagnostic_information.hpp>
#include <iostream>

static void
printUsage(std::ostream& os, const std::string& programName)
{
  os << "Usage: " << programName << " [OPTIONS...]\n"
     << "\n"
     << "Options:\n"
     << "    -f <FILE>   Path to configuration file\n"
     << "    -h          Display this help message\n"
     << "    -V          Display version information\n"
     << std::endl;
}

int
main(int argc, char** argv)
{
  std::string programName(argv[0]);
  std::string configFileName("nlsr.conf");

  int opt;
  while ((opt = getopt(argc, argv, "hf:V")) != -1) {
    switch (opt) {
    case 'h':
      printUsage(std::cout, programName);
      return 0;
    case 'f':
      configFileName = optarg;
      break;
    case 'V':
      std::cout << NLSR_VERSION_BUILD_STRING << std::endl;
      return 0;
    default:
      printUsage(std::cerr, programName);
      return 2;
    }
  }

  ndn::KeyChain keyChain;
  ndn::Face face(nullptr, keyChain);

  nlsr::ConfParameter confParam(face, keyChain, configFileName);
  nlsr::ConfFileProcessor configProcessor(confParam);
  if (!configProcessor.processConfFile()) {
    std::cerr << "Error in configuration file processing" << std::endl;
    return 2;
  }

  // Since confParam is already populated, key is initialized here before
  // and independent of the NLSR class
  auto certificate = confParam.initializeKey();

  nlsr::Nlsr nlsr(face, keyChain, confParam);

  nlsr::security::CertificateStore certStore(face, confParam, nlsr.getLsdb());
  if (certificate) {
    certStore.insert(*certificate);
  }

  try {
    face.processEvents();
  }
  catch (const std::exception& e) {
    std::cerr << "FATAL: " << boost::diagnostic_information(e) << std::endl;
    return 1;
  }

  return 0;
}
