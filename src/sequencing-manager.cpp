/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
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

#include <string>
#include <iostream>
#include <fstream>
#include <pwd.h>
#include <cstdlib>
#include <unistd.h>

#include "sequencing-manager.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("SequencingManager");

using namespace std;

void
SequencingManager::splitSequenceNo(uint64_t seqNo)
{
  m_combinedSeqNo = seqNo;
  m_adjLsaSeq = (m_combinedSeqNo & 0xFFFFF);
  m_corLsaSeq = ((m_combinedSeqNo >> 20) & 0xFFFFF);
  m_nameLsaSeq = ((m_combinedSeqNo >> 40) & 0xFFFFFF);
}

void
SequencingManager::combineSequenceNo()
{
  m_combinedSeqNo = 0;
  m_combinedSeqNo = m_combinedSeqNo | m_adjLsaSeq;
  m_combinedSeqNo = m_combinedSeqNo | (m_corLsaSeq << 20);
  m_combinedSeqNo = m_combinedSeqNo | (m_nameLsaSeq << 40);
}

void
SequencingManager::writeSeqNoToFile() const
{
  std::ofstream outputFile(m_seqFileNameWithPath.c_str(), ios::binary);
  outputFile << m_combinedSeqNo;
  outputFile.close();
}

void
SequencingManager::initiateSeqNoFromFile()
{
  _LOG_DEBUG("Seq File Name: " << m_seqFileNameWithPath);
  std::ifstream inputFile(m_seqFileNameWithPath.c_str(), ios::binary);
  if (inputFile.good()) {
    inputFile >> m_combinedSeqNo;
    splitSequenceNo(m_combinedSeqNo);
    m_adjLsaSeq += 10;
    m_corLsaSeq += 10;
    m_nameLsaSeq += 10;
    combineSequenceNo();
    inputFile.close();
  }
  else {
    splitSequenceNo(0);
  }
}

void
SequencingManager::setSeqFileName(string filePath)
{
  m_seqFileNameWithPath = filePath;
  if (m_seqFileNameWithPath.empty()) {
    m_seqFileNameWithPath = getUserHomeDirectory();
  }
  m_seqFileNameWithPath = m_seqFileNameWithPath + "/nlsrSeqNo.txt";
}

string
SequencingManager::getUserHomeDirectory()
{
  string homeDirPath(getpwuid(getuid())->pw_dir);
  if (homeDirPath.empty()) {
    homeDirPath = getenv("HOME");
  }
  return homeDirPath;
}

void
SequencingManager::writeLog() const
{
  _LOG_DEBUG("----SequencingManager----");
  _LOG_DEBUG("Adj LSA seq no: " << m_adjLsaSeq);
  _LOG_DEBUG("Cor LSA Seq no: " << m_corLsaSeq);
  _LOG_DEBUG("Name LSA Seq no: " << m_nameLsaSeq);
  _LOG_DEBUG("Combined LSDB Seq no: " << m_combinedSeqNo);
}

}//namespace nlsr


