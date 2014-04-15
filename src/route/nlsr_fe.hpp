#ifndef NLSR_FE_HPP
#define NLSR_FE_HPP

#include<list>
#include <iostream>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "nlsr_nexthop.hpp"
#include "nlsr_nhl.hpp"

namespace nlsr
{

  using namespace std;

  class FibEntry
  {
  public:
    FibEntry()
      : m_name()
      , m_timeToRefresh(0)
      , m_seqNo(0)
      , m_nhl()
    {
    }

    FibEntry(string n)
      : m_timeToRefresh(0)
      , m_seqNo(0)
      , m_nhl()
    {
      m_name=n;
    }

    string getName()
    {
      return m_name;
    }

    Nhl& getNhl()
    {
      return m_nhl;
    }

    int getTimeToRefresh()
    {
      return m_timeToRefresh;
    }

    void setTimeToRefresh(int ttr)
    {
      m_timeToRefresh=ttr;
    }

    void setExpiringEventId(ndn::EventId feid)
    {
      m_expiringEventId=feid;
    }

    ndn::EventId getExpiringEventId()
    {
      return m_expiringEventId;
    }

    void setSeqNo(int fsn)
    {
      m_seqNo=fsn;
    }

    int getSeqNo()
    {
      return m_seqNo;
    }

    bool isEqualNextHops(Nhl& nhlOther);

  private:
    string m_name;
    int m_timeToRefresh;
    ndn::EventId m_expiringEventId;
    int m_seqNo;
    Nhl m_nhl;
  };

  ostream& operator<<(ostream& os,FibEntry fe);

} //namespace nlsr

#endif
