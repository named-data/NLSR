#include <iostream>
#include "nlsr_lsdb.hpp"
#include "nlsr_rtc.hpp"
#include "nlsr_map.hpp"
#include "nlsr_lsa.hpp"
#include "nlsr.hpp"

using namespace std;

void 
RoutingTableCalculator::allocateAdjMatrix()
{
	adjMatrix = new double*[numOfRouter];
	for(int i = 0; i < numOfRouter; ++i) 
	{
    adjMatrix[i] = new double[numOfRouter];
	}
}

void
RoutingTableCalculator::makeAdjMatrix(nlsr& pnlsr, Map pMap)
{
	std::list<AdjLsa> adjLsdb=pnlsr.getLsdb().getAdjLsdb();
	for( std::list<AdjLsa>::iterator it=adjLsdb.begin(); 
	                                                 it!= adjLsdb.end() ; it++)
	{
		string linkStartRouter=(*it).getOrigRouter();
		int row=pMap.getMappingNoByRouterName(linkStartRouter);
		std::list<Adjacent> adl=(*it).getAdl().getAdjList();
		for( std::list<Adjacent>::iterator itAdl=adl.begin(); 
																								itAdl!= adl.end() ; itAdl++)
		{
			string linkEndRouter=(*itAdl).getAdjacentName();
			int col=pMap.getMappingNoByRouterName(linkEndRouter);
			double cost=(*itAdl).getLinkCost();
			if ( (row >= 0 && row<numOfRouter) && (col >= 0 && col<numOfRouter) )
			{
				adjMatrix[row][col]=cost;
			}
		}
		
	}
}

void 
RoutingTableCalculator::freeAdjMatrix()
{
	for(int i = 0; i < numOfRouter; ++i) 
	{
    	delete [] adjMatrix[i];
	}
	delete [] adjMatrix;
}

void
LinkStateRoutingTableCalculator::calculatePath(Map& pMap, RoutingTable& rt, nlsr& pnlsr)
{
}

void
HypRoutingTableCalculator::calculatePath(Map& pMap, RoutingTable& rt, nlsr& pnlsr)
{
}

void
HypDryRoutingTableCalculator::calculatePath(Map& pMap, RoutingTable& rt, nlsr& pnlsr)
{
}

