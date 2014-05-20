#ifndef NLSR_NEXTHOP_LIST_HPP
#define NLSR_NEXTHOP_LIST_HPP

#include <list>
#include <iostream>
#include <boost/cstdint.hpp>

#include <ndn-cxx/face.hpp>

#include "nexthop.hpp"
#include "adjacent.hpp"

namespace nlsr {

class NexthopList
{
public:
  NexthopList()
  {
  }

  ~NexthopList()
  {
  }

  void
  addNextHop(NextHop& nh);

  void
  removeNextHop(NextHop& nh);

  void
  sort();

  size_t
  getSize()
  {
    return m_nexthopList.size();
  }

  void
  reset()
  {
    m_nexthopList.clear();
  }

  std::list<NextHop>&
  getNextHops()
  {
    return m_nexthopList;
  }

  void
  writeLog();

private:
  std::list<NextHop> m_nexthopList;
};

std::ostream&
operator<<(std::ostream& os, NexthopList& nhl);

}//namespace nlsr

#endif //NLSR_NEXTHOP_LIST_HPP
