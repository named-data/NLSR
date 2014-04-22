#ifndef NLSR_NEXTHOP_HPP
#define NLSR_NEXTHOP_HPP

#include <iostream>

namespace nlsr {
class NextHop
{
public:
  NextHop()
    : m_connectingFace(0)
    , m_routeCost(0)
  {
  }

  NextHop(int cf, double rc)
  {
    m_connectingFace = cf;
    m_routeCost = rc;
  }

  int
  getConnectingFace() const
  {
    return m_connectingFace;
  }

  void
  setConnectingFace(int cf)
  {
    m_connectingFace = cf;
  }

  double
  getRouteCost() const
  {
    return m_routeCost;
  }

  void
  setRouteCost(double rc)
  {
    m_routeCost = rc;
  }

private:
  int m_connectingFace;
  double m_routeCost;
};


std::ostream&
operator<<(std::ostream& os, NextHop& nh);

}//namespace nlsr

#endif //NLSR_NEXTHOP_HPP
