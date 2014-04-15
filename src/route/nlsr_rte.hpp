#ifndef NLSR_RTE_HPP
#define NLSR_RTE_HPP

#include<iostream>

#include "nlsr_nhl.hpp"

namespace nlsr
{

  using namespace std;

  class RoutingTableEntry
  {
  public:
    RoutingTableEntry()
      : m_destination()
      , m_nhl()
    {
    }

    ~RoutingTableEntry()
    {
    }

    RoutingTableEntry(string dest)
      : m_nhl()
    {
      m_destination=dest;
    }

    string getDestination()
    {
      return m_destination;
    }

    Nhl& getNhl()
    {
      return m_nhl;
    }

  private:
    string m_destination;
    Nhl m_nhl;
  };

  ostream&
  operator<<(ostream& os, RoutingTableEntry& rte);

}

#endif
