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
  NamePrefixTable(Nlsr& nlsr)
    : m_nlsr(nlsr)
  {
  }

  void
  addEntry(const ndn::Name& name, const ndn::Name& destRouter);

  void
  removeEntry(const ndn::Name& name, const ndn::Name& destRouter);

  void
  updateWithNewRoute();

  void
  print();

private:
  void
  addEntry(const ndn::Name& name, RoutingTableEntry& rte);

  void
  removeEntry(const ndn::Name& name, RoutingTableEntry& rte);

private:
  Nlsr& m_nlsr;
  std::list<NamePrefixTableEntry> m_table;
};

}//namespace nlsr

#endif //NLSR_NAME_PREFIX_TABLE_HPP
