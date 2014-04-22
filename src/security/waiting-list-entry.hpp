#ifndef NLSR_WLE_HPP
#define NLSR_WLE_HPP

#include <list>
#include <iostream>

namespace nlsr {
class WaitingListEntry
{
public:
  WaitingListEntry()
    : m_responsibleCert()
    , m_waitingCerts()
  {}

  WaitingListEntry(std::string resCert)
    : m_responsibleCert(resCert)
    , m_waitingCerts()
  {}

  std::string
  getResponsibleCert() const
  {
    return m_responsibleCert;
  }

  void
  setResponsibleCert(std::string resCert)
  {
    m_responsibleCert = resCert;
  }

  std::list<std::string>
  getWaitingCerts() const
  {
    return m_waitingCerts;
  }

  bool
  addWaitee(std::string waiteeName);

private:
  std::string m_responsibleCert;
  std::list<std::string> m_waitingCerts;
};

std::ostream&
operator<<(std::ostream& os, const WaitingListEntry& we);
} //end name space

#endif //NLSR_WLE_HPP
