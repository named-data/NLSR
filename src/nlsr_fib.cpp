#include<list>
#include "nlsr_fe.hpp"
#include "nlsr_fib.hpp"
#include "nlsr_nhl.hpp"
#include "nlsr.hpp"

namespace nlsr
{

    using namespace std;
    using namespace ndn;

    static bool
    fibEntryNameCompare(FibEntry& fe, string name)
    {
        return fe.getName() == name ;
    }

    void
    Fib::cancelScheduledFeExpiringEvent(Nlsr& pnlsr, EventId eid)
    {
        pnlsr.getScheduler().cancelEvent(eid);
    }


    ndn::EventId
    Fib::scheduleFibEntryRefreshing(Nlsr& pnlsr, string name, int feSeqNum,
                                    int refreshTime)
    {
        return pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(refreshTime),
                ndn::bind(&Fib::refreshFibEntry,this,name,feSeqNum));
    }

    void
    Fib::refreshFibEntry(string name, int feSeqNum)
    {

    }

    void
    Fib::removeFromFib(Nlsr& pnlsr, string name)
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
            cancelScheduledFeExpiringEvent(pnlsr, (*it).getFeExpiringEventId());
            fibTable.erase(it);
        }
    }


    void
    Fib::updateFib(Nlsr& pnlsr,string name, Nhl& nextHopList, int maxFacesPerPrefix)
    {
        int startFace=0;
        int endFace=getNumberOfFacesForName(nextHopList,maxFacesPerPrefix);
        std::list<FibEntry >::iterator it = std::find_if( fibTable.begin(),
                                            fibTable.end(), bind(&fibEntryNameCompare, _1, name));
        if( it != fibTable.end() )
        {
            nextHopList.sortNhl();
            if ( !(*it).isEqualNextHops(nextHopList) )
            {
                std::list<NextHop>::iterator nhit=nextHopList.getNextHopList().begin();
                (*it).getNhl().addNextHop((*nhit));
                removeFibEntryHop((*it).getNhl(),(*nhit).getConnectingFace());
                startFace++;
                nhit++;
                for( int i=startFace; i< endFace; nhit++,i++)
                {
                    (*it).getNhl().addNextHop((*nhit));
                }

                (*it).setTimeToRefresh(fibEntryRefreshTime);
            }
            (*it).getNhl().sortNhl();
            cancelScheduledFeExpiringEvent(pnlsr, (*it).getFeExpiringEventId());
            (*it).setFeSeqNo((*it).getFeSeqNo()+1);
            (*it).setFeExpiringEventId(scheduleFibEntryRefreshing(pnlsr,
                                       (*it).getName() ,
                                       (*it).getFeSeqNo(),fibEntryRefreshTime));
            //update NDN-FIB
        }
        else
        {
            nextHopList.sortNhl();
            FibEntry newEntry(name);
            std::list<NextHop>::iterator nhit=nextHopList.getNextHopList().begin();
            for(int i=startFace; i< endFace ; i++)
            {
                newEntry.getNhl().addNextHop((*nhit));
                ++nhit;
            }
            newEntry.getNhl().sortNhl();
            newEntry.setTimeToRefresh(fibEntryRefreshTime);
            newEntry.setFeSeqNo(1);
            fibTable.push_back(newEntry);

            //cancelScheduledFeExpiringEvent(pnlsr, newEntry().getFeExpiringEventId());

            //Update NDN-FIB
        }
    }



    void Fib::cleanFib(Nlsr& pnlsr)
    {
        for( std::list<FibEntry >::iterator it=fibTable.begin(); it != fibTable.end();
                ++it)
        {
            for(std::list<NextHop>::iterator nhit=(*it).getNhl().getNextHopList().begin();
                    nhit != (*it).getNhl().getNextHopList().begin(); nhit++)
            {
                cancelScheduledFeExpiringEvent(pnlsr,(*it).getFeExpiringEventId());
                //remove entry from NDN-FIB
            }
        }

        if ( fibTable.size() > 0 )
        {
            fibTable.clear();
        }
    }


    void
    Fib::removeFibEntryHop(Nhl& nl, int doNotRemoveHopFaceId)
    {
        for( std::list<NextHop >::iterator it=nl.getNextHopList().begin();
                it != nl.getNextHopList().end();   ++it)
        {
            if ( (*it).getConnectingFace() != doNotRemoveHopFaceId )
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
            cout<<(*it);
        }
    }

} //namespace nlsr
