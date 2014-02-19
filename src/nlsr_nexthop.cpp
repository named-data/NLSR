#include "nlsr_nexthop.hpp"

namespace nlsr {

ostream&
operator<<(ostream& os, NextHop& nh)
{
	os<<"Face: "<<nh.getConnectingFace()<<"  Route Cost: "<<nh.getRouteCost();
	return os;
}

}//namespace nlsr
