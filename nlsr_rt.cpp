#include<iostream>
#include<string>
#include<list>

#include "nlsr_rt.hpp"
#include "nlsr.hpp"
#include "nlsr_map.hpp"
#include "nlsr_rtc.hpp"

using namespace std;

void
RoutingTable::calculate(nlsr& pnlsr)
{

	if ( 	pnlsr.getIsRoutingTableCalculating() == 0 )
	{
		pnlsr.setIsRoutingTableCalculating(1); //setting routing table calculation
		
		if ( pnlsr.getLsdb().doesLsaExist(
						pnlsr.getConfParameter().getRouterPrefix()+"/"+"2",2) )
		{
			if(pnlsr.getIsBuildAdjLsaSheduled() != 1)
			{
				cout<<"CLearing old routing table ....."<<endl;
				clearRoutingTable();
				clearDryRoutingTable(); // for dry run options
				// calculate Link State routing
				if( (pnlsr.getConfParameter().getIsHyperbolicCalc() == 0 ) 
						|| (pnlsr.getConfParameter().getIsHyperbolicCalc() == 2 ) )
				{
					calculateLsRoutingTable(pnlsr);
				}
				//calculate hyperbolic routing
				if ( pnlsr.getConfParameter().getIsHyperbolicCalc() == 1 )
				{
					calculateHypRoutingTable(pnlsr);
				}
				//calculate dry hyperbolic routing
				if ( pnlsr.getConfParameter().getIsHyperbolicCalc() == 2 )
				{
					calculateHypDryRoutingTable(pnlsr);
				}

				//need to update NPT here
			}
			else
			{
				cout<<"Adjacency building is scheduled, so ";
				cout<<"routing table can not be calculated :("<<endl;
			}
		}
		else
		{
			cout<<"No Adj LSA of router itself,";
			cout<<	" so Routing table can not be calculated :("<<endl;	
			clearRoutingTable();
		  clearDryRoutingTable(); // for dry run options
		  // need to update NPT here
		}

		pnlsr.setIsRouteCalculationScheduled(0); //clear scheduled flag
		pnlsr.setIsRoutingTableCalculating(0); //unsetting routing table calculation
	}
	else
	{
		pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(15),
									 ndn::bind(&RoutingTable::calculate,this,boost::ref(pnlsr)));
		pnlsr.setIsRouteCalculationScheduled(1);
	}
}


void 
RoutingTable::calculateLsRoutingTable(nlsr& pnlsr)
{
	cout<<"RoutingTable::calculateLsRoutingTable Called"<<endl;
	Map vMap;
	vMap.createMapFromAdjLsdb(pnlsr);
	int numOfRouter=vMap.getMapSize();

	LinkStateRoutingTableCalculator lsrtc(numOfRouter);
	lsrtc.calculatePath(vMap,boost::ref(*this),pnlsr);
	
	
}

void 
RoutingTable::calculateHypRoutingTable(nlsr& pnlsr)
{
	
}

void 
RoutingTable::calculateHypDryRoutingTable(nlsr&pnlsr)
{
	
}

void 
RoutingTable::clearRoutingTable()
{
	if( rTable.size() > 0 )
	{
		rTable.clear();
	}
}

void 
RoutingTable::clearDryRoutingTable()
{
	if (dryTable.size()>0 )
	{
		dryTable.clear();
	}
}

