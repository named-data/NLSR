#include <iostream>
#include <string>

#include "nlsr_rte.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_rte.cpp"

namespace nlsr
{

  using namespace std;

  ostream&
  operator<<(ostream& os, RoutingTableEntry& rte)
  {
    os<<"Destination: "<<rte.getDestination()<<endl;
    os<<"Nexthops: "<<endl;
    int i=1;
    std::list< NextHop > nhl = rte.getNhl().getNextHopList();
    for( std::list<NextHop>::iterator it=nhl.begin();
         it!= nhl.end() ; it++,i++)
    {
      os <<"  Nexthop "<<i<<": "<<(*it)<<endl;
    }
    return os;
  }

}//namespace nlsr
