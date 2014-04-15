#include <string>
#include <iostream>
#include <fstream>
#include <pwd.h>
#include <cstdlib>
#include <unistd.h>

#include "nlsr_sm.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_sm.cpp"

namespace nlsr
{

  using namespace std;

  void
  SequencingManager::splittSequenceNo(uint64_t seqNo)
  {
    m_combinedSeqNo=seqNo;
    m_adjLsaSeq = (m_combinedSeqNo & 0xFFFFF);
    m_corLsaSeq = ((m_combinedSeqNo >> 20) & 0xFFFFF);
    m_nameLsaSeq = ((m_combinedSeqNo >> 40) & 0xFFFFF);
  }

  void
  SequencingManager::combineSequenceNo()
  {
    m_combinedSeqNo=0;
    m_combinedSeqNo = m_combinedSeqNo | m_adjLsaSeq;
    m_combinedSeqNo = m_combinedSeqNo | (m_corLsaSeq<<20);
    m_combinedSeqNo = m_combinedSeqNo | (m_nameLsaSeq<<40);
  }

  void
  SequencingManager::writeSeqNoToFile()
  {
    std::ofstream outputFile(m_seqFileNameWithPath.c_str(),ios::binary);
    outputFile<<m_combinedSeqNo;
    outputFile.close();
  }

  void
  SequencingManager::initiateSeqNoFromFile()
  {
    cout<<"Seq File Name: "<< m_seqFileNameWithPath<<endl;
    std::ifstream inputFile(m_seqFileNameWithPath.c_str(),ios::binary);
    if ( inputFile.good() )
    {
      inputFile>>m_combinedSeqNo;
      splittSequenceNo(m_combinedSeqNo);
      m_adjLsaSeq+=10;
      m_corLsaSeq+=10;
      m_nameLsaSeq+=10;
      combineSequenceNo();
      inputFile.close();
    }
    else
    {
      splittSequenceNo(0);
    }
  }

  void
  SequencingManager::setSeqFileName(string filePath)
  {
    m_seqFileNameWithPath=filePath;
    if( m_seqFileNameWithPath.empty() )
    {
      m_seqFileNameWithPath=getUserHomeDirectory();
    }
    m_seqFileNameWithPath=m_seqFileNameWithPath+"/nlsrSeqNo.txt";
  }

  string
  SequencingManager::getUserHomeDirectory()
  {
    string homeDirPath(getpwuid(getuid())->pw_dir);
    if( homeDirPath.empty() )
    {
      homeDirPath = getenv("HOME");
    }
    return homeDirPath;
  }

  ostream&
  operator <<(ostream& os, const SequencingManager& sm)
  {
    std::cout<<"----SequencingManager----"<<std::endl;
    std::cout<<"Adj LSA seq no: "<<sm.getAdjLsaSeq()<<endl;
    std::cout<<"Cor LSA Seq no: "<<sm.getCorLsaSeq()<<endl;
    std::cout<<"Name LSA Seq no: "<<sm.getNameLsaSeq()<<endl;
    std::cout<<"Combined LSDB Seq no: "<<sm.getCombinedSeqNo()<<endl;
    return os;
  }

}//namespace nlsr


