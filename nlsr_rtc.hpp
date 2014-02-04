#ifndef NLSR_RTC_HPP
#define NLSR_RTC_HPP

#include <list>
#include <iostream>



class Map;
class RoutingTable;
class nlsr;


using namespace std;

class RoutingTableCalculator
{
public:
	RoutingTableCalculator()
	{
	}
	RoutingTableCalculator(int rn)
	{
		numOfRouter=rn;
	}

	void allocateAdjMatrix();
	void makeAdjMatrix(nlsr& pnlsr,Map pMap);
	void freeAdjMatrix();

protected:
	double ** adjMatrix;
	int numOfRouter;
};

class LinkStateRoutingTableCalculator: public RoutingTableCalculator
{
public:
	LinkStateRoutingTableCalculator(int rn)
	{
		numOfRouter=rn;
	}

	void calculatePath(Map& pMap, RoutingTable& rt, nlsr& pnlsr);

};

class HypRoutingTableCalculator: public RoutingTableCalculator
{
public:
	HypRoutingTableCalculator(int rn)
	{
		numOfRouter=rn;
	}

	void calculatePath(Map& pMap, RoutingTable& rt, nlsr& pnlsr);

};

class HypDryRoutingTableCalculator: public RoutingTableCalculator
{
	public:
	HypDryRoutingTableCalculator(int rn)
	{
		numOfRouter=rn;
	}

	void calculatePath(Map& pMap, RoutingTable& rt, nlsr& pnlsr);

};

#endif
