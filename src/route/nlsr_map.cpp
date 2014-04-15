#include<iostream>
#include<list>

#include "nlsr.hpp"
#include "nlsr_adjacent.hpp"
#include "nlsr_lsa.hpp"
#include "nlsr_lsdb.hpp"
#include "nlsr_map.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_map.cpp"

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
  Map::addElement(string& rtrName)
  {
    MapEntry me(rtrName,m_mappingIndex);
    if (  addElement(me) )
    {
      m_mappingIndex++;
    }
  }

  bool
  Map::addElement(MapEntry& mpe)
  {
    //cout << mpe;
    std::list<MapEntry >::iterator it = std::find_if( m_table.begin(),
                                        m_table.end(),
                                        bind(&mapEntryCompareByRouter, _1, mpe.getRouter()));
    if ( it == m_table.end() )
    {
      m_table.push_back(mpe);
      return true;
    }
    return false;
  }

  string
  Map::getRouterNameByMappingNo(int mn)
  {
    std::list<MapEntry >::iterator it = std::find_if( m_table.begin(),
                                        m_table.end(),
                                        bind(&mapEntryCompareByMappingNo, _1, mn));
    if ( it != m_table.end() )
    {
      return (*it).getRouter();
    }
    return "";
  }

  int
  Map::getMappingNoByRouterName(string& rName)
  {
    std::list<MapEntry >::iterator it = std::find_if( m_table.begin(),
                                        m_table.end(),
                                        bind(&mapEntryCompareByRouter, _1, rName));
    if ( it != m_table.end() )
    {
      return (*it).getMappingNumber();
    }
    return -1;
  }

  void
  Map::createFromAdjLsdb(Nlsr& pnlsr)
  {
    std::list<AdjLsa> adjLsdb=pnlsr.getLsdb().getAdjLsdb();
    for( std::list<AdjLsa>::iterator it=adjLsdb.begin();
         it!= adjLsdb.end() ; it++)
    {
      string linkStartRouter=(*it).getOrigRouter();
      addElement(linkStartRouter);
      std::list<Adjacent> adl=(*it).getAdl().getAdjList();
      for( std::list<Adjacent>::iterator itAdl=adl.begin();
           itAdl!= adl.end() ; itAdl++)
      {
        string linkEndRouter=(*itAdl).getName();
        addElement(linkEndRouter);
      }
    }
  }

  void
  Map::reset()
  {
    m_table.clear();
    m_mappingIndex=0;
  }

  ostream&
  operator<<(ostream& os, Map& map)
  {
    os<<"---------------Map----------------------"<<endl;
    std::list< MapEntry > ml=map.getMapList();
    for( std::list<MapEntry>::iterator it=ml.begin(); it!= ml.end() ; it++)
    {
      os<< (*it);
    }
    return os;
  }

} //namespace nlsr
