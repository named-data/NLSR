#include<string>
#include "nlsr_lsdb.hpp"
#include "nlsr.hpp"

using namespace std;





bool 
Lsdb::doesLsaExist(string key, int lsType)
{
	if ( lsType == 1)
	{
		return doesNameLsaExist(key);
	}
	else if ( lsType == 2)
	{
		return doesAdjLsaExist(key);
	}
	else if ( lsType == 3)
	{
		return doesCorLsaExist(key);
	}
	
	return false;
	
}

static bool
nameLsaCompare(NameLsa& nlsa1, NameLsa& nlsa2){
	return nlsa1.getLsaKey()==nlsa1.getLsaKey();
}

static bool
nameLsaCompareByKey(NameLsa& nlsa1, string& key){
	return nlsa1.getLsaKey()==key;
}


bool
Lsdb::buildAndInstallOwnNameLsa(nlsr& pnlsr)
{
	NameLsa nameLsa(pnlsr.getConfParameter().getRouterPrefix()
					, 1
					, pnlsr.getNameLsaSeq()+1
					, pnlsr.getConfParameter().getRouterDeadInterval()
					, pnlsr.getNpl() );
	pnlsr.setNameLsaSeq(pnlsr.getNameLsaSeq()+1);
	cout<<nameLsa;
	return installNameLsa(nameLsa);

}

NameLsa& 
Lsdb::getNameLsa(string key)
{
	std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(), 
																		nameLsdb.end(),	
   																	bind(nameLsaCompareByKey, _1, key));

	if( it != nameLsdb.end()){
		return (*it);
	}
}



bool 
Lsdb::installNameLsa(NameLsa &nlsa)
{
	bool doesLsaExist_ = doesNameLsaExist(nlsa.getLsaKey());
	if ( !doesLsaExist_ )
	{
		// add name LSA
		addNameLsa(nlsa);
		// update NPT and FIB
	}
	else
	{
		// check for newer name LSA
		NameLsa oldNameLsa=getNameLsa(nlsa.getLsaKey());
		// Discard or Update Name lsa, NPT, FIB
	}
	
	return true;
}

bool 
Lsdb::addNameLsa(NameLsa &nlsa)
{
	std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(), 
																		nameLsdb.end(),	
   																	bind(nameLsaCompare, _1, nlsa));

	if( it == nameLsdb.end()){
		nameLsdb.push_back(nlsa);
		return true;
	}
	return false;
}

bool 
Lsdb::removeNameLsa(string& key)
{
	return false;
}
	
void 
Lsdb::printNameLsdb()
{
	for( std::list<NameLsa>::iterator it=nameLsdb.begin(); 
	                                                 it!= nameLsdb.end() ; it++)
	{
		cout<< (*it) <<endl;
	}
}

bool 
Lsdb::doesNameLsaExist(string key)
{
	std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(), 
																		nameLsdb.end(),	
   																	bind(nameLsaCompareByKey, _1, key));

	if( it == nameLsdb.end()){
		return false;
	}

	return true;
}


bool 
Lsdb::doesAdjLsaExist(string key)
{
	return false;
}

bool 
Lsdb::doesCorLsaExist(string key)
{
	return false;
}
