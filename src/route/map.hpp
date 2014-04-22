#ifndef NLSR_MAP_HPP
#define NLSR_MAP_HPP

#include <iostream>
#include <list>

#include <ndn-cpp-dev/face.hpp>

namespace nlsr {

class Nlsr;

class MapEntry
{
public:
  MapEntry()
    : m_router()
    , m_mappingNumber(-1)
  {
  }

  ~MapEntry()
  {
  }

  MapEntry(std::string rtr, int mn)
  {
    m_router = rtr;
    m_mappingNumber = mn;
  }

  std::string
  getRouter() const
  {
    return m_router;
  }

  int
  getMappingNumber() const
  {
    return m_mappingNumber;
  }

private:
  std::string m_router;
  int m_mappingNumber;
};

std::ostream&
operator<<(std::ostream& os, MapEntry& mpe);

class Map
{
public:
  Map()
    : m_mappingIndex(0)
  {
  }


  void
  addElement(std::string& rtrName);

  void
  createFromAdjLsdb(Nlsr& pnlsr);

  std::string
  getRouterNameByMappingNo(int mn);

  int
  getMappingNoByRouterName(std::string& rName);

  void
  reset();

  std::list<MapEntry>&
  getMapList()
  {
    return m_table;
  }

  int
  getMapSize() const
  {
    return m_table.size();
  }


private:
  bool
  addElement(MapEntry& mpe);

  int m_mappingIndex;
  std::list<MapEntry> m_table;
};

std::ostream&
operator<<(std::ostream& os, Map& map);

} // namespace nlsr
#endif //NLSR_MAP_HPP
