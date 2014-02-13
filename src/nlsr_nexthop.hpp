#ifndef NLSR_NEXTHOP_HPP
#define NLSR_NEXTHOP_HPP

#include<iostream>

using namespace std;

class NextHop
{
public:
	NextHop()
		: connectingFace(0)
		, routeCost(0)
	{
	}

	NextHop(int cf, double rc)
	{
		connectingFace=cf;
		routeCost=rc;
	}

	int getConnectingFace()
	{
		return connectingFace;
	}	

	void setConnectingFace(int cf)
	{
		connectingFace=cf;
	}

	double getRouteCost()
	{
		return routeCost;
	}

	void setRouteCost(double rc)
	{
		routeCost=rc;
	}
private:
	int connectingFace;
	double routeCost;
};


ostream&
operator<<(ostream& os, NextHop& nh);

#endif
