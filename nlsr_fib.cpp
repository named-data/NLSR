#include<list>
#include "nlsr_fe.hpp"
#include "nlsr_fib.hpp"
#include "nlsr_nhl.hpp"

using namespace std;

static bool
fibEntryNameCompare(FibEntry& fe, string name)
{
	return fe.getName() == name ;
}



void 
Fib::removeFromFib(string name)
{
	std::list<FibEntry >::iterator it = std::find_if( fibTable.begin(), 
									       fibTable.end(), bind(&fibEntryNameCompare, _1, name));
  if( it != fibTable.end() )
  {
  		for(std::list<NextHop>::iterator nhit=(*it).getNhl().getNextHopList().begin(); 
  		                    nhit != (*it).getNhl().getNextHopList().begin(); nhit++)
  		{
  			//remove entry from NDN-FIB
  		}
  		fibTable.erase(it);
  }
}

/**
If NHL is equal for current FIB and NPT then to change
Otherwise
 Add the first Nexthop to FIB
 remove all old nexthop from FIB
 And add all other Nexthop to FIB
*/

void 
Fib::updateFib(string name, Nhl& nextHopList, int maxFacesPerPrefix)
{
	int startFace=0;
	int endFace=getNumberOfFacesForName(nextHopList,maxFacesPerPrefix);
	std::list<FibEntry >::iterator it = std::find_if( fibTable.begin(), 
									       fibTable.end(), bind(&fibEntryNameCompare, _1, name));
  if( it != fibTable.end() )
  {
  		if ( !(*it).isEqualNextHops(nextHopList) ) 
  		{
  			std::list<NextHop>::iterator nhit=nextHopList.getNextHopList().begin();
  			(*it).getNhl().addNextHop((*nhit));
			removeFibEntryHop((*it).getNhl(),(*nhit).getConnectingFace());
			startFace++;
  			nhit++;
  			for( int i=startFace;i< endFace;nhit++,i++)
  			{
  				(*it).getNhl().addNextHop((*nhit));
  			}
  		}
  		(*it).getNhl().sortNhl();
  }
  else
  {
  		FibEntry newEntry(name);
  		for(std::list<NextHop>::iterator nhit=nextHopList.getNextHopList().begin();
  															nhit!=nextHopList.getNextHopList().end();++nhit)
  		{
  			newEntry.getNhl().addNextHop((*nhit));
  		}
  		newEntry.getNhl().sortNhl();
  		fibTable.push_back(newEntry);	
  }
}

void Fib::cleanFib()
{
	for( std::list<FibEntry >::iterator it=fibTable.begin(); it != fibTable.end();
	                                                                         ++it)
	{
		for(std::list<NextHop>::iterator nhit=(*it).getNhl().getNextHopList().begin(); 
  		                    nhit != (*it).getNhl().getNextHopList().begin(); nhit++)
  		{
  			//remove entry from NDN-FIB
  		}
	}

	if ( fibTable.size() > 0 )
	{
		fibTable.clear();
	}
}


void 
Fib::removeFibEntryHop(Nhl& nl, int doNotRemoveHop)
{
	for( std::list<NextHop >::iterator it=nl.getNextHopList().begin(); 
	                                      it != nl.getNextHopList().end();   ++it)
	{
		if ( (*it).getConnectingFace() != doNotRemoveHop )
		{
			nl.getNextHopList().erase(it);
		}
	}
}


int 
Fib::getNumberOfFacesForName(Nhl& nextHopList, int maxFacesPerPrefix)
{
	int endFace=0;
  	if((maxFacesPerPrefix == 0) || (nextHopList.getNhlSize() <= maxFacesPerPrefix))
  	{
  		return nextHopList.getNhlSize();
  	}
  	else
  	{
  		return maxFacesPerPrefix;
  	}

  	return endFace;
}

void
Fib::printFib()
{
	cout<<"-------------------FIB-----------------------------"<<endl;
	for(std::list<FibEntry>::iterator it = fibTable.begin(); it!=fibTable.end();
	                                                                         ++it)
	{
		//(*it).getNhl().sortNhl();
		cout<<(*it);
	}
}
