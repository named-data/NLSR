#ifndef NLSR_MAP_HPP
#define NLSR_MAP_HPP

#include <iostream>
#include <list>
#include <boost/cstdint.hpp>

#include <ndn-cxx/common.hpp>

#include "map-entry.hpp"

namespace nlsr {

class Nlsr;

class Map
{
public:
  Map()
    : m_mappingIndex(0)
  {
  }


  void
  addEntry(const ndn::Name& rtrName);

  void
  createFromAdjLsdb(Nlsr& pnlsr);

  const ndn::Name
  getRouterNameByMappingNo(int32_t mn);

  int32_t
  getMappingNoByRouterName(const ndn::Name& rName);

  void
  reset();

  std::list<MapEntry>&
  getMapList()
  {
    return m_table;
  }

  size_t
  getMapSize() const
  {
    return m_table.size();
  }


private:
  bool
  addEntry(MapEntry& mpe);

  int32_t m_mappingIndex;
  std::list<MapEntry> m_table;
};

std::ostream&
operator<<(std::ostream& os, Map& map);

} // namespace nlsr
#endif //NLSR_MAP_HPP
