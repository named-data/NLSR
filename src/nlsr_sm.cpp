#include <string>
#include <iostream>
#include <fstream>
#include <pwd.h>
#include <cstdlib>
#include <unistd.h>

#include "nlsr_sm.hpp"

namespace nlsr
{

  using namespace std;

  void
  SequencingManager::splittSequenceNo(uint64_t seqNo)
  {
    combinedSeqNo=seqNo;
    adjLsaSeq = (combinedSeqNo & 0xFFFFF);
    corLsaSeq = ((combinedSeqNo >> 20) & 0xFFFFF);
    nameLsaSeq = ((combinedSeqNo >> 40) & 0xFFFFF);
  }

  void
  SequencingManager::combineSequenceNo()
  {
    combinedSeqNo=0;
    combinedSeqNo = combinedSeqNo | adjLsaSeq;
    combinedSeqNo = combinedSeqNo | (corLsaSeq<<20);
    combinedSeqNo = combinedSeqNo | (nameLsaSeq<<40);
  }

  void
  SequencingManager::writeSeqNoToFile()
  {
    std::ofstream outputFile(seqFileNameWithPath.c_str(),ios::binary);
    outputFile<<combinedSeqNo;
    outputFile.close();
  }

  void
  SequencingManager::initiateSeqNoFromFile()
  {
    cout<<"Seq File Name: "<< seqFileNameWithPath<<endl;
    std::ifstream inputFile(seqFileNameWithPath.c_str(),ios::binary);
    if ( inputFile.good() )
    {
      inputFile>>combinedSeqNo;
      splittSequenceNo(combinedSeqNo);
      adjLsaSeq+=10;
      corLsaSeq+=10;
      nameLsaSeq+=10;
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
    seqFileNameWithPath=filePath;
    if( seqFileNameWithPath.empty() )
    {
      seqFileNameWithPath=getUserHomeDirectory();
    }
    seqFileNameWithPath=seqFileNameWithPath+"/nlsrSeqNo.txt";
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


