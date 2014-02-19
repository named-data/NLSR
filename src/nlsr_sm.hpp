#ifndef NLSR_SM_HPP
#define NLSR_SM_HPP

#include<list>
#include<string>
#include <ndn-cpp-dev/face.hpp>

namespace nlsr {

using namespace std;

class SequencingManager
{
public:
	SequencingManager()
		: nameLsaSeq(0)
		, adjLsaSeq(0)
		, corLsaSeq(0)
	{
	}

	SequencingManager(uint32_t nlsn, uint32_t alsn, uint32_t clsn)
	{
		nameLsaSeq=nlsn;
		adjLsaSeq=alsn;
		corLsaSeq=clsn;
	}
	
	uint32_t getNameLsaSeq()
	{
		return nameLsaSeq;
	}

	void setNameLsaSeq(uint32_t nlsn){
		nameLsaSeq=nlsn;
	}

	uint32_t getAdjLsaSeq()
	{
		return adjLsaSeq;
	}

	void setAdjLsaSeq(uint32_t alsn){
		adjLsaSeq=alsn;
	}

	uint32_t getCorLsaSeq()
	{
		return corLsaSeq;
	}

	void setCorLsaSeq(uint32_t clsn){
		corLsaSeq=clsn;
	}

private:
	uint32_t nameLsaSeq;
	uint32_t adjLsaSeq;
	uint32_t corLsaSeq;
};


}//namespace nlsr
#endif
