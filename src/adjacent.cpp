#include <iostream>
#include <string>
#include <cmath>
#include <limits>

#include "adjacent.hpp"

namespace nlsr {

using namespace std;

Adjacent::Adjacent(const string& an, int cf, double lc, int s, int iton)
{
  m_name = an;
  m_connectingFace = cf;
  m_linkCost = lc;
  m_status = s;
  m_interestTimedOutNo = iton;
}

bool
Adjacent::isEqual(Adjacent& adj)
{
  return (m_name == adj.getName()) &&
         (m_connectingFace == adj.getConnectingFace()) &&
         (std::abs(m_linkCost - adj.getLinkCost()) <
          std::numeric_limits<double>::epsilon()) ;
}

std::ostream&
operator<<(std::ostream& os, Adjacent& adj)
{
  os << "Adjacent : " << adj.getName()	<< endl;
  os << "Connecting Face: " << adj.getConnectingFace() << endl;
  os << "Link Cost: " << adj.getLinkCost() << endl;
  os << "Status: " << adj.getStatus() << endl;
  os << "Interest Timed out: " << adj.getInterestTimedOutNo() << endl;
  return os;
}

} //namespace nlsr
