#include <iostream>
#include <string>
#include <list>

#include "routing-table.hpp"
#include "nlsr.hpp"
#include "map.hpp"
#include "routing-table-calculator.hpp"
#include "routing-table-entry.hpp"
#include "name-prefix-table.hpp"

namespace nlsr {

using namespace std;

void
RoutingTable::calculate(Nlsr& pnlsr)
{
  //debugging purpose
  std::cout << pnlsr.getConfParameter() << std::endl;
  pnlsr.getNamePrefixTable().print();
  pnlsr.getLsdb().printAdjLsdb();
  pnlsr.getLsdb().printCorLsdb();
  pnlsr.getLsdb().printNameLsdb();
  if (pnlsr.getIsRoutingTableCalculating() == 0)
  {
    pnlsr.setIsRoutingTableCalculating(1); //setting routing table calculation
    if (pnlsr.getLsdb().doesLsaExist(
          pnlsr.getConfParameter().getRouterPrefix() + "/" + "2", 2))
    {
      if (pnlsr.getIsBuildAdjLsaSheduled() != 1)
      {
        std::cout << "CLearing old routing table ....." << std::endl;
        clearRoutingTable();
        clearDryRoutingTable(); // for dry run options
        // calculate Link State routing
        if ((pnlsr.getConfParameter().getIsHyperbolicCalc() == 0)
            || (pnlsr.getConfParameter().getIsHyperbolicCalc() == 2))
        {
          calculateLsRoutingTable(pnlsr);
        }
        //calculate hyperbolic routing
        if (pnlsr.getConfParameter().getIsHyperbolicCalc() == 1)
        {
          calculateHypRoutingTable(pnlsr);
        }
        //calculate dry hyperbolic routing
        if (pnlsr.getConfParameter().getIsHyperbolicCalc() == 2)
        {
          calculateHypDryRoutingTable(pnlsr);
        }
        //need to update NPT here
        pnlsr.getNamePrefixTable().updateWithNewRoute(pnlsr);
        //debugging purpose
        printRoutingTable();
        pnlsr.getNamePrefixTable().print();
        pnlsr.getFib().print();
        //debugging purpose end
      }
      else
      {
        std::cout << "Adjacency building is scheduled, so ";
        std::cout << "routing table can not be calculated :(" << std::endl;
      }
    }
    else
    {
      std::cout << "No Adj LSA of router itself,";
      std::cout <<	" so Routing table can not be calculated :(" << std::endl;
      clearRoutingTable();
      clearDryRoutingTable(); // for dry run options
      // need to update NPT here
      std::cout << "Calling Update NPT With new Route" << std::endl;
      pnlsr.getNamePrefixTable().updateWithNewRoute(pnlsr);
      //debugging purpose
      printRoutingTable();
      pnlsr.getNamePrefixTable().print();
      pnlsr.getFib().print();
      //debugging purpose end
    }
    pnlsr.setIsRouteCalculationScheduled(0); //clear scheduled flag
    pnlsr.setIsRoutingTableCalculating(0); //unsetting routing table calculation
  }
  else
  {
    scheduleRoutingTableCalculation(pnlsr);
  }
}


void
RoutingTable::calculateLsRoutingTable(Nlsr& pnlsr)
{
  std::cout << "RoutingTable::calculateLsRoutingTable Called" << std::endl;
  Map vMap;
  vMap.createFromAdjLsdb(pnlsr);
  int numOfRouter = vMap.getMapSize();
  LinkStateRoutingTableCalculator lsrtc(numOfRouter);
  lsrtc.calculatePath(vMap, boost::ref(*this), pnlsr);
}

void
RoutingTable::calculateHypRoutingTable(Nlsr& pnlsr)
{
  Map vMap;
  vMap.createFromAdjLsdb(pnlsr);
  int numOfRouter = vMap.getMapSize();
  HypRoutingTableCalculator hrtc(numOfRouter, 0);
  hrtc.calculatePath(vMap, boost::ref(*this), pnlsr);
}

void
RoutingTable::calculateHypDryRoutingTable(Nlsr& pnlsr)
{
  Map vMap;
  vMap.createFromAdjLsdb(pnlsr);
  int numOfRouter = vMap.getMapSize();
  HypRoutingTableCalculator hrtc(numOfRouter, 1);
  hrtc.calculatePath(vMap, boost::ref(*this), pnlsr);
}

void
RoutingTable::scheduleRoutingTableCalculation(Nlsr& pnlsr)
{
  if (pnlsr.getIsRouteCalculationScheduled() != 1)
  {
    pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(15),
                                       ndn::bind(&RoutingTable::calculate, this, boost::ref(pnlsr)));
    pnlsr.setIsRouteCalculationScheduled(1);
  }
}

static bool
routingTableEntryCompare(RoutingTableEntry& rte, string& destRouter)
{
  return rte.getDestination() == destRouter;
}

// function related to manipulation of routing table
void
RoutingTable::addNextHop(string destRouter, NextHop& nh)
{
  RoutingTableEntry* rteChk = findRoutingTableEntry(destRouter);
  if (rteChk == 0)
  {
    RoutingTableEntry rte(destRouter);
    rte.getNhl().addNextHop(nh);
    m_rTable.push_back(rte);
  }
  else
  {
    rteChk->getNhl().addNextHop(nh);
  }
}

RoutingTableEntry*
RoutingTable::findRoutingTableEntry(const string destRouter)
{
  std::list<RoutingTableEntry>::iterator it = std::find_if(m_rTable.begin(),
                                                           m_rTable.end(),
                                                           bind(&routingTableEntryCompare, _1, destRouter));
  if (it != m_rTable.end())
  {
    return &(*it);
  }
  return 0;
}

void
RoutingTable::printRoutingTable()
{
  std::cout << "---------------Routing Table------------------" << std::endl;
  for (std::list<RoutingTableEntry>::iterator it = m_rTable.begin() ;
       it != m_rTable.end(); ++it)
  {
    std::cout << (*it) << std::endl;
  }
}


//function related to manipulation of dry routing table
void
RoutingTable::addNextHopToDryTable(string destRouter, NextHop& nh)
{
  std::list<RoutingTableEntry>::iterator it = std::find_if(m_dryTable.begin(),
                                                           m_dryTable.end(),
                                                           bind(&routingTableEntryCompare, _1, destRouter));
  if (it == m_dryTable.end())
  {
    RoutingTableEntry rte(destRouter);
    rte.getNhl().addNextHop(nh);
    m_dryTable.push_back(rte);
  }
  else
  {
    (*it).getNhl().addNextHop(nh);
  }
}

void
RoutingTable::printDryRoutingTable()
{
  std::cout << "--------Dry Run's Routing Table--------------" << std::endl;
  for (std::list<RoutingTableEntry>::iterator it = m_dryTable.begin() ;
       it != m_dryTable.end(); ++it)
  {
    cout << (*it) << endl;
  }
}


void
RoutingTable::clearRoutingTable()
{
  if (m_rTable.size() > 0)
  {
    m_rTable.clear();
  }
}

void
RoutingTable::clearDryRoutingTable()
{
  if (m_dryTable.size() > 0)
  {
    m_dryTable.clear();
  }
}

}//namespace nlsr

