#include<iostream>
#include<list>

#include "nlsr.hpp"
#include "nlsr_adjacent.hpp"
#include "nlsr_lsa.hpp"
#include "nlsr_lsdb.hpp"
#include "nlsr_map.hpp"

namespace nlsr
{

  using namespace std;

  ostream&
  operator<<(ostream& os, MapEntry& mpe)
  {
    os<<"MapEntry: ( Router: "<<mpe.getRouter()<<" Mapping No: ";
    os<<mpe.getMappingNumber()<<" )"<<endl;
    return os;
  }

  static bool
  mapEntryCompareByRouter(MapEntry& mpe1, string& rtrName)
  {
    return mpe1.getRouter()==rtrName;
  }

  static bool
  mapEntryCompareByMappingNo(MapEntry& mpe1, int mappingNo)
  {
    return mpe1.getMappingNumber()==mappingNo;
  }

  void
  Map::addMapElement(string& rtrName)
  {
    MapEntry me(rtrName,mappingIndex);
    if (  addMapElement(me) )
    {
      mappingIndex++;
    }
  }

  bool
  Map::addMapElement(MapEntry& mpe)
  {
    //cout << mpe;
    std::list<MapEntry >::iterator it = std::find_if( rMap.begin(),
                                        rMap.end(),
                                        bind(&mapEntryCompareByRouter, _1, mpe.getRouter()));
    if ( it == rMap.end() )
    {
      rMap.push_back(mpe);
      return true;
    }
    return false;
  }

  string
  Map::getRouterNameByMappingNo(int mn)
  {
    std::list<MapEntry >::iterator it = std::find_if( rMap.begin(),
                                        rMap.end(),
                                        bind(&mapEntryCompareByMappingNo, _1, mn));
    if ( it != rMap.end() )
    {
      return (*it).getRouter();
    }
    return "";
  }

  int
  Map::getMappingNoByRouterName(string& rName)
  {
    std::list<MapEntry >::iterator it = std::find_if( rMap.begin(),
                                        rMap.end(),
                                        bind(&mapEntryCompareByRouter, _1, rName));
    if ( it != rMap.end() )
    {
      return (*it).getMappingNumber();
    }
    return -1;
  }

  void
  Map::createMapFromAdjLsdb(Nlsr& pnlsr)
  {
    std::list<AdjLsa> adjLsdb=pnlsr.getLsdb().getAdjLsdb();
    for( std::list<AdjLsa>::iterator it=adjLsdb.begin();
         it!= adjLsdb.end() ; it++)
    {
      string linkStartRouter=(*it).getOrigRouter();
      addMapElement(linkStartRouter);
      std::list<Adjacent> adl=(*it).getAdl().getAdjList();
      for( std::list<Adjacent>::iterator itAdl=adl.begin();
           itAdl!= adl.end() ; itAdl++)
      {
        string linkEndRouter=(*itAdl).getAdjacentName();
        addMapElement(linkEndRouter);
      }
    }
  }

  void
  Map::resetMap()
  {
    rMap.clear();
    mappingIndex=0;
  }

  ostream&
  operator<<(ostream& os, Map& rMap)
  {
    os<<"---------------Map----------------------"<<endl;
    std::list< MapEntry > ml=rMap.getMapList();
    for( std::list<MapEntry>::iterator it=ml.begin(); it!= ml.end() ; it++)
    {
      os<< (*it);
    }
    return os;
  }

} //namespace nlsr
