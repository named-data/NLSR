#include <list>
#include <utility>
#include <algorithm>

#include "npt.hpp"
#include "npte.hpp"
#include "nlsr.hpp"


namespace nlsr {

using namespace std;

static bool
npteCompare(Npte& npte, string& name)
{
  return npte.getNamePrefix() == name;
}



void
Npt::addNpte(string name, RoutingTableEntry& rte, Nlsr& pnlsr)
{
  std::list<Npte>::iterator it = std::find_if(m_npteList.begin(),
                                              m_npteList.end(), bind(&npteCompare, _1, name));
  if (it == m_npteList.end())
  {
    Npte newEntry(name);
    newEntry.addRoutingTableEntry(rte);
    newEntry.generateNhlfromRteList();
    newEntry.getNhl().sort();
    m_npteList.push_back(newEntry);
    if (rte.getNhl().getSize() > 0)
    {
      pnlsr.getFib().update(pnlsr, name, newEntry.getNhl());
    }
  }
  else
  {
    if (rte.getNhl().getSize() > 0)
    {
      (*it).addRoutingTableEntry(rte);
      (*it).generateNhlfromRteList();
      (*it).getNhl().sort();
      pnlsr.getFib().update(pnlsr, name, (*it).getNhl());
    }
    else
    {
      (*it).resetRteListNextHop();
      (*it).getNhl().reset();
      pnlsr.getFib().remove(pnlsr, name);
    }
  }
}

void
Npt::removeNpte(string name, RoutingTableEntry& rte, Nlsr& pnlsr)
{
  std::list<Npte>::iterator it = std::find_if(m_npteList.begin(),
                                              m_npteList.end(), bind(&npteCompare, _1, name));
  if (it != m_npteList.end())
  {
    string destRouter = rte.getDestination();
    (*it).removeRoutingTableEntry(rte);
    if (((*it).getRteListSize() == 0) &&
        (!pnlsr.getLsdb().doesLsaExist(destRouter + "/1", 1)) &&
        (!pnlsr.getLsdb().doesLsaExist(destRouter + "/2", 2)) &&
        (!pnlsr.getLsdb().doesLsaExist(destRouter + "/3", 3)))
    {
      m_npteList.erase(it);
      pnlsr.getFib().remove(pnlsr, name);
    }
    else
    {
      (*it).generateNhlfromRteList();
      pnlsr.getFib().update(pnlsr, name, (*it).getNhl());
    }
  }
}


void
Npt::addNpteByDestName(string name, string destRouter, Nlsr& pnlsr)
{
  RoutingTableEntry* rteCheck =
    pnlsr.getRoutingTable().findRoutingTableEntry(destRouter);
  if (rteCheck != 0)
  {
    addNpte(name, *(rteCheck) , pnlsr);
  }
  else
  {
    RoutingTableEntry rte(destRouter);
    addNpte(name, rte, pnlsr);
  }
}

void
Npt::removeNpte(string name, string destRouter, Nlsr& pnlsr)
{
  RoutingTableEntry* rteCheck =
    pnlsr.getRoutingTable().findRoutingTableEntry(destRouter);
  if (rteCheck != 0)
  {
    removeNpte(name, *(rteCheck), pnlsr);
  }
  else
  {
    RoutingTableEntry rte(destRouter);
    removeNpte(name, rte, pnlsr);
  }
}

void
Npt::updateWithNewRoute(Nlsr& pnlsr)
{
  for (std::list<Npte>::iterator it = m_npteList.begin(); it != m_npteList.end();
       ++it)
  {
    std::list<RoutingTableEntry> rteList = (*it).getRteList();
    for (std::list<RoutingTableEntry>::iterator rteit = rteList.begin();
         rteit != rteList.end(); ++rteit)
    {
      RoutingTableEntry* rteCheck =
        pnlsr.getRoutingTable().findRoutingTableEntry((*rteit).getDestination());
      if (rteCheck != 0)
      {
        addNpte((*it).getNamePrefix(), *(rteCheck), pnlsr);
      }
      else
      {
        RoutingTableEntry rte((*rteit).getDestination());
        addNpte((*it).getNamePrefix(), rte, pnlsr);
      }
    }
  }
}

void
Npt::print()
{
  std::cout << "----------------NPT----------------------" << std::endl;
  for (std::list<Npte>::iterator it = m_npteList.begin(); it != m_npteList.end();
       ++it)
  {
    cout << (*it) << endl;
  }
}

} //namespace nlsr
