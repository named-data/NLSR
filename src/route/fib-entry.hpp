#ifndef NLSR_FIB_ENTRY_HPP
#define NLSR_FIB_ENTRY_HPP

#include <list>
#include <iostream>
#include <boost/cstdint.hpp>

#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/time.hpp>

#include "nexthop.hpp"
#include "nexthop-list.hpp"

namespace nlsr {

class FibEntry
{
public:
  FibEntry()
    : m_name()
    , m_expirationTimePoint()
    , m_seqNo(0)
    , m_nexthopList()
  {
  }

  FibEntry(const ndn::Name& name)
    : m_expirationTimePoint()
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

  const ndn::time::system_clock::TimePoint&
  getExpirationTimePoint() const
  {
    return m_expirationTimePoint;
  }

  void
  setExpirationTimePoint(const ndn::time::system_clock::TimePoint& ttr)
  {
    m_expirationTimePoint = ttr;
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
  ndn::time::system_clock::TimePoint m_expirationTimePoint;
  ndn::EventId m_expiringEventId;
  int32_t m_seqNo;
  NexthopList m_nexthopList;
};

std::ostream&
operator<<(std::ostream& os, FibEntry fe);

} //namespace nlsr

#endif //NLSR_FIB_ENTRY_HPP
