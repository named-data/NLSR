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
protected:
	void allocateAdjMatrix();
	void makeAdjMatrix(nlsr& pnlsr,Map pMap);
	void printAdjMatrix();
	int getNumOfLinkfromAdjMatrix(int sRouter);
	void freeAdjMatrix();
	void adjustAdMatrix(int source, int link, double linkCost);
	void getLinksFromAdjMatrix(int *links, double *linkCosts, int source);

protected:
	double ** adjMatrix;
	int numOfRouter;
};

class LinkStateRoutingTableCalculator: public RoutingTableCalculator
{
public:
	LinkStateRoutingTableCalculator(int rn)
		: EMPTY_PARENT(-12345)
		, INF_DISTANCE(2147483647)
		, NO_MAPPING_NUM(-1)
	{
		numOfRouter=rn;
	}
	void setNoLink(int nl)
	{
		vNoLink=nl;
	}

	void calculatePath(Map& pMap, RoutingTable& rt, nlsr& pnlsr);
	

private:
	void doDijkstraPathCalculation(int sourceRouter);
	void sortQueueByDistance(int *Q, double *dist,int start,int element);
	int isNotExplored(int *Q, int u,int start, int element);
	void printAllLsPath(int sourceRouter);
	void printLsPath(int destRouter);

	void allocateParent();
	void allocateDistance();
	void freeParent();
	void freeDistance();

	void allocateLinks();
	void allocateLinkCosts();
	void freeLinks();
	void freeLinksCosts();
	

private:	
	int *parent;
	double *distance;

	int vNoLink;
	int *links;
	double *linkCosts;
	const int EMPTY_PARENT;
	const double INF_DISTANCE;
	const int NO_MAPPING_NUM;

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
