#ifndef ADL_HPP
#define ADL_HPP

#include <ndn-cpp-dev/face.hpp>
#include "nlsr_adjacent.hpp"
#include<list>

using namespace std;

class Adl{

public:
	Adl();
	~Adl();
	int insert(Adjacent& adj);
	int updateAdjacentStatus(string adjName, int s);
	int updateAdjacentLinkCost(string adjName, double lc);	
	void printAdl();
	std::list<Adjacent> getAdjList();


private:	
	std::list< Adjacent > adjList;
};	

#endif
