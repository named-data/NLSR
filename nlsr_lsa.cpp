#include<string>
#include<iostream>
#include<algorithm>

#include "nlsr_lsa.hpp"

using namespace std;

string 
Lsa::getLsaKey()
{
	string key;
	key=origRouter + "/" + boost::lexical_cast<std::string>(lsType) + "/" 
	    + boost::lexical_cast<std::string>(lsSeqNo);
	return key;
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

