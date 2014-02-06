#include <iostream>

#include "nlsr_nhl.hpp"
#include "nlsr_nexthop.hpp"

using namespace std;

static bool
nexthopCompare(NextHop& nh1, NextHop& nh2){
	return nh1.getConnectingFace()==nh2.getConnectingFace();
}

/** 
Add next hop to the Next Hop list
If next hop is new it is added
If next hop already exists in next
hop list then updates the route
cost with new next hop's route cost
*/

void
Nhl::addNextHop(NextHop& nh)
{
	std::list<NextHop >::iterator it = std::find_if( nexthopList.begin(), 
									nexthopList.end(),	
   								bind(&nexthopCompare, _1, nh));
	if ( it == nexthopList.end() ){
		nexthopList.push_back(nh);
	}
	if ( (*it).getRouteCost() > nh.getRouteCost() )
	{
		(*it).setRouteCost(nh.getRouteCost());
	}
}


ostream&
operator<<(ostream& os, Nhl& nhl)
{
	std::list< NextHop > nexthopList = nhl.getNextHopList();
	int i=1;
	for( std::list<NextHop>::iterator it=nexthopList.begin(); 
	                                            it!= nexthopList.end() ; it++,i++)
	{
		os << "Nexthop "<<i<<": "<<(*it)<<endl;
	}
	return os;
}
