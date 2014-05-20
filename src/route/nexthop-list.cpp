#include <iostream>
#include "nexthop-list.hpp"
#include "nexthop.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("NexthopList");

using namespace std;

static bool
nexthopCompare(NextHop& nh1, NextHop& nh2)
{
  return nh1.getConnectingFaceUri() == nh2.getConnectingFaceUri();
}

static bool
nexthopRemoveCompare(NextHop& nh1, NextHop& nh2)
{
  return (nh1.getConnectingFaceUri() == nh2.getConnectingFaceUri() &&
          nh1.getRouteCost() == nh2.getRouteCost()) ;
}

static bool
nextHopSortingComparator(const NextHop& nh1, const NextHop& nh2)
{
  return nh1.getRouteCost() < nh2.getRouteCost();
}

/**
Add next hop to the Next Hop list
If next hop is new it is added
If next hop already exists in next
hop list then updates the route
cost with new next hop's route cost
*/

void
NexthopList::addNextHop(NextHop& nh)
{
  std::list<NextHop>::iterator it = std::find_if(m_nexthopList.begin(),
                                                 m_nexthopList.end(),
                                                 ndn::bind(&nexthopCompare, _1, nh));
  if (it == m_nexthopList.end()) {
    m_nexthopList.push_back(nh);
  }
  if ((*it).getRouteCost() > nh.getRouteCost()) {
    (*it).setRouteCost(nh.getRouteCost());
  }
}

/**
Remove a next hop only if both next hop face and route cost are same

*/

void
NexthopList::removeNextHop(NextHop& nh)
{
  std::list<NextHop>::iterator it = std::find_if(m_nexthopList.begin(),
                                                 m_nexthopList.end(),
                                                 ndn::bind(&nexthopRemoveCompare, _1, nh));
  if (it != m_nexthopList.end()) {
    m_nexthopList.erase(it);
  }
}

void
NexthopList::sort()
{
  m_nexthopList.sort(nextHopSortingComparator);
}

void
NexthopList::writeLog()
{
  int i = 1;
  for (std::list<NextHop>::iterator it = m_nexthopList.begin();
       it != m_nexthopList.end() ; it++, i++) {
    _LOG_DEBUG("Nexthop " << i << ": " << (*it).getConnectingFaceUri()
               << " Route Cost: " << (*it).getRouteCost());
  }
}

ostream&
operator<<(ostream& os, NexthopList& nhl)
{
  std::list<NextHop> nexthopList = nhl.getNextHops();
  int i = 1;
  for (std::list<NextHop>::iterator it = nexthopList.begin();
       it != nexthopList.end() ; it++, i++) {
    os << "Nexthop " << i << ": " << (*it) << endl;
  }
  return os;
}

}//namespace nlsr
