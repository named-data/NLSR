#include <ndn-cpp-dev/face.hpp>
#include "nlsr_wl.hpp"

#define THIS_FILE "nlsr_wl.cpp"

namespace nlsr
{
  static bool
  waitingListCompare(const WaitingListEntry& w1, const std::string& respCert)
  {
    return w1.getResponsibleCert() == respCert;
  }
  
  std::pair<WaitingListEntry, bool> 
  WaitingList::getWaitingListEntry(std::string respCert)
  {
    std::list<WaitingListEntry>::iterator it = std::find_if( m_waitingTable.begin(),
                m_waitingTable.end(),ndn::bind(&waitingListCompare, _1, respCert));
    if( it != m_waitingTable.end() )
    {
      return std::make_pair(*(it),true);
    }
    
    WaitingListEntry wle;
    return std::make_pair(wle,false);
    
  }
  
  bool 
  WaitingList::add(std::string respCert, std::string waitee)
  {
    std::list<WaitingListEntry>::iterator it = std::find_if( m_waitingTable.begin(),
                m_waitingTable.end(),ndn::bind(&waitingListCompare, _1, respCert));
    if( it == m_waitingTable.end() )
    {
      WaitingListEntry newWle(respCert);
      newWle.addWaitee(waitee);
      m_waitingTable.push_back(newWle);
      return true;
    }
    else
    {
      return it->addWaitee(waitee);
    }
    return false;
  }
  
  bool 
  WaitingList::remove(std::string respCert)
  {
    std::list<WaitingListEntry>::iterator it = std::find_if( m_waitingTable.begin(),
                m_waitingTable.end(),ndn::bind(&waitingListCompare, _1, respCert));
    if( it == m_waitingTable.end() )
    {
      return false;
    }
    else
    {
      m_waitingTable.erase(it);
      return true;
    }
    return false;
  }
  
  std::ostream& 
  operator<<(std::ostream& os, WaitingList wl)
  {
    os<<"-------Waiting List--------"<<std::endl;
    std::list<WaitingListEntry> wles=wl.getWaitingTable();
    for( std::list<WaitingListEntry> ::iterator it=wles.begin(); 
                                                        it != wles.end(); ++it)
    {
      os<<*(it)<<std::endl;
    }
    return os;
  }
}
