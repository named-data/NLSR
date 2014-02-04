#ifndef NLSR_ADL_HPP
#define NLSR_ADL_HPP

#include <ndn-cpp-dev/face.hpp>
#include "nlsr_adjacent.hpp"
#include<list>

class nlsr;

using namespace std;

class Adl{

public:
	Adl();
	~Adl();
	int insert(Adjacent& adj);
	int updateAdjacentStatus(string adjName, int s);
	int updateAdjacentLinkCost(string adjName, double lc);	
	std::list<Adjacent>& getAdjList();
	bool isNeighbor(string adjName);
	void incrementTimedOutInterestCount(string& neighbor);
	int getTimedOutInterestCount(string& neighbor);
	int getStatusOfNeighbor(string& neighbor);
	void setStatusOfNeighbor(string& neighbor, int status);
	void setTimedOutInterestCount(string& neighbor, int count);

	bool isAdjLsaBuildable(nlsr& pnlsr);
	int getNumOfActiveNeighbor();

	int getAdlSize()
	{
		return adjList.size();
	}

	void printAdl();

private:	
	std::list< Adjacent > adjList;
};	

#endif
