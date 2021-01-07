/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  The University of Memphis,
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
 */

#ifndef NLSR_SEQUENCING_MANAGER_HPP
#define NLSR_SEQUENCING_MANAGER_HPP

#include "conf-parameter.hpp"
#include "lsa/lsa.hpp"
#include "test-access-control.hpp"

#include <ndn-cxx/face.hpp>

#include <list>
#include <string>

namespace nlsr {

class SequencingManager
{
public:
  SequencingManager(const std::string& filePath, int hypState);

  void
  setLsaSeq(uint64_t seqNo, Lsa::Type lsaType)
  {
    switch (lsaType) {
      case Lsa::Type::ADJACENCY:
        m_adjLsaSeq = seqNo;
        break;
      case Lsa::Type::COORDINATE:
        m_corLsaSeq = seqNo;
        break;
      case Lsa::Type::NAME:
        m_nameLsaSeq = seqNo;
        break;
      default:
        return;
    }
  }

  uint64_t
  getLsaSeq(Lsa::Type lsaType)
  {
    switch (lsaType) {
      case Lsa::Type::ADJACENCY:
        return m_adjLsaSeq;
      case Lsa::Type::COORDINATE:
        return m_corLsaSeq;
      case Lsa::Type::NAME:
        return m_nameLsaSeq;
      default:
        return 0;
    }
  }

  uint64_t
  getNameLsaSeq() const
  {
    return m_nameLsaSeq;
  }

  void
  setNameLsaSeq(uint64_t nlsn)
  {
    m_nameLsaSeq = nlsn;
  }

  uint64_t
  getAdjLsaSeq() const
  {
    return m_adjLsaSeq;
  }

  void
  setAdjLsaSeq(uint64_t alsn)
  {
    m_adjLsaSeq = alsn;
  }

  uint64_t
  getCorLsaSeq() const
  {
    return m_corLsaSeq;
  }

  void
  setCorLsaSeq(uint64_t clsn)
  {
    m_corLsaSeq = clsn;
  }

  void
  increaseNameLsaSeq()
  {
    m_nameLsaSeq++;
  }

  void
  increaseAdjLsaSeq()
  {
    m_adjLsaSeq++;
  }

  void
  increaseCorLsaSeq()
  {
    m_corLsaSeq++;
  }

  void
  writeSeqNoToFile() const;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  void
  initiateSeqNoFromFile();

private:
  /*! \brief Set the sequence file directory

    If the string is empty, home directory is set as sequence file directory

  \param filePath The directory where sequence file will be stored
 */
  void
  setSeqFileDirectory(const std::string& filePath);

  void
  writeLog() const;

private:
  uint64_t m_nameLsaSeq = 0;
  uint64_t m_adjLsaSeq = 0;
  uint64_t m_corLsaSeq = 0;
  std::string m_seqFileNameWithPath;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  int m_hyperbolicState;
};

} // namespace nlsr
#endif // NLSR_SEQUENCING_MANAGER_HPP
