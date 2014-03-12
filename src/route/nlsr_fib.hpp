#ifndef NLSR_FIB_HPP
#define NLSR_FIB_HPP

#include <list>
#include "nlsr_fe.hpp"

namespace nlsr
{

  class Nlsr;

  using namespace std;
  using namespace ndn;

  class Fib
  {
  public:
    Fib()
      : fibTable()
      , fibEntryRefreshTime(0)
    {
    }

    void removeFromFib(Nlsr& pnlsr, string name);
    void updateFib(Nlsr& pnlsr, string name, Nhl& nextHopList);
    void cleanFib(Nlsr& pnlsr);
    void setFibEntryRefreshTime(int fert)
    {
      fibEntryRefreshTime=fert;
    }

    void printFib();

  private:
    void removeFibEntryHop(Nlsr& pnlsr, Nhl& nl, int doNotRemoveHopFaceId);
    int getNumberOfFacesForName(Nhl& nextHopList, int maxFacesPerPrefix);
    ndn::EventId
    scheduleFibEntryRefreshing(Nlsr& pnlsr, string name, int feSeqNum,
                               int refreshTime);
    void cancelScheduledFeExpiringEvent(Nlsr& pnlsr, EventId eid);
    void refreshFibEntry(string name, int feSeqNum);

  private:
    std::list<FibEntry> fibTable;
    int fibEntryRefreshTime;
  };

}//namespace nlsr
#endif
