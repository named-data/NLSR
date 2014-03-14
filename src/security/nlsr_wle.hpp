#ifndef NLSR_WLE_HPP
#define NLSR_WLE_HPP

#include <list>
#include <iostream>

namespace nlsr
{
  class WaitingListEntry
  {
    public:
      WaitingListEntry()
        : responsibleCert()
        , waitingCerts()
      {}
      
      WaitingListEntry(std::string resCert)
        : responsibleCert(resCert)
        , waitingCerts()
      {}
      
      std::string getResponsibleCert() const
      {
        return responsibleCert;
      }
      
      void setResponsibleCert(std::string resCert)
      {
        responsibleCert=resCert;
      }
      
      std::list<std::string> getWaitingCerts() const
      {
        return waitingCerts;
      }
      
      bool addWaitee(std::string waiteeName);
      
    private:
      std::string responsibleCert;
      std::list<std::string> waitingCerts;
  };
  
  std::ostream& operator<<(std::ostream& os, const WaitingListEntry& we);
} //end name space

#endif
