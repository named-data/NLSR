#ifndef NLSR_NPT_HPP
#define NLSR_NPT_HPP

#include <list>
#include "nlsr_npte.hpp"
#include "nlsr_rte.hpp"

namespace nlsr
{

    using namespace std;

    class Nlsr;

    class Npt
    {
    public:
        Npt()
        {
        }
        void addNpte(string name, string destRouter, Nlsr& pnlsr);
        void removeNpte(string name, string destRouter, Nlsr& pnlsr);
        void updateNptWithNewRoute(Nlsr& pnlsr);
        void printNpt();
    private:
        void addNpte(string name, RoutingTableEntry& rte, Nlsr& pnlsr);
        void removeNpte(string name, RoutingTableEntry& rte, Nlsr& pnlsr);
    private:
        std::list<Npte> npteList;
    };

}//namespace nlsr

#endif
