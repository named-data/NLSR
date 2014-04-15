#ifndef NLSR_MAP_HPP
#define NLSR_MAP_HPP

#include <iostream>
#include <list>

#include <ndn-cpp-dev/face.hpp>

namespace nlsr
{

  class Nlsr;

  using namespace std;

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

    MapEntry(string rtr, int mn)
    {
      m_router=rtr;
      m_mappingNumber=mn;
    }

    string getRouter()
    {
      return m_router;
    }

    int getMappingNumber()
    {
      return m_mappingNumber;
    }
  private:
    string m_router;
    int m_mappingNumber;
  };

  ostream&
  operator<<(ostream& os, MapEntry& mpe);

  class Map
  {
  public:
    Map()
      : m_mappingIndex(0)
    {
    }


    void addElement(string& rtrName);
    void createFromAdjLsdb(Nlsr& pnlsr);
    string getRouterNameByMappingNo(int mn);
    int getMappingNoByRouterName(string& rName);
    void reset();
    std::list< MapEntry >& getMapList()
    {
      return m_table;
    }

    int getMapSize()
    {
      return m_table.size();
    }


  private:
    bool addElement(MapEntry& mpe);

    int m_mappingIndex;
    std::list< MapEntry > m_table;
  };

  ostream&
  operator<<(ostream& os, Map& map);

} // namespace nlsr
#endif
