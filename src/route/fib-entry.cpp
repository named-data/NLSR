#include <list>
#include "fib-entry.hpp"
#include "nexthop.hpp"

namespace nlsr {

using namespace std;

bool
FibEntry::isEqualNextHops(NexthopList& nhlOther)
{
  if (m_nexthopList.getSize() != nhlOther.getSize())
  {
    return false;
  }
  else
  {
    uint32_t nhCount = 0;
    std::list<NextHop>::iterator it1, it2;
    for (it1 = m_nexthopList.getNextHops().begin(),
         it2 = nhlOther.getNextHops().begin() ;
         it1 != m_nexthopList.getNextHops().end() ; it1++, it2++)
    {
      if (it1->getConnectingFace() == it2->getConnectingFace())
      {
        it1->setRouteCost(it2->getRouteCost());
        nhCount++;
      }
      else
      {
        break;
      }
    }
    return nhCount == m_nexthopList.getSize();
  }
}

ostream&
operator<<(ostream& os, FibEntry fe)
{
  os << "Name Prefix: " << fe.getName() << endl;
  os << "Time to Refresh: " << fe.getTimeToRefresh() << endl;
  os << fe.getNexthopList() << endl;
  return os;
}

}//namespace nlsr
