#ifndef NLSR_WL_HPP
#define NLSR_WL_HPP

#include "nlsr_wle.hpp"

namespace nlsr
{
  class WaitingList
  {
    public:
      WaitingList()
        : waitingTable()
      {}
      
      std::list<WaitingListEntry>& getWaitingTable()
      {
        return waitingTable;
      }
      
      bool addtoWaitingList(std::string respCert, std::string waitee);
      std::pair<WaitingListEntry, bool> getWaitingListEntry(std::string respCert);
      bool removeFromWaitingList(std::string respCert);
      
    private:
      std::list<WaitingListEntry> waitingTable;
  };
  
  std::ostream& operator<<(std::ostream& os, WaitingList wl);
}

#endif
