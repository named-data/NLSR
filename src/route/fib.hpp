#ifndef NLSR_FIB_HPP
#define NLSR_FIB_HPP

#include <list>
#include "fib-entry.hpp"

namespace nlsr {

class Nlsr;

using namespace std;
using namespace ndn;

class Fib
{
public:
  Fib()
    : m_table()
    , m_refreshTime(0)
  {
  }

  void
  remove(Nlsr& pnlsr, string name);

  void
  update(Nlsr& pnlsr, string name, Nhl& nextHopList);

  void
  clean(Nlsr& pnlsr);

  void
  setEntryRefreshTime(int fert)
  {
    m_refreshTime = fert;
  }

  void
  print();

private:
  void
  removeHop(Nlsr& pnlsr, Nhl& nl, int doNotRemoveHopFaceId);

  int
  getNumberOfFacesForName(Nhl& nextHopList, int maxFacesPerPrefix);

  ndn::EventId
  scheduleEntryRefreshing(Nlsr& pnlsr, string name, int feSeqNum,
                          int refreshTime);
  void
  cancelScheduledExpiringEvent(Nlsr& pnlsr, EventId eid);

  void
  refreshEntry(string name, int feSeqNum);

private:
  std::list<FibEntry> m_table;
  int m_refreshTime;
};

}//namespace nlsr
#endif //NLSR_FIB_HPP
