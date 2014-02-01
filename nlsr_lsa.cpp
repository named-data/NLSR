#include<string>
#include<iostream>
#include<algorithm>

#include "nlsr_lsa.hpp"
#include "nlsr_npl.hpp"

using namespace std;

string 
Lsa::getLsaKey()
{
	string key;
	key=origRouter + "/" + boost::lexical_cast<std::string>(lsType);
	return key;
}

NameLsa::NameLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt, Npl& npl)
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
