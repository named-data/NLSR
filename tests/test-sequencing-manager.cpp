/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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

#include "tests/boost-test.hpp"

#include <filesystem>
#include <fstream>
#include <system_error>

namespace nlsr::tests {

using namespace ndn;

class SequencingManagerFixture
{
public:
  ~SequencingManagerFixture()
  {
    std::error_code ec;
    std::filesystem::remove(m_seqFile, ec); // ignore error
  }

  void
  writeToFile(const std::string& testSeq)
  {
    std::ofstream outputFile(m_seqFile, std::ofstream::trunc);
    outputFile << testSeq;
  }

  void
  initiateFromFile()
  {
    m_seqManager.initiateSeqNoFromFile();
  }

  void
  checkSeqNumbers(const uint64_t& name, const uint64_t& adj, const uint64_t& cor)
  {
    BOOST_CHECK_EQUAL(m_seqManager.getNameLsaSeq(), name);
    BOOST_CHECK_EQUAL(m_seqManager.getAdjLsaSeq(), adj);
    BOOST_CHECK_EQUAL(m_seqManager.getCorLsaSeq(), cor);
  }

public:
  SequencingManager m_seqManager{"/tmp", HYPERBOLIC_STATE_OFF};

private:
  std::filesystem::path m_seqFile{"/tmp/nlsrSeqNo.txt"};
};

BOOST_FIXTURE_TEST_SUITE(TestSequencingManager, SequencingManagerFixture)

BOOST_AUTO_TEST_CASE(SeparateSeqNumber)
{
  initiateFromFile();
  checkSeqNumbers(0, 0, 0);

  // LS
  writeToFile("NameLsaSeq 100\nAdjLsaSeq 100\nCorLsaSeq 0");
  m_seqManager.m_hyperbolicState = HYPERBOLIC_STATE_OFF;
  initiateFromFile();
  checkSeqNumbers(100 + 10, 100 + 10, 0);

  // HR
  writeToFile("NameLsa 100\nAdjLsa 0\nCorLsa 100");
  m_seqManager.m_hyperbolicState = HYPERBOLIC_STATE_ON;
  initiateFromFile();
  // AdjLsa is set to 0 since HR is on
  checkSeqNumbers(100 + 10, 0, 100 + 10);
}

BOOST_AUTO_TEST_CASE(CorruptFile)
{
  writeToFile("NameLsaSeq");
  initiateFromFile();
  checkSeqNumbers(10, 10, 0);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nlsr::tests
