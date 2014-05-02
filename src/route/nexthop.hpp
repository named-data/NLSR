#ifndef NLSR_NEXTHOP_HPP
#define NLSR_NEXTHOP_HPP

#include <iostream>
#include <boost/cstdint.hpp>

namespace nlsr {
class NextHop
{
public:
  NextHop()
    : m_connectingFace(0)
    , m_routeCost(0)
  {
  }

  NextHop(uint32_t cf, double rc)
  {
    m_connectingFace = cf;
    m_routeCost = rc;
  }

  uint32_t
  getConnectingFace() const
  {
    return m_connectingFace;
  }

  void
  setConnectingFace(uint32_t cf)
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
  uint32_t m_connectingFace;
  double m_routeCost;
};


std::ostream&
operator<<(std::ostream& os, NextHop& nh);

}//namespace nlsr

#endif //NLSR_NEXTHOP_HPP
