#include<iostream>
#include<string>
#include<cmath>
#include<limits>

#include "nlsr_adjacent.hpp"

namespace nlsr
{

  using namespace std;

  Adjacent::Adjacent(const string& an, int cf, double lc, int s, int iton)
  {
    adjacentName=an;
    connectingFace=cf;
    linkCost=lc;
    status=s;
    interestTimedOutNo=iton;
  }

  bool
  Adjacent::isAdjacentEqual(Adjacent& adj)
  {
    return ( adjacentName == adj.getAdjacentName() ) &&
           ( connectingFace == adj.getConnectingFace() ) &&
           (std::abs(linkCost - adj.getLinkCost()) <
            std::numeric_limits<double>::epsilon()) ;
  }

  std::ostream&
  operator << (std::ostream &os, Adjacent &adj)
  {
    cout<<"Adjacent : "<< adj.getAdjacentName()	<< endl;
    cout<<"Connecting Face: "<<adj.getConnectingFace()<<endl;
    cout<<"Link Cost: "<<adj.getLinkCost()<<endl;
    cout<<"Status: "<<adj.getStatus()<<endl;
    cout<<"Interest Timed out: "<<adj.getInterestTimedOutNo()<<endl;
    return os;
  }

} //namespace nlsr
