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
  Fib::updateFib(Nlsr& pnlsr,string name, Nhl& nextHopList)
  {
    std::cout<<"Fib::updateFib Called"<<std::endl;
    int startFace=0;
    int endFace=getNumberOfFacesForName(nextHopList,
                                        pnlsr.getConfParameter().getMaxFacesPerPrefix());
    std::list<FibEntry >::iterator it = std::find_if( fibTable.begin(),
                                        fibTable.end(),
                                        bind(&fibEntryNameCompare, _1, name));
    if( it == fibTable.end() )
    {
      if( nextHopList.getNhlSize() > 0 )
      {
        nextHopList.sortNhl();
        FibEntry newEntry(name);
        std::list<NextHop> nhl=nextHopList.getNextHopList();
        std::list<NextHop>::iterator nhit=nhl.begin();
        for(int i=startFace; i< endFace && nhit!=nhl.end(); ++nhit, i++)
        {
          newEntry.getNhl().addNextHop((*nhit));
          //Add entry to NDN-FIB
        }
        newEntry.getNhl().sortNhl();
        newEntry.setTimeToRefresh(fibEntryRefreshTime);
        newEntry.setFeSeqNo(1);
        newEntry.setFeExpiringEventId(scheduleFibEntryRefreshing(pnlsr,
                                      name ,1,fibEntryRefreshTime));
        fibTable.push_back(newEntry);
      }
    }
    else
    {
      std::cout<<"Old FIB Entry"<<std::endl;
      if( nextHopList.getNhlSize() > 0 )
      {
        nextHopList.sortNhl();
        if ( !it->isEqualNextHops(nextHopList) )
        {
          std::list<NextHop> nhl=nextHopList.getNextHopList();
          std::list<NextHop>::iterator nhit=nhl.begin();
          // Add first Entry to NDN-FIB
          removeFibEntryHop(pnlsr, it->getNhl(),nhit->getConnectingFace());
          it->getNhl().resetNhl();
          it->getNhl().addNextHop((*nhit));
          ++startFace;
          ++nhit;
          for(int i=startFace; i< endFace && nhit!=nhl.end(); ++nhit, i++)
          {
            it->getNhl().addNextHop((*nhit));
            //Add Entry to NDN_FIB
          }
        }
        it->setTimeToRefresh(fibEntryRefreshTime);
        cancelScheduledFeExpiringEvent(pnlsr, it->getFeExpiringEventId());
        it->setFeSeqNo(it->getFeSeqNo()+1);
        (*it).setFeExpiringEventId(scheduleFibEntryRefreshing(pnlsr,
                                   it->getName() ,
                                   it->getFeSeqNo(),fibEntryRefreshTime));
      }
      else
      {
        removeFromFib(pnlsr,name);
      }
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
        //Remove entry from NDN-FIB
      }
    }
    if ( fibTable.size() > 0 )
    {
      fibTable.clear();
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
  Fib::removeFibEntryHop(Nlsr& pnlsr, Nhl& nl, int doNotRemoveHopFaceId)
  {
    for( std::list<NextHop >::iterator it=nl.getNextHopList().begin();
         it != nl.getNextHopList().end();   ++it)
    {
      if ( it->getConnectingFace() != doNotRemoveHopFaceId )
      {
        //Remove FIB Entry from NDN-FIB
      }
    }
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
