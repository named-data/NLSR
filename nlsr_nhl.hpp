#ifndef NLSR_NHL_HPP
#define NLSR_NHL_HPP

#include <ndn-cpp-dev/face.hpp>
#include "nlsr_adjacent.hpp"
#include <list>
#include <iostream>

#include "nlsr_nexthop.hpp"

using namespace std;

class Nhl
{
public:
	Nhl()
	{
	}

	~Nhl()
	{
	}
	void addNextHop(NextHop &nh);
	void removeNextHop(NextHop &nh);
	void sortNhl();
	int getNhlSize()
	{
		return nexthopList.size();
	}
	void resetNhl()
	{
		if (nexthopList.size() > 0 )
		{
			nexthopList.clear();
		}
	}
	std::list< NextHop >& getNextHopList()
	{
		return nexthopList;
	}

private:
	std::list< NextHop > nexthopList;
};

ostream&
operator<<(ostream& os, Nhl& nhl);

#endif
