#ifndef NLSR_NAME_PREFIX_TABLE_ENTRY_HPP
#define NLSR_NAME_PREFIX_TABLE_ENTRY_HPP

#include <list>
#include <utility>
#include <boost/cstdint.hpp>

#include "routing-table-entry.hpp"

namespace nlsr {

class NamePrefixTableEntry
{
public:
  NamePrefixTableEntry()
  {
  }

  NamePrefixTableEntry(const std::string& namePrefix)
    : m_nexthopList()
  {
    m_namePrefix = namePrefix;
  }

  const std::string&
  getNamePrefix() const
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
        (*it).getNexthopList().reset();
      }
    }
  }

  size_t
  getRteListSize()
  {
    return m_rteList.size();
  }

  NexthopList&
  getNexthopList()
  {
    return m_nexthopList;
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
  NexthopList m_nexthopList;
};

std::ostream&
operator<<(std::ostream& os, NamePrefixTableEntry& npte);

}//namespace nlsr

#endif //NLSR_NAME_PREFIX_TABLE_ENTRY_HPP
