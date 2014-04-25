#ifndef NLSR_RTE_HPP
#define NLSR_RTE_HPP

#include <iostream>

#include "nexthop-list.hpp"

namespace nlsr {

class RoutingTableEntry
{
public:
  RoutingTableEntry()
    : m_destination()
    , m_nhl()
  {
  }

  ~RoutingTableEntry()
  {
  }

  RoutingTableEntry(std::string dest)
    : m_nhl()
  {
    m_destination = dest;
  }

  std::string
  getDestination()
  {
    return m_destination;
  }

  NexthopList&
  getNhl()
  {
    return m_nhl;
  }

private:
  std::string m_destination;
  NexthopList m_nhl;
};

std::ostream&
operator<<(std::ostream& os, RoutingTableEntry& rte);

} //namespace nlsr

#endif //NLSR_RTE_HPP
