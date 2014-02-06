#ifndef NLSR_LSA_HPP
#define NLSR_LSA_HPP

#include "nlsr_adjacent.hpp"
#include "nlsr_npl.hpp"
#include "nlsr_adl.hpp"

using namespace std;

class Lsa{
public:
	Lsa()
		: origRouter()
		, lsSeqNo()
		, lifeTime()
	{
	}	


	void setLsType(uint8_t lst)
	{
		lsType=lst;
	}

	uint8_t getLsType()
	{
		return lsType;
	}

	void setLsSeqNo(uint32_t lsn)
	{
		lsSeqNo=lsn;
	}

	uint32_t getLsSeqNo()
	{
		return lsSeqNo;
	}

	string& getOrigRouter()
	{
		return origRouter;
	}

	void setOrigRouter(string& org)
	{
		origRouter=org;
	}

	uint32_t getLifeTime()
	{
		return lifeTime;
	}

	void setLifeTime(uint32_t lt)
	{
		lifeTime=lt;
	}
	//string getLsaKey();
protected:
	string origRouter;
	uint8_t lsType;
	uint32_t lsSeqNo;
	uint32_t lifeTime;
};

class NameLsa:public Lsa{
public:
	NameLsa()
		: Lsa()
		, npl()
	{
		setLsType(1);
	}

	NameLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt, Npl npl);

	Npl& getNpl(){
		return npl;
	}

	void addNameToLsa(string& name)
	{
		npl.insertIntoNpl(name);
	}

	string getNameLsaKey();

	string getNameLsaData();
	
private:
	Npl npl;
	
};

std::ostream& 
operator<<(std::ostream& os, NameLsa& nLsa);

class AdjLsa: public Lsa{
public:
	AdjLsa()
		: Lsa()
		, adl()
	{
		setLsType(2);
	}

	AdjLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt, 
	                                                        uint32_t nl ,Adl padl);
	Adl& getAdl(){
		return adl;
	}

	void addAdjacentToLsa(Adjacent adj)
	{
		adl.insert(adj);
	}
	string getAdjLsaKey();
	string getAdjLsaData();
	uint32_t getNoLink()
	{
		return noLink;
	}

private:
	uint32_t noLink;
	Adl adl;
};

std::ostream& 
operator<<(std::ostream& os, AdjLsa& aLsa);

class CorLsa:public Lsa{
public:
	CorLsa()
		:Lsa()
	{
		setLsType(3);
	}

	CorLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt
	      																							, double r, double theta);
	string getCorLsaKey();
	string getCorLsaData();
	
	double getCorRadius()
	{
		if ( corRad >= 0 )
		{	
			return corRad;
		}
		else 
		{
			return -1;
		}
	}
	
	void setCorRadius(double cr)
	{
		corRad=cr;
	}

	double getCorTheta()
	{
		return corTheta;
	}

	void setCorTheta(double ct){
		corTheta=ct;
	}
private:
	double corRad;
	double corTheta;

};

std::ostream& 
operator<<(std::ostream& os, CorLsa& cLsa);




#endif
