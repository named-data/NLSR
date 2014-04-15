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
    void addNpteByDestName(string name, string destRouter, Nlsr& pnlsr);
    void removeNpte(string name, string destRouter, Nlsr& pnlsr);
    void updateWithNewRoute(Nlsr& pnlsr);
    void print();
  private:
    void addNpte(string name, RoutingTableEntry& rte, Nlsr& pnlsr);
    void removeNpte(string name, RoutingTableEntry& rte, Nlsr& pnlsr);
  private:
    std::list<Npte> m_npteList;
  };

}//namespace nlsr

#endif
