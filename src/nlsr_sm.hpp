#ifndef NLSR_SM_HPP
#define NLSR_SM_HPP

#include<list>
#include<string>
#include <ndn-cpp-dev/face.hpp>

namespace nlsr
{

    using namespace std;

    class SequencingManager
    {
    public:
        SequencingManager()
            : nameLsaSeq(0)
            , adjLsaSeq(0)
            , corLsaSeq(0)
            , combinedSeqNo(0)
            , seqFileNameWithPath()
        {
        }

        SequencingManager(uint64_t seqNo)
        {
            splittSequenceNo(seqNo);
        }

        SequencingManager(uint64_t nlsn, uint64_t alsn, uint64_t clsn)
        {
            nameLsaSeq=nlsn;
            adjLsaSeq=alsn;
            corLsaSeq=clsn;
            combineSequenceNo();
        }

        uint64_t getNameLsaSeq() const
        {
            return nameLsaSeq;
        }

        void setNameLsaSeq(uint64_t nlsn)
        {
            nameLsaSeq=nlsn;
            combineSequenceNo();
        }

        uint64_t getAdjLsaSeq() const
        {
            return adjLsaSeq;
        }

        void setAdjLsaSeq(uint64_t alsn)
        {
            adjLsaSeq=alsn;
            combineSequenceNo();
        }

        uint64_t getCorLsaSeq() const
        {
            return corLsaSeq;
        }

        void setCorLsaSeq(uint64_t clsn)
        {
            corLsaSeq=clsn;
            combineSequenceNo();
        }

        uint64_t getCombinedSeqNo() const
        {
            return combinedSeqNo;
        }

        void writeSeqNoToFile();
        void initiateSeqNoFromFile();
        void setSeqFileName(string filePath);
        string getUserHomeDirectory();

    private:
        void splittSequenceNo(uint64_t seqNo);
        void combineSequenceNo();


    private:
        uint64_t nameLsaSeq;
        uint64_t adjLsaSeq;
        uint64_t corLsaSeq;
        uint64_t combinedSeqNo;
        string seqFileNameWithPath;
    };


    ostream& operator <<(ostream& os, const SequencingManager& sm);
}//namespace nlsr
#endif
