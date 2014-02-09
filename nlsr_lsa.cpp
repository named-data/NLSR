#include<string>
#include<iostream>
#include<algorithm>
#include<cmath>
#include<limits>

#include "nlsr_lsa.hpp"
#include "nlsr_npl.hpp"
#include "nlsr_adjacent.hpp"
#include "nlsr.hpp"

using namespace std;


string
NameLsa::getNameLsaKey()
{
	string key;
	key=origRouter + "/" + boost::lexical_cast<std::string>(1);
	return key;
}

NameLsa::NameLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt, Npl npl)
{
	origRouter=origR;
	lsType=lst;
	lsSeqNo=lsn;
	lifeTime=lt;

	std::list<string> nl=npl.getNameList();
	for( std::list<string>::iterator it=nl.begin(); it != nl.end(); it++)
	{
		addNameToLsa((*it));
	}

	
}

string
NameLsa::getNameLsaData()
{
	string nameLsaData;
	nameLsaData=origRouter + "|" + boost::lexical_cast<std::string>(lsType) + "|" 
	    + boost::lexical_cast<std::string>(lsSeqNo) + "|" 
	    + boost::lexical_cast<std::string>(lifeTime);
	nameLsaData+="|";
	nameLsaData+=boost::lexical_cast<std::string>(npl.getNplSize());

	std::list<string> nl=npl.getNameList();
	for( std::list<string>::iterator it=nl.begin(); it != nl.end(); it++)
	{
		nameLsaData+="|";
		nameLsaData+=(*it);
	}
	
	return nameLsaData;
}

std::ostream& 
operator<<(std::ostream& os, NameLsa& nLsa) 
{
	os<<"Name Lsa: "<<endl;
	os<<"  Origination Router: "<<nLsa.getOrigRouter()<<endl;
	os<<"  Ls Type: "<<(unsigned short)nLsa.getLsType()<<endl;
	os<<"  Ls Seq No: "<<(unsigned int)nLsa.getLsSeqNo()<<endl;
	os<<"  Ls Lifetime: "<<(unsigned int)nLsa.getLifeTime()<<endl;
	os<<"  Names: "<<endl;
	int i=1;
	std::list<string> nl=nLsa.getNpl().getNameList();
	for( std::list<string>::iterator it=nl.begin(); it != nl.end(); it++)
	{
		os<<"    Name "<<i<<": "<<(*it)<<endl;
	}

	return os;  
}



CorLsa::CorLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt
	      																							, double r, double theta)
{
	origRouter=origR;
	lsType=lst;
	lsSeqNo=lsn;
	lifeTime=lt;
	corRad=r;
	corTheta=theta;
}

string
CorLsa::getCorLsaKey()
{
	string key;
	key=origRouter + "/" + boost::lexical_cast<std::string>(3);
	return key;
}

bool 
CorLsa::isLsaContentEqual(CorLsa& clsa)
{
	return (std::abs(corRad - clsa.getCorRadius()) < 
	                                    std::numeric_limits<double>::epsilon()) &&
	       (std::abs(corTheta - clsa.getCorTheta()) < 
	                                    std::numeric_limits<double>::epsilon());
}

string 
CorLsa::getCorLsaData()
{
	string corLsaData;
	corLsaData=origRouter + "|";
	corLsaData+=(boost::lexical_cast<std::string>(lsType) + "|");
	corLsaData+=(boost::lexical_cast<std::string>(lsSeqNo) + "|");
	corLsaData+=(boost::lexical_cast<std::string>(lifeTime) + "|");
	corLsaData+=(boost::lexical_cast<std::string>(corRad) + "|");
	corLsaData+=(boost::lexical_cast<std::string>(corTheta) + "|");

	return corLsaData;
}

std::ostream& 
operator<<(std::ostream& os, CorLsa& cLsa)
{
	os<<"Cor Lsa: "<<endl;
	os<<"  Origination Router: "<<cLsa.getOrigRouter()<<endl;
	os<<"  Ls Type: "<<(unsigned short)cLsa.getLsType()<<endl;
	os<<"  Ls Seq No: "<<(unsigned int)cLsa.getLsSeqNo()<<endl;
	os<<"  Ls Lifetime: "<<(unsigned int)cLsa.getLifeTime()<<endl;
	os<<"    Hyperbolic Radius: "<<cLsa.getCorRadius()<<endl;
	os<<"    Hyperbolic Theta: "<<cLsa.getCorTheta()<<endl;

	return os;
}


AdjLsa::AdjLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt, 
	                                                        uint32_t nl ,Adl padl)
{
	origRouter=origR;
	lsType=lst;
	lsSeqNo=lsn;
	lifeTime=lt;
	noLink=nl;

	std::list<Adjacent> al=padl.getAdjList();
	for( std::list<Adjacent>::iterator it=al.begin(); it != al.end(); it++)
	{
		if((*it).getStatus()==1)
		{
				addAdjacentToLsa((*it));
		}
	}
}

string
AdjLsa::getAdjLsaKey()
{
	string key;
	key=origRouter + "/" + boost::lexical_cast<std::string>(2);
	return key;
}

bool 
AdjLsa::isLsaContentEqual(AdjLsa& alsa)
{
	return adl.isAdlEqual(alsa.getAdl());
}


string 
AdjLsa::getAdjLsaData(){
	string adjLsaData;
	adjLsaData=origRouter + "|" + boost::lexical_cast<std::string>(lsType) + "|" 
	    + boost::lexical_cast<std::string>(lsSeqNo) + "|" 
	    + boost::lexical_cast<std::string>(lifeTime);
	adjLsaData+="|";
	adjLsaData+=boost::lexical_cast<std::string>(adl.getAdlSize());

	std::list<Adjacent> al=adl.getAdjList();
	for( std::list<Adjacent>::iterator it=al.begin(); it != al.end(); it++)
	{
		adjLsaData+="|";
		adjLsaData+=(*it).getAdjacentName();
		adjLsaData+="|";
		adjLsaData+=boost::lexical_cast<std::string>((*it).getConnectingFace());
		adjLsaData+="|";
		adjLsaData+=boost::lexical_cast<std::string>((*it).getLinkCost());
		
	}
	return adjLsaData;
}


void 
AdjLsa::addNptEntriesForAdjLsa(nlsr& pnlsr)
{
	if ( getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
	{
		pnlsr.getNpt().addNpte(getOrigRouter(), getOrigRouter(),pnlsr);
	}

}


void 
AdjLsa::removeNptEntriesForAdjLsa(nlsr& pnlsr)
{
	if ( getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
	{
		pnlsr.getNpt().removeNpte(getOrigRouter(), getOrigRouter(),pnlsr);
	}
}



std::ostream& 
operator<<(std::ostream& os, AdjLsa& aLsa) 
{
	os<<"Adj Lsa: "<<endl;
	os<<"  Origination Router: "<<aLsa.getOrigRouter()<<endl;
	os<<"  Ls Type: "<<(unsigned short)aLsa.getLsType()<<endl;
	os<<"  Ls Seq No: "<<(unsigned int)aLsa.getLsSeqNo()<<endl;
	os<<"  Ls Lifetime: "<<(unsigned int)aLsa.getLifeTime()<<endl;
	os<<"  No Link: "<<(unsigned int)aLsa.getNoLink()<<endl;
	os<<"  Adjacents: "<<endl;
	int i=1;
	std::list<Adjacent> al=aLsa.getAdl().getAdjList();
	for( std::list<Adjacent>::iterator it=al.begin(); it != al.end(); it++)
	{
		os<<"    Adjacent "<<i<<": "<<endl;
		os<<"      Adjacent Name: "<<(*it).getAdjacentName()<<endl;
		os<<"      Connecting Face: "<<(*it).getConnectingFace()<<endl;
		os<<"      Link Cost: "<<(*it).getLinkCost()<<endl;
	}
	
	return os;  
}
