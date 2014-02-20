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

// Following two methods will update FIB with response to change in NPT

    void
    Npt::addNpte(string name, RoutingTableEntry& rte, Nlsr& pnlsr)
    {
        std::list<Npte >::iterator it = std::find_if( npteList.begin(),
                                        npteList.end(), bind(&npteCompare, _1, name));

        if ( it == npteList.end() )
        {
            Npte newEntry(	name);
            newEntry.addRoutingTableEntry(rte);
            newEntry.generateNhlfromRteList();
            npteList.push_back(newEntry);
            // update FIB here with nhl list newEntry.getNhl()
            pnlsr.getFib().updateFib(pnlsr, name,newEntry.getNhl(),
                                     pnlsr.getConfParameter().getMaxFacesPerPrefix());
        }
        else
        {
            (*it).addRoutingTableEntry(rte);
            (*it).generateNhlfromRteList();
            // update FIB here with nhl list from (*it).getNhl()
            pnlsr.getFib().updateFib(pnlsr, name,(*it).getNhl() ,
                                     pnlsr.getConfParameter().getMaxFacesPerPrefix());
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
                npteList.erase(it); // remove entry from NPT
                // remove FIB entry with this name
                pnlsr.getFib().removeFromFib(pnlsr,name);

            }
            else
            {
                (*it).generateNhlfromRteList();
                // update FIB entry with new NHL
                pnlsr.getFib().updateFib(pnlsr, name,(*it).getNhl(),
                                         pnlsr.getConfParameter().getMaxFacesPerPrefix());
            }
        }
    }


    void
    Npt::addNpte(string name, string destRouter, Nlsr& pnlsr)
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
