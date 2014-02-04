#ifndef NLSR_RTE_HPP
#define NLSR_RTE_HPP

#include<iostream>

#include "nlsr_nhl.hpp"

using namespace std;

class RoutingTableEntry
{
public:
	RoutingTableEntry()
		: destination()
		, nhl()
	{
		
	}

	~RoutingTableEntry()
	{
	}

	RoutingTableEntry(string dest)
		: nhl()
	{
		destination=dest;
	}

	string getDestination()
	{
		return destination;
	}

	Nhl& getNhl()
	{
		return nhl;
	}
	
private:
	string destination;
	Nhl nhl;
};

ostream&
operator<<(ostream& os, RoutingTableEntry &rte);

#endif
