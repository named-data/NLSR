#include <list>
#include "fib-entry.hpp"
#include "nexthop.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("FibEntry");

using namespace std;

bool
FibEntry::isEqualNextHops(NexthopList& nhlOther)
{
  if (m_nexthopList.getSize() != nhlOther.getSize()) {
    return false;
  }
  else {
    uint32_t nhCount = 0;
    std::list<NextHop>::iterator it1, it2;
    for (it1 = m_nexthopList.getNextHops().begin(),
         it2 = nhlOther.getNextHops().begin() ;
         it1 != m_nexthopList.getNextHops().end() ; it1++, it2++) {
      if (it1->getConnectingFaceUri() == it2->getConnectingFaceUri()) {
        it1->setRouteCost(it2->getRouteCost());
        nhCount++;
      }
      else {
        break;
      }
    }
    return nhCount == m_nexthopList.getSize();
  }
}

void
FibEntry::writeLog()
{
  _LOG_DEBUG("Name Prefix: " << m_name);
  _LOG_DEBUG("Time to Refresh: " << m_expirationTimePoint);
  _LOG_DEBUG("Seq No: " << m_seqNo);
  m_nexthopList.writeLog();
}

ostream&
operator<<(ostream& os, FibEntry fe)
{
  os << "Name Prefix: " << fe.getName() << endl;
  os << "Time to Refresh: " << fe.getExpirationTimePoint() << endl;
  os << fe.getNexthopList() << endl;
  return os;
}

}//namespace nlsr
