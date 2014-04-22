#include <list>
#include <utility>
#include "npte.hpp"
#include "routing-table-entry.hpp"
#include "nexthop.hpp"

namespace nlsr {

using namespace std;

void
Npte::generateNhlfromRteList()
{
  m_nhl.reset();
  for (std::list<RoutingTableEntry>::iterator it = m_rteList.begin();
       it != m_rteList.end(); ++it)
  {
    for (std::list<NextHop>::iterator nhit =
           (*it).getNhl().getNextHopList().begin();
         nhit != (*it).getNhl().getNextHopList().end(); ++nhit)
    {
      m_nhl.addNextHop((*nhit));
    }
  }
}



static bool
rteCompare(RoutingTableEntry& rte, string& destRouter)
{
  return rte.getDestination() == destRouter;
}

void
Npte::removeRoutingTableEntry(RoutingTableEntry& rte)
{
  std::list<RoutingTableEntry>::iterator it = std::find_if(m_rteList.begin(),
                                                           m_rteList.end(),
                                                           bind(&rteCompare, _1, rte.getDestination()));
  if (it != m_rteList.end())
  {
    m_rteList.erase(it);
  }
}

void
Npte::addRoutingTableEntry(RoutingTableEntry& rte)
{
  std::list<RoutingTableEntry>::iterator it = std::find_if(m_rteList.begin(),
                                                           m_rteList.end(),
                                                           bind(&rteCompare, _1, rte.getDestination()));
  if (it == m_rteList.end())
  {
    m_rteList.push_back(rte);
  }
  else
  {
    (*it).getNhl().reset(); // reseting existing routing table's next hop
    for (std::list<NextHop>::iterator nhit = rte.getNhl().getNextHopList().begin();
         nhit != rte.getNhl().getNextHopList().end(); ++nhit)
    {
      (*it).getNhl().addNextHop((*nhit));
    }
  }
}

//debugging purpose
ostream&
operator<<(ostream& os, Npte& npte)
{
  os << "Name: " << npte.getNamePrefix() << endl;
  std::list<RoutingTableEntry> rteList = npte.getRteList();
  for (std::list<RoutingTableEntry>::iterator it = rteList.begin();
       it != rteList.end(); ++it)
  {
    cout << (*it);
  }
  os << npte.getNhl();
  return os;
}

}//namespace nlsr
