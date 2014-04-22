#ifndef NLSR_SM_HPP
#define NLSR_SM_HPP

#include <list>
#include <string>
#include <ndn-cpp-dev/face.hpp>

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
    splittSequenceNo(seqNo);
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

  uint64_t
  getCombinedSeqNo() const
  {
    return m_combinedSeqNo;
  }

  void
  writeSeqNoToFile();

  void
  initiateSeqNoFromFile();

  void
  setSeqFileName(std::string filePath);

  std::string
  getUserHomeDirectory();

private:
  void
  splittSequenceNo(uint64_t seqNo);

  void
  combineSequenceNo();


private:
  uint64_t m_nameLsaSeq;
  uint64_t m_adjLsaSeq;
  uint64_t m_corLsaSeq;
  uint64_t m_combinedSeqNo;
  std::string m_seqFileNameWithPath;
};


std::ostream&
operator<<(std::ostream& os, const SequencingManager& sm);

}//namespace nlsr
#endif //NLSR_SM_HPP
