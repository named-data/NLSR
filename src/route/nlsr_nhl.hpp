#ifndef NLSR_NHL_HPP
#define NLSR_NHL_HPP

#include <ndn-cpp-dev/face.hpp>
#include "nlsr_adjacent.hpp"
#include <list>
#include <iostream>

#include "nlsr_nexthop.hpp"

namespace nlsr
{

  using namespace std;

  class Nhl
  {
  public:
    Nhl()
      : m_nexthopList()
    {
    }

    ~Nhl()
    {
    }
    void addNextHop(NextHop &nh);
    void removeNextHop(NextHop &nh);
    void sort();
    int getSize()
    {
      return m_nexthopList.size();
    }
    void reset()
    {
      if (m_nexthopList.size() > 0 )
      {
        m_nexthopList.clear();
      }
    }
    std::list< NextHop >& getNextHopList()
    {
      return m_nexthopList;
    }

  private:
    std::list< NextHop > m_nexthopList;
  };

  ostream&
  operator<<(ostream& os, Nhl& nhl);

}//namespace nlsr

#endif
