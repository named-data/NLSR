#ifndef NLSR_FIB_HPP
#define NLSR_FIB_HPP

#include <list>
#include "nlsr_fe.hpp"

class nlsr;

using namespace std;

class Fib
{
public:
	Fib()
	{
	}

	void removeFromFib(string name);
	void updateFib(string name, Nhl& nextHopList, int maxFacesPerPrefix);
	void cleanFib();
	void printFib();

private:
	void removeFibEntryHop(Nhl& nl, int doNotRemoveHop);
	int getNumberOfFacesForName(Nhl& nextHopList, int maxFacesPerPrefix);
	
private:
	std::list<FibEntry> fibTable;	
};

#endif
