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
	bool buildAndInstallOwnNameLsa(nlsr& pnlsr);
	NameLsa& getNameLsa(string key);
	bool installNameLsa(NameLsa &nlsa);
	bool removeNameLsa(string& key);
	void printNameLsdb(); //debugging

	//function related to Cor LSDB
	bool buildAndInstallOwnCorLsa(nlsr& pnlsr);
	CorLsa& getCorLsa(string key);
	bool installCorLsa(CorLsa &clsa);
	bool removeCorLsa(string& key);
	void printCorLsdb(); //debugging

	//function related to Adj LSDB
	void scheduledAdjLsaBuild(nlsr& pnlsr);
	bool buildAndInstallOwnAdjLsa(nlsr& pnlsr);
	bool removeAdjLsa(string& key);
	bool installAdjLsa(nlsr& pnlsr, AdjLsa &alsa);
	AdjLsa& getAdjLsa(string key);
	std::list<AdjLsa>& getAdjLsdb();
	void printAdjLsdb();
	
private:
	bool addNameLsa(NameLsa &nlsa);
	bool doesNameLsaExist(string key);
	
	bool addCorLsa(CorLsa& clsa);
	bool doesCorLsaExist(string key);

	bool addAdjLsa(AdjLsa &alsa);
	bool doesAdjLsaExist(string key);
	

private:
	std::list<NameLsa> nameLsdb;
	std::list<AdjLsa> adjLsdb;
	std::list<CorLsa> corLsdb;

};

#endif
