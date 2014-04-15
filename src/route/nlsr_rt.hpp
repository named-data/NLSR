#ifndef NLSR_RT_HPP
#define NLSR_RT_HPP

#include<iostream>
#include<utility>
#include<string>

#include "nlsr_rte.hpp"

namespace nlsr
{

  class Nlsr;
  class NextHop;

  using namespace std;

  class RoutingTable
  {
  public:
    RoutingTable()
      : NO_NEXT_HOP(-12345)
    {
    }
    void calculate(Nlsr& pnlsr);
    void addNextHop(string destRouter, NextHop& nh);
    void printRoutingTable();

    void addNextHopToDryTable(string destRouter, NextHop& nh);
    void printDryRoutingTable();
    std::pair<RoutingTableEntry&, bool> findRoutingTableEntry(string destRouter);
    void scheduleRoutingTableCalculation(Nlsr& pnlsr);

  private:
    void calculateLsRoutingTable(Nlsr& pnlsr);
    void calculateHypRoutingTable(Nlsr& pnlsr);
    void calculateHypDryRoutingTable(Nlsr& pnlsr);

    void clearRoutingTable();
    void clearDryRoutingTable();

    const int NO_NEXT_HOP;

    std::list< RoutingTableEntry > m_rTable;
    std::list< RoutingTableEntry > m_dryTable;
  };

}//namespace nlsr

#endif
