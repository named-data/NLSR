#ifndef NLSR_NPTE_HPP
#define NLSR_NPTE_HPP

#include <list>
#include <utility>
#include "routing-table-entry.hpp"

namespace nlsr {

using namespace std;

class NamePrefixTableEntry
{
public:
  NamePrefixTableEntry()
    : m_namePrefix()
    , m_nhl()
  {
  }

  NamePrefixTableEntry(string np)
    : m_nhl()
  {
    m_namePrefix = np;
  }

  std::string
  getNamePrefix()
  {
    return m_namePrefix;
  }

  std::list<RoutingTableEntry>&
  getRteList()
  {
    return m_rteList;
  }

  void
  resetRteListNextHop()
  {
    if (m_rteList.size() > 0)
    {
      for (std::list<RoutingTableEntry>::iterator it = m_rteList.begin();
           it != m_rteList.end(); ++it)
      {
        (*it).getNhl().reset();
      }
    }
  }

  int
  getRteListSize()
  {
    return m_rteList.size();
  }

  NexthopList&
  getNhl()
  {
    return m_nhl;
  }

  void
  generateNhlfromRteList();

  void
  removeRoutingTableEntry(RoutingTableEntry& rte);

  void
  addRoutingTableEntry(RoutingTableEntry& rte);

private:
  std::string m_namePrefix;
  std::list<RoutingTableEntry> m_rteList;
  NexthopList m_nhl;
};

std::ostream&
operator<<(std::ostream& os, NamePrefixTableEntry& npte);

}//namespace nlsr

#endif //NLSR_NPTE_HPP
