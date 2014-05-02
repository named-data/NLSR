#include <list>
#include <utility>
#include "name-prefix-table-entry.hpp"
#include "routing-table-entry.hpp"
#include "nexthop.hpp"

namespace nlsr {

using namespace std;

void
NamePrefixTableEntry::generateNhlfromRteList()
{
  m_nexthopList.reset();
  for (std::list<RoutingTableEntry>::iterator it = m_rteList.begin();
       it != m_rteList.end(); ++it)
  {
    for (std::list<NextHop>::iterator nhit =
           (*it).getNexthopList().getNextHops().begin();
         nhit != (*it).getNexthopList().getNextHops().end(); ++nhit)
    {
      m_nexthopList.addNextHop((*nhit));
    }
  }
}



static bool
rteCompare(RoutingTableEntry& rte, string& destRouter)
{
  return rte.getDestination() == destRouter;
}

void
NamePrefixTableEntry::removeRoutingTableEntry(RoutingTableEntry& rte)
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
NamePrefixTableEntry::addRoutingTableEntry(RoutingTableEntry& rte)
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
    (*it).getNexthopList().reset(); // reseting existing routing table's next hop
    for (std::list<NextHop>::iterator nhit = rte.getNexthopList().getNextHops().begin();
         nhit != rte.getNexthopList().getNextHops().end(); ++nhit)
    {
      (*it).getNexthopList().addNextHop((*nhit));
    }
  }
}

//debugging purpose
ostream&
operator<<(ostream& os, NamePrefixTableEntry& npte)
{
  os << "Name: " << npte.getNamePrefix() << endl;
  std::list<RoutingTableEntry> rteList = npte.getRteList();
  for (std::list<RoutingTableEntry>::iterator it = rteList.begin();
       it != rteList.end(); ++it)
  {
    cout << (*it);
  }
  os << npte.getNexthopList();
  return os;
}

}//namespace nlsr
