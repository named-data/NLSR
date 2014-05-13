#ifndef NLSR_NEXTHOP_HPP
#define NLSR_NEXTHOP_HPP

#include <iostream>
#include <boost/cstdint.hpp>

namespace nlsr {
class NextHop
{
public:
  NextHop()
    : m_connectingFaceUri()
    , m_routeCost(0)
  {
  }

  NextHop(const std::string& cfu, double rc)
  {
    m_connectingFaceUri = cfu;
    m_routeCost = rc;
  }

  const std::string&
  getConnectingFaceUri() const
  {
    return m_connectingFaceUri;
  }

  void
  setConnectingFaceUri(const std::string& cfu)
  {
    m_connectingFaceUri = cfu;
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
  std::string m_connectingFaceUri;
  double m_routeCost;
};


inline std::ostream&
operator<<(std::ostream& os, const NextHop& nh)
{
  os << "Face: " << nh.getConnectingFaceUri() << "  Route Cost: " <<
     nh.getRouteCost();
  return os;
}

}//namespace nlsr

#endif //NLSR_NEXTHOP_HPP
