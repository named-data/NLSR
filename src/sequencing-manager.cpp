/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
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

#include "sequencing-manager.hpp"
#include "logger.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <pwd.h>
#include <cstdlib>
#include <unistd.h>
#include <boost/algorithm/string.hpp>

namespace nlsr {

INIT_LOGGER(SequencingManager);

SequencingManager::SequencingManager(std::string filePath, int hypState)
  : m_nameLsaSeq(0)
  , m_adjLsaSeq(0)
  , m_corLsaSeq(0)
  , m_hyperbolicState(hypState)
{
  setSeqFileDirectory(filePath);
  initiateSeqNoFromFile();
}

void
SequencingManager::writeSeqNoToFile() const
{
  writeLog();
  std::ofstream outputFile(m_seqFileNameWithPath.c_str());
  std::ostringstream os;
  os << "NameLsaSeq " << std::to_string(m_nameLsaSeq) << "\n"
     << "AdjLsaSeq "  << std::to_string(m_adjLsaSeq)  << "\n"
     << "CorLsaSeq "  << std::to_string(m_corLsaSeq);
  outputFile << os.str();
  outputFile.close();
}

void
SequencingManager::initiateSeqNoFromFile()
{
  NLSR_LOG_DEBUG("Seq File Name: " << m_seqFileNameWithPath);
  std::ifstream inputFile(m_seqFileNameWithPath.c_str());

  // Good checks that file is not (bad or eof or fail)
  if (inputFile.good()) {
    std::string lsaOrCombinedSeqNo;
    uint64_t seqNo = 0;

    // If file has a combined seq number, lsaOrCombinedSeqNo would hold it
    // and seqNo will be zero everytime
    inputFile >> lsaOrCombinedSeqNo >> seqNo;
    m_nameLsaSeq = seqNo;

    inputFile >> lsaOrCombinedSeqNo >> seqNo;
    m_adjLsaSeq = seqNo;

    inputFile >> lsaOrCombinedSeqNo >> seqNo;
    m_corLsaSeq = seqNo;

    // File was in old format and had a combined sequence number
    // if all of the seqNo should are still zero and
    // lsaOrCombinedSeqNo != CorLsaSeq
    if (m_nameLsaSeq == 0 && m_adjLsaSeq == 0 && m_corLsaSeq == 0 &&
        lsaOrCombinedSeqNo != "CorLsaSeq") {
      NLSR_LOG_DEBUG("Old file had combined sequence number: " << lsaOrCombinedSeqNo);
      std::istringstream iss(lsaOrCombinedSeqNo);
      iss >> seqNo;
      m_adjLsaSeq = (seqNo & 0xFFFFF);
      m_corLsaSeq = ((seqNo >> 20) & 0xFFFFF);
      m_nameLsaSeq = ((seqNo >> 40) & 0xFFFFFF);
    }

    inputFile.close();

    m_nameLsaSeq += 10;

    // Increment the adjacency LSA seq. no. if link-state or dry HR is enabled
    if (m_hyperbolicState != HYPERBOLIC_STATE_ON) {
      if (m_corLsaSeq != 0) {
        NLSR_LOG_WARN("This router was previously configured for hyperbolic"
                   << " routing without clearing the seq. no. file.");
        m_corLsaSeq = 0;
      }
      m_adjLsaSeq += 10;
    }

    // Similarly, increment the coordinate LSA seq. no only if link-state is disabled.
    if (m_hyperbolicState != HYPERBOLIC_STATE_OFF) {
      if (m_adjLsaSeq != 0) {
        NLSR_LOG_WARN("This router was previously configured for link-state"
                  << " routing without clearing the seq. no. file.");
        m_adjLsaSeq = 0;
      }
      m_corLsaSeq += 10;
    }
  }
  writeLog();
}

void
SequencingManager::setSeqFileDirectory(const std::string& filePath)
{
  m_seqFileNameWithPath = filePath;

  if (m_seqFileNameWithPath.empty()) {
    std::string homeDirPath(getpwuid(getuid())->pw_dir);
    if (homeDirPath.empty()) {
      homeDirPath = getenv("HOME");
    }
    m_seqFileNameWithPath = homeDirPath;
  }
  m_seqFileNameWithPath = m_seqFileNameWithPath + "/nlsrSeqNo.txt";
}

void
SequencingManager::writeLog() const
{
  NLSR_LOG_DEBUG("----SequencingManager----");
  NLSR_LOG_DEBUG("Adj LSA seq no: " << m_adjLsaSeq);
  NLSR_LOG_DEBUG("Cor LSA Seq no: " << m_corLsaSeq);
  NLSR_LOG_DEBUG("Name LSA Seq no: " << m_nameLsaSeq);
}

} // namespace nlsr
