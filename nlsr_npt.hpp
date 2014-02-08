#ifndef NLSR_NPT_HPP
#define NLSR_NPT_HPP

#include <list>
#include "nlsr_npte.hpp"
#include "nlsr_rte.hpp"

using namespace std;

class nlsr;

class Npt
{
public: 
	Npt()
	{
	}
	void addNpte(string name, string destRouter, nlsr& pnlsr);
	void removeNpte(string name, string destRouter, nlsr& pnlsr);
	void updateNptWithNewRoute(nlsr& pnlsr);
	void printNpt();
private:
	void addNpte(string name, RoutingTableEntry& rte, nlsr& pnlsr);
	void removeNpte(string name, RoutingTableEntry& rte, nlsr& pnlsr);
private:
	std::list<Npte> npteList;
};

#endif
