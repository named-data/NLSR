#include <iostream>
#include <list>

#include "nlsr.hpp"
#include "adjacent.hpp"
#include "lsa.hpp"
#include "lsdb.hpp"
#include "map.hpp"

namespace nlsr {

using namespace std;

static bool
mapEntryCompareByRouter(MapEntry& mpe1, const string& rtrName)
{
  return mpe1.getRouter() == rtrName;
}

static bool
mapEntryCompareByMappingNo(MapEntry& mpe1, int32_t mappingNo)
{
  return mpe1.getMappingNumber() == mappingNo;
}

void
Map::addEntry(const string& rtrName)
{
  MapEntry me(rtrName, m_mappingIndex);
  if (addEntry(me))
  {
    m_mappingIndex++;
  }
}

bool
Map::addEntry(MapEntry& mpe)
{
  //cout << mpe;
  std::list<MapEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&mapEntryCompareByRouter, _1, mpe.getRouter()));
  if (it == m_table.end())
  {
    m_table.push_back(mpe);
    return true;
  }
  return false;
}

string
Map::getRouterNameByMappingNo(int32_t mn)
{
  std::list<MapEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&mapEntryCompareByMappingNo,
                                                       _1, mn));
  if (it != m_table.end())
  {
    return (*it).getRouter();
  }
  return "";
}

int32_t
Map::getMappingNoByRouterName(string& rName)
{
  std::list<MapEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&mapEntryCompareByRouter,
                                                       _1, rName));
  if (it != m_table.end())
  {
    return (*it).getMappingNumber();
  }
  return -1;
}

void
Map::createFromAdjLsdb(Nlsr& pnlsr)
{
  std::list<AdjLsa> adjLsdb = pnlsr.getLsdb().getAdjLsdb();
  for (std::list<AdjLsa>::iterator it = adjLsdb.begin();
       it != adjLsdb.end() ; it++)
  {
    string linkStartRouter = (*it).getOrigRouter();
    addEntry(linkStartRouter);
    std::list<Adjacent> adl = (*it).getAdl().getAdjList();
    for (std::list<Adjacent>::iterator itAdl = adl.begin();
         itAdl != adl.end() ; itAdl++)
    {
      string linkEndRouter = (*itAdl).getName();
      addEntry(linkEndRouter);
    }
  }
}

void
Map::reset()
{
  m_table.clear();
  m_mappingIndex = 0;
}

ostream&
operator<<(ostream& os, Map& map)
{
  os << "---------------Map----------------------" << endl;
  std::list<MapEntry> ml = map.getMapList();
  for (std::list<MapEntry>::iterator it = ml.begin(); it != ml.end() ; it++)
  {
    os << (*it);
  }
  return os;
}

} //namespace nlsr
