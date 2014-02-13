#ifndef NLSR_NPTE_HPP
#define NLSR_NPTE_HPP

#include <list>
#include <utility>
#include "nlsr_rte.hpp"

using namespace std;

class Npte
{
public:
	Npte()
		: namePrefix()
		, nhl()
	{
	}
	Npte(string np)
		: nhl()
	{
		namePrefix=np;
	}

	string getNamePrefix()
	{
		return namePrefix;
	}

	std::list<RoutingTableEntry>& getRteList()
	{
		return rteList;
	}

	int getRteListSize()
	{
		return rteList.size();
	}

	Nhl& getNhl()
	{
		return nhl;
	}
	void generateNhlfromRteList();
	void removeRoutingTableEntry(RoutingTableEntry& rte);
	void addRoutingTableEntry(RoutingTableEntry &rte);

private:
	string namePrefix;
	std::list<RoutingTableEntry> rteList;
	Nhl nhl;
};

ostream&
operator<<(ostream& os, Npte& npte);

#endif
