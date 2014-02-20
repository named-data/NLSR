#include <list>
#include "nlsr_fe.hpp"
#include "nlsr_nexthop.hpp"

namespace nlsr
{

    using namespace std;

    bool
    FibEntry::isEqualNextHops(Nhl &nhlOther)
    {
        if ( nhl.getNhlSize() != nhlOther.getNhlSize() )
        {
            return false;
        }
        else
        {
            int nhCount=0;
            std::list<NextHop>::iterator it1, it2;
            for ( it1=nhl.getNextHopList().begin(),
                    it2 = nhlOther.getNextHopList().begin() ;
                    it1 != nhl.getNextHopList().end() ; it1++, it2++)
            {
                if ((*it1).getConnectingFace() == (*it2).getConnectingFace() )
                {
                    (*it1).setRouteCost((*it2).getRouteCost());
                    nhCount++;
                }
                else
                {
                    break;
                }
            }
            return nhCount == nhl.getNhlSize();
        }
    }

    ostream&
    operator<<(ostream& os, FibEntry& fe)
    {
        os<<"Name Prefix: "<<fe.getName()<<endl;
        os<<"Time to Refresh: "<<fe.getTimeToRefresh()<<endl;
        os<<fe.getNhl()<<endl;
        return os;
    }

}//namespace nlsr
