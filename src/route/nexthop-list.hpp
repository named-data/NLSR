#ifndef NLSR_NHL_HPP
#define NLSR_NHL_HPP

#include <ndn-cxx/face.hpp>
#include <list>
#include <iostream>

#include "nexthop.hpp"
#include "adjacent.hpp"

namespace nlsr {

class NexthopList
{
public:
  NexthopList()
    : m_nexthopList()
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

  int
  getSize()
  {
    return m_nexthopList.size();
  }

  void
  reset()
  {
    if (m_nexthopList.size() > 0)
    {
      m_nexthopList.clear();
    }
  }

  std::list<NextHop>&
  getNextHopList()
  {
    return m_nexthopList;
  }

private:
  std::list<NextHop> m_nexthopList;
};

std::ostream&
operator<<(std::ostream& os, NexthopList& nhl);

}//namespace nlsr

#endif //NLSR_NLH_HPP
