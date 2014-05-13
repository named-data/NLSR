#include <list>
#include <utility>
#include <algorithm>

#include "nlsr.hpp"
#include "name-prefix-table.hpp"
#include "name-prefix-table-entry.hpp"
#include "routing-table.hpp"



namespace nlsr {

using namespace std;

static bool
npteCompare(NamePrefixTableEntry& npte, const ndn::Name& name)
{
  return npte.getNamePrefix() == name;
}



void
NamePrefixTable::addEntry(const ndn::Name& name, RoutingTableEntry& rte)
{
  std::list<NamePrefixTableEntry>::iterator it = std::find_if(m_table.begin(),
                                                              m_table.end(),
                                                              ndn::bind(&npteCompare, _1, name));
  if (it == m_table.end()) {
    NamePrefixTableEntry newEntry(name);
    newEntry.addRoutingTableEntry(rte);
    newEntry.generateNhlfromRteList();
    newEntry.getNexthopList().sort();
    m_table.push_back(newEntry);
    if (rte.getNexthopList().getSize() > 0) {
      m_nlsr.getFib().update(name, newEntry.getNexthopList());
    }
  }
  else {
    if (rte.getNexthopList().getSize() > 0) {
      (*it).addRoutingTableEntry(rte);
      (*it).generateNhlfromRteList();
      (*it).getNexthopList().sort();
      m_nlsr.getFib().update(name, (*it).getNexthopList());
    }
    else {
      (*it).resetRteListNextHop();
      (*it).getNexthopList().reset();
      m_nlsr.getFib().remove(name);
    }
  }
}

void
NamePrefixTable::removeEntry(const ndn::Name& name, RoutingTableEntry& rte)
{
  std::list<NamePrefixTableEntry>::iterator it = std::find_if(m_table.begin(),
                                                              m_table.end(),
                                                              ndn::bind(&npteCompare, _1, name));
  if (it != m_table.end()) {
    ndn::Name destRouter = rte.getDestination();
    (*it).removeRoutingTableEntry(rte);
    if (((*it).getRteListSize() == 0) &&
        (!m_nlsr.getLsdb().doesLsaExist(destRouter.append("/name"),
                                        std::string("name"))) &&
        (!m_nlsr.getLsdb().doesLsaExist(destRouter.append("/adjacency"),
                                        std::string("adjacency"))) &&
        (!m_nlsr.getLsdb().doesLsaExist(destRouter.append("/coordinate"),
                                        std::string("coordinate")))) {
      m_table.erase(it);
      m_nlsr.getFib().remove(name);
    }
    else {
      (*it).generateNhlfromRteList();
      m_nlsr.getFib().update(name, (*it).getNexthopList());
    }
  }
}


void
NamePrefixTable::addEntry(const ndn::Name& name, const ndn::Name& destRouter)
{
  //
  RoutingTableEntry* rteCheck =
    m_nlsr.getRoutingTable().findRoutingTableEntry(destRouter);
  if (rteCheck != 0) {
    addEntry(name, *(rteCheck));
  }
  else {
    RoutingTableEntry rte(destRouter);
    addEntry(name, rte);
  }
}

void
NamePrefixTable::removeEntry(const ndn::Name& name, const ndn::Name& destRouter)
{
  //
  RoutingTableEntry* rteCheck =
    m_nlsr.getRoutingTable().findRoutingTableEntry(destRouter);
  if (rteCheck != 0) {
    removeEntry(name, *(rteCheck));
  }
  else {
    RoutingTableEntry rte(destRouter);
    removeEntry(name, rte);
  }
}

void
NamePrefixTable::updateWithNewRoute()
{
  for (std::list<NamePrefixTableEntry>::iterator it = m_table.begin();
       it != m_table.end(); ++it) {
    std::list<RoutingTableEntry> rteList = (*it).getRteList();
    for (std::list<RoutingTableEntry>::iterator rteit = rteList.begin();
         rteit != rteList.end(); ++rteit) {
      RoutingTableEntry* rteCheck =
        m_nlsr.getRoutingTable().findRoutingTableEntry((*rteit).getDestination());
      if (rteCheck != 0) {
        addEntry((*it).getNamePrefix(), *(rteCheck));
      }
      else {
        RoutingTableEntry rte((*rteit).getDestination());
        addEntry((*it).getNamePrefix(), rte);
      }
    }
  }
}

void
NamePrefixTable::print()
{
  std::cout << "----------------NPT----------------------" << std::endl;
  for (std::list<NamePrefixTableEntry>::iterator it = m_table.begin();
       it != m_table.end();
       ++it) {
    cout << (*it) << endl;
  }
}

} //namespace nlsr
