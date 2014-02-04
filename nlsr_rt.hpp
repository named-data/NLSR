#ifndef NLSR_RT_HPP
#define NLSR_RT_HPP

#include<iostream>
#include<string>

#include "nlsr_rte.hpp"

class nlsr;

using namespace std;

class RoutingTable
{
public:
	RoutingTable()
	{
	}
	void calculate(nlsr& pnlsr);

private:
	void calculateLsRoutingTable(nlsr& pnlsr);
	void calculateHypRoutingTable(nlsr& pnlsr);
	void calculateHypDryRoutingTable(nlsr&pnlsr);
	
	void clearRoutingTable();
	void clearDryRoutingTable();
	
	std::list< RoutingTableEntry > rTable;
	std::list< RoutingTableEntry > dryTable;
};

#endif
