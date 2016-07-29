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

#ifndef NLSR_SEQUENCING_MANAGER_HPP
#define NLSR_SEQUENCING_MANAGER_HPP

#include <list>
#include <string>
#include <boost/cstdint.hpp>

#include <ndn-cxx/face.hpp>

#include "conf-parameter.hpp"

namespace nlsr {

class SequencingManager
{
public:
  SequencingManager()
    : m_nameLsaSeq(0)
    , m_adjLsaSeq(0)
    , m_corLsaSeq(0)
    , m_combinedSeqNo(0)
    , m_seqFileNameWithPath()
  {
  }

  SequencingManager(uint64_t seqNo)
  {
    splitSequenceNo(seqNo);
  }

  SequencingManager(uint64_t nlsn, uint64_t alsn, uint64_t clsn)
  {
    m_nameLsaSeq = nlsn;
    m_adjLsaSeq  = alsn;
    m_corLsaSeq  = clsn;
    combineSequenceNo();
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
    combineSequenceNo();
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
    combineSequenceNo();
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
    combineSequenceNo();
  }

  void
  increaseNameLsaSeq()
  {
    m_nameLsaSeq++;
    combineSequenceNo();
  }

  void
  increaseAdjLsaSeq()
  {
    m_adjLsaSeq++;
    combineSequenceNo();
  }

  void
  increaseCorLsaSeq()
  {
    m_corLsaSeq++;
    combineSequenceNo();
  }

  uint64_t
  getCombinedSeqNo() const
  {
    return m_combinedSeqNo;
  }

  void
  writeSeqNoToFile() const;

  void
  initiateSeqNoFromFile(int hypState);

  void
  setSeqFileName(std::string filePath);

  std::string
  getUserHomeDirectory();

  void
  writeLog() const;

private:
  void
  splitSequenceNo(uint64_t seqNo);

  void
  combineSequenceNo();

private:
  uint64_t m_nameLsaSeq;
  uint64_t m_adjLsaSeq;
  uint64_t m_corLsaSeq;
  uint64_t m_combinedSeqNo;
  std::string m_seqFileNameWithPath;
};

}//namespace nlsr
#endif //NLSR_SEQUENCING_MANAGER_HPP
