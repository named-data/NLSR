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
 **/

#include "nlsr.hpp"
#include "nlsr_conf_processor.hpp"

using namespace nlsr;

int
main(int argc, char **argv)
{
  src::logger lg;
  nlsr::Nlsr nlsr_;
  string programName(argv[0]);
  nlsr_.setConfFileName("nlsr.conf");
  int opt;
  while ((opt = getopt(argc, argv, "df:p:h")) != -1)
  {
    switch (opt)
      {
      case 'f':
        nlsr_.setConfFileName(optarg);
        break;
      case 'd':
        nlsr_.setIsDaemonProcess(optarg);
        break;
      case 'p':
        {
          stringstream sst(optarg);
          int ap;
          sst>>ap;
          nlsr_.setApiPort(ap);
        }
        break;
      case 'h':
      default:
        nlsr_.usage(programName);
        return EXIT_FAILURE;
      }
  }
  ConfFileProcessor cfp(nlsr_.getConfFileName());
  int res=cfp.processConfFile(nlsr_);
  if ( res < 0 )
  {
    std::cerr<<"Error in configuration file processing! Exiting from NLSR"<<std::endl;
    return EXIT_FAILURE;
  }
  nlsr_.initialize();
  try
  {
    nlsr_.startEventLoop();
  }
  catch(std::exception &e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}
