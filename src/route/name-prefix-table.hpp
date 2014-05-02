#ifndef NLSR_NAME_PREFIX_TABLE_HPP
#define NLSR_NAME_PREFIX_TABLE_HPP

#include <list>
#include <boost/cstdint.hpp>

#include "name-prefix-table-entry.hpp"
#include "routing-table-entry.hpp"

namespace nlsr {
class Nlsr;

class NamePrefixTable
{
public:
  NamePrefixTable()
  {
  }

  void
  addEntry(const std::string& name, const std::string& destRouter, Nlsr& pnlsr);

  void
  removeEntry(const std::string& name, const std::string& destRouter, Nlsr& pnlsr);

  void
  updateWithNewRoute(Nlsr& pnlsr);

  void
  print();

private:
  void
  addEntry(const std::string& name, RoutingTableEntry& rte, Nlsr& pnlsr);

  void
  removeEntry(const std::string& name, RoutingTableEntry& rte, Nlsr& pnlsr);

private:
  std::list<NamePrefixTableEntry> m_table;
};

}//namespace nlsr

#endif //NLSR_NAME_PREFIX_TABLE_HPP
