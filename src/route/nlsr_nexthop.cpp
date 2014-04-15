#include "nlsr_nexthop.hpp"
#include "utility/nlsr_logger.hpp"
#define THIS_FILE "nlsr_nexthop.cpp"

namespace nlsr
{

  ostream&
  operator<<(ostream& os, NextHop& nh)
  {
    os<<"Face: "<<nh.getConnectingFace()<<"  Route Cost: "<<nh.getRouteCost();
    return os;
  }

}//namespace nlsr
