#include <iostream>
#include <list>
#include <ndn-cpp-dev/face.hpp>
#include "nlsr_wle.hpp"

#define THIS_FILE "nlsr_wle.cpp"

namespace nlsr
{
  static bool
  waiteeCompare(std::string& w1, std::string& w2)
  {
    return w1 == w2 ;
  }
  
  bool
  WaitingListEntry::addWaitee(std::string waiteeName)
  {
    std::list<std::string>::iterator it = std::find_if( m_waitingCerts.begin(),
                m_waitingCerts.end(),ndn::bind(&waiteeCompare, _1, waiteeName));
    if( it == m_waitingCerts.end() )
    {
      m_waitingCerts.push_back(waiteeName);
      return true;
    }
    
    return false;
  }

  std::ostream& 
  operator<<(std::ostream& os, const WaitingListEntry& we)
  {
    os<<"-------------Wiating List Entry-------------"<<std::endl;
    os<<"Responsible Certificate: "<<we.getResponsibleCert()<<std::endl;
    std::list<std::string> waitee=we.getWaitingCerts();
    int i=1;
    for(std::list<std::string>::iterator it=waitee.begin(); 
                                             it!=waitee.end(); ++i, ++it)
    {
      os<<"Waite "<<i<<": "<<*(it)<<std::endl;
    }
    return os;
  }
}
