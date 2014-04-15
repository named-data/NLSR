#include<iostream>
#include<string>
#include<list>

#include "nlsr_rt.hpp"
#include "nlsr.hpp"
#include "nlsr_map.hpp"
#include "nlsr_rtc.hpp"
#include "nlsr_rte.hpp"
#include "nlsr_npt.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_rt.cpp"

namespace nlsr
{

  using namespace std;

  void
  RoutingTable::calculate(Nlsr& pnlsr)
  {
    //debugging purpose
    std::cout<<pnlsr.getConfParameter()<<std::endl;
    pnlsr.getNpt().print();
    pnlsr.getLsdb().printAdjLsdb();
    pnlsr.getLsdb().printCorLsdb();
    pnlsr.getLsdb().printNameLsdb();
    if ( 	pnlsr.getIsRoutingTableCalculating() == 0 )
    {
      pnlsr.setIsRoutingTableCalculating(1); //setting routing table calculation
      if ( pnlsr.getLsdb().doesLsaExist(
             pnlsr.getConfParameter().getRouterPrefix()+"/"+"2",2) )
      {
        if(pnlsr.getIsBuildAdjLsaSheduled() != 1)
        {
          cout<<"CLearing old routing table ....."<<endl;
          clearRoutingTable();
          clearDryRoutingTable(); // for dry run options
          // calculate Link State routing
          if( (pnlsr.getConfParameter().getIsHyperbolicCalc() == 0 )
              || (pnlsr.getConfParameter().getIsHyperbolicCalc() == 2 ) )
          {
            calculateLsRoutingTable(pnlsr);
          }
          //calculate hyperbolic routing
          if ( pnlsr.getConfParameter().getIsHyperbolicCalc() == 1 )
          {
            calculateHypRoutingTable(pnlsr);
          }
          //calculate dry hyperbolic routing
          if ( pnlsr.getConfParameter().getIsHyperbolicCalc() == 2 )
          {
            calculateHypDryRoutingTable(pnlsr);
          }
          //need to update NPT here
          pnlsr.getNpt().updateWithNewRoute(pnlsr);
          //debugging purpose
          printRoutingTable();
          pnlsr.getNpt().print();
          pnlsr.getFib().print();
          //debugging purpose end
        }
        else
        {
          cout<<"Adjacency building is scheduled, so ";
          cout<<"routing table can not be calculated :("<<endl;
        }
      }
      else
      {
        cout<<"No Adj LSA of router itself,";
        cout<<	" so Routing table can not be calculated :("<<endl;
        clearRoutingTable();
        clearDryRoutingTable(); // for dry run options
        // need to update NPT here
        std::cout<<"Calling Update NPT With new Route"<<std::endl;
        pnlsr.getNpt().updateWithNewRoute(pnlsr);
        //debugging purpose
        printRoutingTable();
        pnlsr.getNpt().print();
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
    cout<<"RoutingTable::calculateLsRoutingTable Called"<<endl;
    Map vMap;
    vMap.createFromAdjLsdb(pnlsr);
    int numOfRouter=vMap.getMapSize();
    LinkStateRoutingTableCalculator lsrtc(numOfRouter);
    lsrtc.calculatePath(vMap,boost::ref(*this),pnlsr);
  }

  void
  RoutingTable::calculateHypRoutingTable(Nlsr& pnlsr)
  {
    Map vMap;
    vMap.createFromAdjLsdb(pnlsr);
    int numOfRouter=vMap.getMapSize();
    HypRoutingTableCalculator hrtc(numOfRouter,0);
    hrtc.calculatePath(vMap,boost::ref(*this),pnlsr);
  }

  void
  RoutingTable::calculateHypDryRoutingTable(Nlsr& pnlsr)
  {
    Map vMap;
    vMap.createFromAdjLsdb(pnlsr);
    int numOfRouter=vMap.getMapSize();
    HypRoutingTableCalculator hrtc(numOfRouter,1);
    hrtc.calculatePath(vMap,boost::ref(*this),pnlsr);
  }

  void
  RoutingTable::scheduleRoutingTableCalculation(Nlsr& pnlsr)
  {
    if ( pnlsr.getIsRouteCalculationScheduled() != 1 )
    {
      pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(15),
                                         ndn::bind(&RoutingTable::calculate,this,boost::ref(pnlsr)));
      pnlsr.setIsRouteCalculationScheduled(1);
    }
  }

  static bool
  routingTableEntryCompare(RoutingTableEntry& rte, string& destRouter)
  {
    return rte.getDestination()==destRouter;
  }

// function related to manipulation of routing table
  void
  RoutingTable::addNextHop(string destRouter, NextHop& nh)
  {
    std::pair<RoutingTableEntry&, bool> rte=findRoutingTableEntry(destRouter);
    if( !rte.second )
    {
      RoutingTableEntry rte(destRouter);
      rte.getNhl().addNextHop(nh);
      m_rTable.push_back(rte);
    }
    else
    {
      (rte.first).getNhl().addNextHop(nh);
    }
  }

  std::pair<RoutingTableEntry&, bool>
  RoutingTable::findRoutingTableEntry(string destRouter)
  {
    std::list<RoutingTableEntry >::iterator it = std::find_if( m_rTable.begin(),
        m_rTable.end(),
        bind(&routingTableEntryCompare, _1, destRouter));
    if ( it != m_rTable.end() )
    {
      return std::make_pair(boost::ref((*it)),true);
    }
    RoutingTableEntry rteEmpty;
    return std::make_pair(boost::ref(rteEmpty),false);
  }

  void
  RoutingTable::printRoutingTable()
  {
    cout<<"---------------Routing Table------------------"<<endl;
    for(std::list<RoutingTableEntry>::iterator it=m_rTable.begin() ;
        it != m_rTable.end(); ++it)
    {
      cout<<(*it)<<endl;
    }
  }


//function related to manipulation of dry routing table
  void
  RoutingTable::addNextHopToDryTable(string destRouter, NextHop& nh)
  {
    std::list<RoutingTableEntry >::iterator it = std::find_if( m_dryTable.begin(),
        m_dryTable.end(),
        bind(&routingTableEntryCompare, _1, destRouter));
    if ( it == m_dryTable.end() )
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
    cout<<"--------Dry Run's Routing Table--------------"<<endl;
    for(std::list<RoutingTableEntry>::iterator it=m_dryTable.begin() ;
        it != m_dryTable.end(); ++it)
    {
      cout<<(*it)<<endl;
    }
  }


  void
  RoutingTable::clearRoutingTable()
  {
    if( m_rTable.size() > 0 )
    {
      m_rTable.clear();
    }
  }

  void
  RoutingTable::clearDryRoutingTable()
  {
    if (m_dryTable.size()>0 )
    {
      m_dryTable.clear();
    }
  }

}//namespace nlsr

