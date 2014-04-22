#ifndef NLSR_RT_HPP
#define NLSR_RT_HPP

#include <iostream>
#include <utility>
#include <string>

#include "routing-table-entry.hpp"

namespace nlsr {

class Nlsr;
class NextHop;

class RoutingTable
{
public:
  RoutingTable()
    : m_NO_NEXT_HOP(-12345)
  {
  }
  void
  calculate(Nlsr& pnlsr);

  void
  addNextHop(std::string destRouter, NextHop& nh);

  void
  printRoutingTable();

  void
  addNextHopToDryTable(std::string destRouter, NextHop& nh);

  void
  printDryRoutingTable();

  std::pair<RoutingTableEntry&, bool>
  findRoutingTableEntry(std::string destRouter);

  void
  scheduleRoutingTableCalculation(Nlsr& pnlsr);

  int
  getNoNextHop()
  {
    return m_NO_NEXT_HOP;
  }

private:
  void
  calculateLsRoutingTable(Nlsr& pnlsr);

  void
  calculateHypRoutingTable(Nlsr& pnlsr);

  void
  calculateHypDryRoutingTable(Nlsr& pnlsr);

  void
  clearRoutingTable();

  void
  clearDryRoutingTable();

  const int m_NO_NEXT_HOP;

  std::list<RoutingTableEntry> m_rTable;
  std::list<RoutingTableEntry> m_dryTable;
};

}//namespace nlsr

#endif //NLSR_RT_HPP
