#include <list>
#include <utility>
#include <algorithm>

#include "nlsr.hpp"
#include "name-prefix-table.hpp"
#include "name-prefix-table-entry.hpp"



namespace nlsr {

using namespace std;

static bool
npteCompare(NamePrefixTableEntry& npte, const string& name)
{
  return npte.getNamePrefix() == name;
}



void
NamePrefixTable::addEntry(const string& name, RoutingTableEntry& rte, Nlsr& pnlsr)
{
  std::list<NamePrefixTableEntry>::iterator it = std::find_if(m_table.begin(),
                                                              m_table.end(), bind(&npteCompare, _1, name));
  if (it == m_table.end())
  {
    NamePrefixTableEntry newEntry(name);
    newEntry.addRoutingTableEntry(rte);
    newEntry.generateNhlfromRteList();
    newEntry.getNexthopList().sort();
    m_table.push_back(newEntry);
    if (rte.getNexthopList().getSize() > 0)
    {
      pnlsr.getFib().update(pnlsr, name, newEntry.getNexthopList());
    }
  }
  else
  {
    if (rte.getNexthopList().getSize() > 0)
    {
      (*it).addRoutingTableEntry(rte);
      (*it).generateNhlfromRteList();
      (*it).getNexthopList().sort();
      pnlsr.getFib().update(pnlsr, name, (*it).getNexthopList());
    }
    else
    {
      (*it).resetRteListNextHop();
      (*it).getNexthopList().reset();
      pnlsr.getFib().remove(pnlsr, name);
    }
  }
}

void
NamePrefixTable::removeEntry(const string& name, RoutingTableEntry& rte, Nlsr& pnlsr)
{
  std::list<NamePrefixTableEntry>::iterator it = std::find_if(m_table.begin(),
                                                              m_table.end(), bind(&npteCompare, _1, name));
  if (it != m_table.end())
  {
    string destRouter = rte.getDestination();
    (*it).removeRoutingTableEntry(rte);
    if (((*it).getRteListSize() == 0) &&
        (!pnlsr.getLsdb().doesLsaExist(destRouter + "/1", 1)) &&
        (!pnlsr.getLsdb().doesLsaExist(destRouter + "/2", 2)) &&
        (!pnlsr.getLsdb().doesLsaExist(destRouter + "/3", 3)))
    {
      m_table.erase(it);
      pnlsr.getFib().remove(pnlsr, name);
    }
    else
    {
      (*it).generateNhlfromRteList();
      pnlsr.getFib().update(pnlsr, name, (*it).getNexthopList());
    }
  }
}


void
NamePrefixTable::addEntry(const string& name, const string& destRouter, Nlsr& pnlsr)
{
  RoutingTableEntry* rteCheck =
    pnlsr.getRoutingTable().findRoutingTableEntry(destRouter);
  if (rteCheck != 0)
  {
    addEntry(name, *(rteCheck) , pnlsr);
  }
  else
  {
    RoutingTableEntry rte(destRouter);
    addEntry(name, rte, pnlsr);
  }
}

void
NamePrefixTable::removeEntry(const string& name, const string& destRouter, Nlsr& pnlsr)
{
  RoutingTableEntry* rteCheck =
    pnlsr.getRoutingTable().findRoutingTableEntry(destRouter);
  if (rteCheck != 0)
  {
    removeEntry(name, *(rteCheck), pnlsr);
  }
  else
  {
    RoutingTableEntry rte(destRouter);
    removeEntry(name, rte, pnlsr);
  }
}

void
NamePrefixTable::updateWithNewRoute(Nlsr& pnlsr)
{
  for (std::list<NamePrefixTableEntry>::iterator it = m_table.begin();
       it != m_table.end(); ++it)
  {
    std::list<RoutingTableEntry> rteList = (*it).getRteList();
    for (std::list<RoutingTableEntry>::iterator rteit = rteList.begin();
         rteit != rteList.end(); ++rteit)
    {
      RoutingTableEntry* rteCheck =
        pnlsr.getRoutingTable().findRoutingTableEntry((*rteit).getDestination());
      if (rteCheck != 0)
      {
        addEntry((*it).getNamePrefix(), *(rteCheck), pnlsr);
      }
      else
      {
        RoutingTableEntry rte((*rteit).getDestination());
        addEntry((*it).getNamePrefix(), rte, pnlsr);
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
       ++it)
  {
    cout << (*it) << endl;
  }
}

} //namespace nlsr
