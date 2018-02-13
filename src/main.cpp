/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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

#include <boost/exception/get_error_info.hpp>
#include <sstream>

template<typename E>
std::string
getExtendedErrorMessage(const E& exception)
{
  std::ostringstream errorMessage;
  errorMessage << exception.what();

  const char* const* file = boost::get_error_info<boost::throw_file>(exception);
  const int* line = boost::get_error_info<boost::throw_line>(exception);
  const char* const* func = boost::get_error_info<boost::throw_function>(exception);
  if (file && line) {
    errorMessage << " [from " << *file << ":" << *line;
    if (func) {
      errorMessage << " in " << *func;
    }
    errorMessage << "]";
  }

  return errorMessage.str();
}

int
main(int32_t argc, char** argv)
{
  using namespace nlsr;

  std::string programName(argv[0]);

  std::string configFileName = "nlsr.conf";

  int32_t opt;
  while ((opt = getopt(argc, argv, "df:hV")) != -1) {
    switch (opt) {
      case 'f':
        configFileName = optarg;
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

  NlsrRunner runner(configFileName);

  try {
    runner.run();
  }
  catch (const std::exception& e) {
    std::cerr << getExtendedErrorMessage(e) << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
