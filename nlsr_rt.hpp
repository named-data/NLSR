#ifndef NLSR_RT_HPP
#define NLSR_RT_HPP

#include<iostream>
#include<utility>
#include<string>

#include "nlsr_rte.hpp"

class nlsr;
class NextHop;

using namespace std;

class RoutingTable
{
public:
	RoutingTable()
		: NO_NEXT_HOP(-12345)
	{
	}
	void calculate(nlsr& pnlsr);
	void addNextHop(string destRouter, NextHop& nh);
	void printRoutingTable();

	void addNextHopToDryTable(string destRouter, NextHop& nh);
	void printDryRoutingTable();
	std::pair<RoutingTableEntry&, bool> findRoutingTableEntry(string destRouter);

private:
	void calculateLsRoutingTable(nlsr& pnlsr);
	void calculateHypRoutingTable(nlsr& pnlsr);
	void calculateHypDryRoutingTable(nlsr&pnlsr);
	
	void clearRoutingTable();
	void clearDryRoutingTable();

	const int NO_NEXT_HOP;
	
	std::list< RoutingTableEntry > rTable;
	std::list< RoutingTableEntry > dryTable;
};

#endif
