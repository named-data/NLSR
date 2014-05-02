#include <iostream>
#include <string>
#include <cmath>
#include <limits>


#include "adjacent.hpp"

namespace nlsr {

using namespace std;

Adjacent::Adjacent(const string& an, uint32_t cf, double lc, uint32_t s, uint32_t iton)
{
  m_name = an;
  m_connectingFace = cf;
  m_linkCost = lc;
  m_status = s;
  m_interestTimedOutNo = iton;
}

bool
Adjacent::operator==(const Adjacent& adjacent) const
{
  return (m_name == adjacent.getName()) &&
         (m_connectingFace == adjacent.getConnectingFace()) &&
         (std::abs(m_linkCost - adjacent.getLinkCost()) <
          std::numeric_limits<double>::epsilon()) ;
}

bool
Adjacent::compareName(const Adjacent& adjacent)
{
  return m_name == adjacent.getName();
}

std::ostream&
operator<<(std::ostream& os, const Adjacent& adj)
{
  os << "Adjacent : " << adj.getName() << endl;
  os << "Connecting Face: " << adj.getConnectingFace() << endl;
  os << "Link Cost: " << adj.getLinkCost() << endl;
  os << "Status: " << adj.getStatus() << endl;
  os << "Interest Timed out: " << adj.getInterestTimedOutNo() << endl;
  return os;
}

} //namespace nlsr
