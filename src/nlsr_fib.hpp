#ifndef NLSR_FIB_HPP
#define NLSR_FIB_HPP

#include <list>
#include "nlsr_fe.hpp"

class nlsr;

using namespace std;
using namespace ndn;

class Fib
{
public:
	Fib()
	{
	}

	void removeFromFib(nlsr& pnlsr, string name);
	void updateFib(nlsr& pnlsr, string name, Nhl& nextHopList, int maxFacesPerPrefix);
	void cleanFib(nlsr& pnlsr);
	void setFibEntryRefreshTime(int fert)
	{
		fibEntryRefreshTime=fert;
	}
	
	void printFib();

private:
	void removeFibEntryHop(Nhl& nl, int doNotRemoveHopFaceId);
	int getNumberOfFacesForName(Nhl& nextHopList, int maxFacesPerPrefix);
	ndn::EventId 
	scheduleFibEntryRefreshing(nlsr& pnlsr, string name, int feSeqNum, int refreshTime);
	void cancelScheduledFeExpiringEvent(nlsr& pnlsr, EventId eid);
	void refreshFibEntry(string name, int feSeqNum);
	
private:
	std::list<FibEntry> fibTable;	
	int fibEntryRefreshTime;
};

#endif
