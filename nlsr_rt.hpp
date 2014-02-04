#ifndef NLSR_RT_HPP
#define NLSR_RT_HPP

#include<iostream>
#include<string>

#include "nlsr_rte.hpp"

using namespace std;

class RoutingTable
{
public:
	RoutingTable()
	{
	}
	void calculate()
	{
		cout<<"Routing Table Calculating......"<<endl;
	}

private:
	std::list< RoutingTableEntry > rTable;
};

#endif
