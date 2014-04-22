#include <list>
#include "fib-entry.hpp"
#include "fib.hpp"
#include "nhl.hpp"
#include "nlsr.hpp"


namespace nlsr {

using namespace std;
using namespace ndn;

static bool
fibEntryNameCompare(FibEntry& fe, string name)
{
  return fe.getName() == name ;
}

void
Fib::cancelScheduledExpiringEvent(Nlsr& pnlsr, EventId eid)
{
  pnlsr.getScheduler().cancelEvent(eid);
}


ndn::EventId
Fib::scheduleEntryRefreshing(Nlsr& pnlsr, string name, int feSeqNum,
                             int refreshTime)
{
  return pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(refreshTime),
                                            ndn::bind(&Fib::refreshEntry, this, name, feSeqNum));
}

void
Fib::refreshEntry(string name, int feSeqNum)
{
}

void
Fib::remove(Nlsr& pnlsr, string name)
{
  std::list<FibEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(), bind(&fibEntryNameCompare, _1, name));
  if (it != m_table.end())
  {
    for (std::list<NextHop>::iterator nhit =
           (*it).getNhl().getNextHopList().begin();
         nhit != (*it).getNhl().getNextHopList().begin(); nhit++)
    {
      //remove entry from NDN-FIB
    }
    cancelScheduledExpiringEvent(pnlsr, (*it).getExpiringEventId());
    m_table.erase(it);
  }
}


void
Fib::update(Nlsr& pnlsr, string name, Nhl& nextHopList)
{
  std::cout << "Fib::updateFib Called" << std::endl;
  int startFace = 0;
  int endFace = getNumberOfFacesForName(nextHopList,
                                        pnlsr.getConfParameter().getMaxFacesPerPrefix());
  std::list<FibEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&fibEntryNameCompare, _1, name));
  if (it == m_table.end())
  {
    if (nextHopList.getSize() > 0)
    {
      nextHopList.sort();
      FibEntry newEntry(name);
      std::list<NextHop> nhl = nextHopList.getNextHopList();
      std::list<NextHop>::iterator nhit = nhl.begin();
      for (int i = startFace; i < endFace && nhit != nhl.end(); ++nhit, i++)
      {
        newEntry.getNhl().addNextHop((*nhit));
        //Add entry to NDN-FIB
      }
      newEntry.getNhl().sort();
      newEntry.setTimeToRefresh(m_refreshTime);
      newEntry.setSeqNo(1);
      newEntry.setExpiringEventId(scheduleEntryRefreshing(pnlsr,
                                                          name , 1, m_refreshTime));
      m_table.push_back(newEntry);
    }
  }
  else
  {
    std::cout << "Old FIB Entry" << std::endl;
    if (nextHopList.getSize() > 0)
    {
      nextHopList.sort();
      if (!it->isEqualNextHops(nextHopList))
      {
        std::list<NextHop> nhl = nextHopList.getNextHopList();
        std::list<NextHop>::iterator nhit = nhl.begin();
        // Add first Entry to NDN-FIB
        removeHop(pnlsr, it->getNhl(), nhit->getConnectingFace());
        it->getNhl().reset();
        it->getNhl().addNextHop((*nhit));
        ++startFace;
        ++nhit;
        for (int i = startFace; i < endFace && nhit != nhl.end(); ++nhit, i++)
        {
          it->getNhl().addNextHop((*nhit));
          //Add Entry to NDN_FIB
        }
      }
      it->setTimeToRefresh(m_refreshTime);
      cancelScheduledExpiringEvent(pnlsr, it->getExpiringEventId());
      it->setSeqNo(it->getSeqNo() + 1);
      (*it).setExpiringEventId(scheduleEntryRefreshing(pnlsr,
                                                       it->getName() ,
                                                       it->getSeqNo(), m_refreshTime));
    }
    else
    {
      remove(pnlsr, name);
    }
  }
}



void
Fib::clean(Nlsr& pnlsr)
{
  for (std::list<FibEntry>::iterator it = m_table.begin(); it != m_table.end();
       ++it)
  {
    for (std::list<NextHop>::iterator nhit =
           (*it).getNhl().getNextHopList().begin();
         nhit != (*it).getNhl().getNextHopList().begin(); nhit++)
    {
      cancelScheduledExpiringEvent(pnlsr, (*it).getExpiringEventId());
      //Remove entry from NDN-FIB
    }
  }
  if (m_table.size() > 0)
  {
    m_table.clear();
  }
}

int
Fib::getNumberOfFacesForName(Nhl& nextHopList, int maxFacesPerPrefix)
{
  int endFace = 0;
  if ((maxFacesPerPrefix == 0) || (nextHopList.getSize() <= maxFacesPerPrefix))
  {
    return nextHopList.getSize();
  }
  else
  {
    return maxFacesPerPrefix;
  }
  return endFace;
}

void
Fib::removeHop(Nlsr& pnlsr, Nhl& nl, int doNotRemoveHopFaceId)
{
  for (std::list<NextHop>::iterator it = nl.getNextHopList().begin();
       it != nl.getNextHopList().end();   ++it)
  {
    if (it->getConnectingFace() != doNotRemoveHopFaceId)
    {
      //Remove FIB Entry from NDN-FIB
    }
  }
}

void
Fib::print()
{
  cout << "-------------------FIB-----------------------------" << endl;
  for (std::list<FibEntry>::iterator it = m_table.begin(); it != m_table.end();
       ++it)
  {
    cout << (*it);
  }
}

} //namespace nlsr
