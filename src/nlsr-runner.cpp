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

#include "nlsr-runner.hpp"
#include "conf-file-processor.hpp"

namespace nlsr {

NlsrRunner::NlsrRunner(const std::string& configFileName)
  : m_scheduler(m_ioService)
  , m_face(m_ioService)
  , m_nlsr(m_ioService, m_scheduler, m_face, m_keyChain)
{
  m_nlsr.setConfFileName(configFileName);
}

void
NlsrRunner::run()
{
  ConfFileProcessor configProcessor(m_nlsr, m_nlsr.getConfFileName());

  if (!configProcessor.processConfFile()) {
    BOOST_THROW_EXCEPTION(ConfFileError("Error in configuration file processing"));
  }

  m_nlsr.initialize();

  try {
    m_nlsr.startEventLoop();
  }
  catch (...) {
    m_nlsr.getFib().clean();
    throw;
  }
}

} // namespace nlsr
