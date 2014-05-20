#ifndef NLSR_FIB_ENTRY_HPP
#define NLSR_FIB_ENTRY_HPP

#include <list>
#include <iostream>
#include <boost/cstdint.hpp>

#include <ndn-cxx/util/scheduler.hpp>

#include "nexthop.hpp"
#include "nexthop-list.hpp"

namespace nlsr {

class FibEntry
{
public:
  FibEntry()
    : m_name()
    , m_timeToRefresh(0)
    , m_seqNo(0)
    , m_nexthopList()
  {
  }

  FibEntry(const ndn::Name& name)
    : m_timeToRefresh(0)
    , m_seqNo(0)
    , m_nexthopList()
  {
    m_name = name;
  }

  const ndn::Name&
  getName() const
  {
    return m_name;
  }

  NexthopList&
  getNexthopList()
  {
    return m_nexthopList;
  }

  int32_t
  getTimeToRefresh() const
  {
    return m_timeToRefresh;
  }

  void
  setTimeToRefresh(int32_t ttr)
  {
    m_timeToRefresh = ttr;
  }

  void
  setExpiringEventId(ndn::EventId feid)
  {
    m_expiringEventId = feid;
  }

  ndn::EventId
  getExpiringEventId() const
  {
    return m_expiringEventId;
  }

  void
  setSeqNo(int32_t fsn)
  {
    m_seqNo = fsn;
  }

  int32_t
  getSeqNo()
  {
    return m_seqNo;
  }

  bool
  isEqualNextHops(NexthopList& nhlOther);

  void
  writeLog();

private:
  ndn::Name m_name;
  int32_t m_timeToRefresh;
  ndn::EventId m_expiringEventId;
  int32_t m_seqNo;
  NexthopList m_nexthopList;
};

std::ostream&
operator<<(std::ostream& os, FibEntry fe);

} //namespace nlsr

#endif //NLSR_FIB_ENTRY_HPP
