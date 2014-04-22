#include "nexthop.hpp"

namespace nlsr {

std::ostream&
operator<<(std::ostream& os, NextHop& nh)
{
  os << "Face: " << nh.getConnectingFace() << "  Route Cost: " <<
     nh.getRouteCost();
  return os;
}

}//namespace nlsr
