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
	void scheduleFibRefreshing(nlsr& pnlsr, int refreshTime);
	void cleanFib();
	void setFibEntryRefreshTime(int fert)
	{
		fibEntryRefreshTime=fert;
	}
	
	void printFib();

private:
	void removeFibEntryHop(Nhl& nl, int doNotRemoveHopFaceId);
	int getNumberOfFacesForName(Nhl& nextHopList, int maxFacesPerPrefix);
	void refreshFib(nlsr& pnlsr);
	
private:
	std::list<FibEntry> fibTable;	
	int fibEntryRefreshTime;
};

#endif
