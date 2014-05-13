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
mapEntryCompareByRouter(MapEntry& mpe1, const ndn::Name& rtrName)
{
  return mpe1.getRouter() == rtrName;
}

static bool
mapEntryCompareByMappingNo(MapEntry& mpe1, int32_t mappingNo)
{
  return mpe1.getMappingNumber() == mappingNo;
}

void
Map::addEntry(const ndn::Name& rtrName)
{
  MapEntry me(rtrName, m_mappingIndex);
  if (addEntry(me)) {
    m_mappingIndex++;
  }
}

bool
Map::addEntry(MapEntry& mpe)
{
  //cout << mpe;
  std::list<MapEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  ndn::bind(&mapEntryCompareByRouter,
                                                            _1, mpe.getRouter()));
  if (it == m_table.end()) {
    m_table.push_back(mpe);
    return true;
  }
  return false;
}

const ndn::Name
Map::getRouterNameByMappingNo(int32_t mn)
{
  std::list<MapEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  ndn::bind(&mapEntryCompareByMappingNo,
                                                            _1, mn));
  if (it != m_table.end()) {
    return (*it).getRouter();
  }
  return ndn::Name();
}

int32_t
Map::getMappingNoByRouterName(const ndn::Name& rName)
{
  std::list<MapEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  ndn::bind(&mapEntryCompareByRouter,
                                                            _1, rName));
  if (it != m_table.end()) {
    return (*it).getMappingNumber();
  }
  return -1;
}

void
Map::createFromAdjLsdb(Nlsr& pnlsr)
{
  std::list<AdjLsa> adjLsdb = pnlsr.getLsdb().getAdjLsdb();
  for (std::list<AdjLsa>::iterator it = adjLsdb.begin();
       it != adjLsdb.end() ; it++) {
    addEntry((*it).getOrigRouter());
    std::list<Adjacent> adl = (*it).getAdl().getAdjList();
    for (std::list<Adjacent>::iterator itAdl = adl.begin();
         itAdl != adl.end() ; itAdl++) {
      addEntry((*itAdl).getName());
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
  for (std::list<MapEntry>::iterator it = ml.begin(); it != ml.end() ; it++) {
    os << (*it);
  }
  return os;
}

} //namespace nlsr
