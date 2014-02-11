#ifndef NLSR_FE_HPP
#define NLSR_FE_HPP

#include<list>
#include <iostream>

#include "nlsr_nexthop.hpp"
#include "nlsr_nhl.hpp"

using namespace std;

class FibEntry
{
public:
	FibEntry()
		: name()
		, timeToRefresh(0)
	{
	}

	FibEntry(string n)
	{
		name=n;
	}	

	string getName()
	{
		return name;
	}

	Nhl& getNhl()
	{
		return nhl;
	}

	int getTimeToRefresh()
	{
		return timeToRefresh;
	}

	void setTimeToRefresh(int ttr)
	{
		timeToRefresh=ttr;
	}

	bool isEqualNextHops(Nhl &nhlOther);
	
private:
	string name;
	int timeToRefresh;
	Nhl nhl;
};

ostream& operator<<(ostream& os, FibEntry& fe);

#endif
