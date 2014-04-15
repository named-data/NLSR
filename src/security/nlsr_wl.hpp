#ifndef NLSR_WL_HPP
#define NLSR_WL_HPP

#include "nlsr_wle.hpp"

namespace nlsr
{
  class WaitingList
  {
    public:
      WaitingList()
        : m_waitingTable()
      {}
      
      std::list<WaitingListEntry>& getWaitingTable()
      {
        return m_waitingTable;
      }
      
      bool add(std::string respCert, std::string waitee);
      std::pair<WaitingListEntry, bool> getWaitingListEntry(std::string respCert);
      bool remove(std::string respCert);
      
    private:
      std::list<WaitingListEntry> m_waitingTable;
  };
  
  std::ostream& operator<<(std::ostream& os, WaitingList wl);
}

#endif
