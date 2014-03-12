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
      : router()
      , mappingNumber(-1)
    {
    }

    ~MapEntry()
    {
    }

    MapEntry(string rtr, int mn)
    {
      router=rtr;
      mappingNumber=mn;
    }

    string getRouter()
    {
      return router;
    }

    int getMappingNumber()
    {
      return mappingNumber;
    }
  private:
    string router;
    int mappingNumber;
  };

  ostream&
  operator<<(ostream& os, MapEntry& mpe);

  class Map
  {
  public:
    Map()
      : mappingIndex(0)
    {
    }


    void addMapElement(string& rtrName);
    void createMapFromAdjLsdb(Nlsr& pnlsr);
    string getRouterNameByMappingNo(int mn);
    int getMappingNoByRouterName(string& rName);
    void resetMap();
    std::list< MapEntry >& getMapList()
    {
      return rMap;
    }

    int getMapSize()
    {
      return rMap.size();
    }


  private:
    bool addMapElement(MapEntry& mpe);

    int mappingIndex;
    std::list< MapEntry > rMap;
  };

  ostream&
  operator<<(ostream& os, Map& rMap);

} // namespace nlsr
#endif
