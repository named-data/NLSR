/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
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
 */

#ifndef NLSR_TEST_HOME_FIXTURE_HPP
#define NLSR_TEST_HOME_FIXTURE_HPP

#include "boost-test.hpp"

#include <fstream>

#include <ndn-cxx/security/v2/key-chain.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace nlsr {
namespace tests {

/**
 * @brief Fixture to adjust/restore NDN_CLIENT_PIB and NDN_CLIENT_TPM paths
 *
 * Note that the specified PATH will be removed after fixture is destroyed.
 * **Do not specify non-temporary paths.**
 */
template<class Path>
class PibDirFixture
{
public:
  PibDirFixture()
    : m_pibDir(Path().PATH)
  {
    if (getenv("NDN_CLIENT_PIB") != nullptr) {
      m_oldPib = getenv("NDN_CLIENT_PIB");
    }
    if (getenv("NDN_CLIENT_TPM") != nullptr) {
      m_oldTpm = getenv("NDN_CLIENT_TPM");
    }

    /// @todo Consider change to an in-memory PIB/TPM
    setenv("NDN_CLIENT_PIB", ("pib-sqlite3:" + m_pibDir).c_str(), true);
    setenv("NDN_CLIENT_TPM", ("tpm-file:" + m_pibDir).c_str(), true);
  }

  ~PibDirFixture()
  {
    if (!m_oldPib.empty()) {
      setenv("NDN_CLIENT_PIB", m_oldPib.c_str(), true);
    }
    else {
      unsetenv("NDN_CLIENT_PIB");
    }

    if (!m_oldTpm.empty()) {
      setenv("NDN_CLIENT_TPM", m_oldTpm.c_str(), true);
    }
    else {
      unsetenv("NDN_CLIENT_TPM");
    }

    boost::filesystem::remove_all(m_pibDir);
    //const_cast<std::string&>(ndn::security::v2::KeyChain::getDefaultPibLocator()).clear();
    //const_cast<std::string&>(ndn::security::v2::KeyChain::getDefaultTpmLocator()).clear();
  }

protected:
  const std::string m_pibDir;

private:
  std::string m_oldPib;
  std::string m_oldTpm;
};

/**
 * @brief Extension of PibDirFixture to set TEST_HOME variable and allow config file creation
 */
template<class Path>
class TestHomeFixture : public PibDirFixture<Path>
{
public:
  TestHomeFixture()
  {
    setenv("TEST_HOME", this->m_pibDir.c_str(), true);
  }

  ~TestHomeFixture()
  {
    unsetenv("TEST_HOME");
  }

  void
  createClientConf(std::initializer_list<std::string> lines)
  {
    boost::filesystem::create_directories(boost::filesystem::path(this->m_pibDir) / ".ndn");
    std::ofstream of((boost::filesystem::path(this->m_pibDir) / ".ndn" / "client.conf").c_str());
    for (auto line : lines) {
      boost::replace_all(line, "%PATH%", this->m_pibDir);
      of << line << std::endl;
    }
  }
};

struct DefaultPibDir
{
  const std::string PATH = "build/keys";
};

} // namespace tests
} // namespace nlsr

#endif // NLSR_TEST_HOME_FIXTURE_HPP
