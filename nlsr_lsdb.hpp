#ifndef NLSR_LSDB_HPP
#define NLSR_LSDB_HPP

#include "nlsr_lsa.hpp"

using namespace std;

class nlsr;

class Lsdb{
public:
	Lsdb()
	{
	}

	
	bool doesLsaExist(string key, int lsType);
	// function related to Name LSDB 
	bool buildAndInstallOwnNameLsa(nlsr& nlsr);
	NameLsa& getNameLsa(string key);
	bool installNameLsa(NameLsa &nlsa);
	bool removeNameLsa(string& key);
	void printNameLsdb(); //debugging

	//function related to Cor LSDB
	bool buildAndInstallOwnCorLsa(nlsr& nlsr);
	CorLsa& getCorLsa(string key);
	bool installCorLsa(CorLsa &nlsa);
	bool removeCorLsa(string& key);
	void printCorLsdb(); //debugging
	
private:
	bool addNameLsa(NameLsa &nlsa);
	bool doesNameLsaExist(string key);
	bool doesAdjLsaExist(string key);
	bool doesCorLsaExist(string key);

private:
	std::list<NameLsa> nameLsdb;
	std::list<AdjLsa> adjLsdb;
	std::list<CorLsa> corLsdb;

};

#endif
