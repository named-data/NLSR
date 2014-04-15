#include<iostream>
#include<string>
#include<cmath>
#include<limits>
#include "nlsr_adjacent.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_adjacent.cpp"

namespace nlsr
{

  using namespace std;

  Adjacent::Adjacent(const string& an, int cf, double lc, int s, int iton)
  {
    m_name=an;
    m_connectingFace=cf;
    m_linkCost=lc;
    m_status=s;
    m_interestTimedOutNo=iton;
  }

  bool
  Adjacent::isEqual(Adjacent& adj)
  {
    return ( m_name == adj.getName() ) &&
           ( m_connectingFace == adj.getConnectingFace() ) &&
           (std::abs(m_linkCost - adj.getLinkCost()) <
            std::numeric_limits<double>::epsilon()) ;
  }

  std::ostream&
  operator << (std::ostream &os, Adjacent &adj)
  {
    cout<<"Adjacent : "<< adj.getName()	<< endl;
    cout<<"Connecting Face: "<<adj.getConnectingFace()<<endl;
    cout<<"Link Cost: "<<adj.getLinkCost()<<endl;
    cout<<"Status: "<<adj.getStatus()<<endl;
    cout<<"Interest Timed out: "<<adj.getInterestTimedOutNo()<<endl;
    return os;
  }

} //namespace nlsr
