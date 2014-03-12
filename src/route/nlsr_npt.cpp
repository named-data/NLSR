#include <list>
#include <utility>
#include <algorithm>

#include "nlsr_npt.hpp"
#include "nlsr_npte.hpp"
#include "nlsr.hpp"

namespace nlsr
{

  using namespace std;

  static bool
  npteCompare(Npte& npte, string& name)
  {
    return npte.getNamePrefix()==name;
  }



  void
  Npt::addNpte(string name, RoutingTableEntry& rte, Nlsr& pnlsr)
  {
    std::list<Npte >::iterator it = std::find_if( npteList.begin(),
                                    npteList.end(), bind(&npteCompare, _1, name));
    if ( it == npteList.end() )
    {
      Npte newEntry(name);
      newEntry.addRoutingTableEntry(rte);
      newEntry.generateNhlfromRteList();
      newEntry.getNhl().sortNhl();
      npteList.push_back(newEntry);
      if(rte.getNhl().getNhlSize()> 0)
      {
        pnlsr.getFib().updateFib(pnlsr, name,newEntry.getNhl());
      }
    }
    else
    {
      if ( rte.getNhl().getNhlSize()> 0 )
      {
        (*it).addRoutingTableEntry(rte);
        (*it).generateNhlfromRteList();
        (*it).getNhl().sortNhl();
        pnlsr.getFib().updateFib(pnlsr, name,(*it).getNhl());
      }
      else
      {
        (*it).resetRteListNextHop();
        (*it).getNhl().resetNhl();
        pnlsr.getFib().removeFromFib(pnlsr,name);
      }
    }
  }

  void
  Npt::removeNpte(string name, RoutingTableEntry& rte, Nlsr& pnlsr)
  {
    std::list<Npte >::iterator it = std::find_if( npteList.begin(),
                                    npteList.end(), bind(&npteCompare, _1, name));
    if ( it != npteList.end() )
    {
      string destRouter=rte.getDestination();
      (*it).removeRoutingTableEntry(rte);
      if ( ((*it).getRteListSize() == 0 ) &&
           (!pnlsr.getLsdb().doesLsaExist(destRouter+"/1",1) ) &&
           (!pnlsr.getLsdb().doesLsaExist(destRouter+"/2",2) ) &&
           (!pnlsr.getLsdb().doesLsaExist(destRouter+"/3",3) )   )
      {
        npteList.erase(it);
        pnlsr.getFib().removeFromFib(pnlsr,name);
      }
      else
      {
        (*it).generateNhlfromRteList();
        pnlsr.getFib().updateFib(pnlsr, name,(*it).getNhl());
      }
    }
  }


  void
  Npt::addNpteByDestName(string name, string destRouter, Nlsr& pnlsr)
  {
    std::pair<RoutingTableEntry& , bool> rteCheck=
      pnlsr.getRoutingTable().findRoutingTableEntry(destRouter);
    if(rteCheck.second)
    {
      addNpte(name,rteCheck.first,pnlsr);
    }
    else
    {
      RoutingTableEntry rte(destRouter);
      addNpte(name, rte,pnlsr);
    }
  }

  void
  Npt::removeNpte(string name, string destRouter, Nlsr& pnlsr)
  {
    std::pair<RoutingTableEntry& , bool> rteCheck=
      pnlsr.getRoutingTable().findRoutingTableEntry(destRouter);
    if(rteCheck.second)
    {
      removeNpte(name,rteCheck.first,pnlsr);
    }
    else
    {
      RoutingTableEntry rte(destRouter);
      removeNpte(name, rte,pnlsr);
    }
  }

  void
  Npt::updateNptWithNewRoute(Nlsr& pnlsr)
  {
    for(std::list<Npte >::iterator it=npteList.begin(); it!=npteList.end(); ++it)
    {
      std::list<RoutingTableEntry> rteList=(*it).getRteList();
      for(std::list<RoutingTableEntry >::iterator rteit=rteList.begin();
          rteit !=rteList.end(); ++rteit)
      {
        std::pair<RoutingTableEntry& , bool> rteCheck=
          pnlsr.getRoutingTable().findRoutingTableEntry((*rteit).getDestination());
        if(rteCheck.second)
        {
          addNpte((*it).getNamePrefix(),rteCheck.first,pnlsr);
        }
        else
        {
          RoutingTableEntry rte((*rteit).getDestination());
          addNpte((*it).getNamePrefix(), rte,pnlsr);
        }
      }
    }
  }

  void
  Npt::printNpt()
  {
    cout<<"----------------NPT----------------------"<<endl;
    for(std::list<Npte >::iterator it=npteList.begin(); it!=npteList.end(); ++it)
    {
      cout <<(*it)<<endl;
    }
  }

}
