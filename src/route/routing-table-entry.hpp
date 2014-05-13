#ifndef NLSR_ROUTING_TABLE_ENTRY_HPP
#define NLSR_ROUTING_TABLE_ENTRY_HPP

#include <iostream>
#include <ndn-cxx/name.hpp>
#include "nexthop-list.hpp"

namespace nlsr {

class RoutingTableEntry
{
public:
  RoutingTableEntry()
  {
  }

  ~RoutingTableEntry()
  {
  }

  RoutingTableEntry(const ndn::Name& dest)
  {
    m_destination = dest;
  }

  const ndn::Name&
  getDestination() const
  {
    return m_destination;
  }

  NexthopList&
  getNexthopList()
  {
    return m_nexthopList;
  }

private:
  ndn::Name m_destination;
  NexthopList m_nexthopList;
};

inline std::ostream&
operator<<(std::ostream& os, RoutingTableEntry& rte)
{
  os << "Destination: " << rte.getDestination() << std::endl;
  os << "Nexthops: " << std::endl;
  int32_t i = 1;
  std::list<NextHop> nhl = rte.getNexthopList().getNextHops();
  for (std::list<NextHop>::iterator it = nhl.begin();
       it != nhl.end() ; it++, i++) {
    os << "  Nexthop " << i << ": " << (*it) << std::endl;
  }
  return os;
}

} //namespace nlsr

#endif //NLSR_ROUTING_TABLE_ENTRY_HPP
