#ifndef NLSR_NPT_HPP
#define NLSR_NPT_HPP

#include <list>
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
  addNpteByDestName(std::string name, std::string destRouter, Nlsr& pnlsr);

  void
  removeNpte(std::string name, std::string destRouter, Nlsr& pnlsr);

  void
  updateWithNewRoute(Nlsr& pnlsr);

  void
  print();

private:
  void
  addNpte(std::string name, RoutingTableEntry& rte, Nlsr& pnlsr);

  void
  removeNpte(std::string name, RoutingTableEntry& rte, Nlsr& pnlsr);

private:
  std::list<NamePrefixTableEntry> m_npteList;
};

}//namespace nlsr

#endif //NLSR_NPT_HPP
