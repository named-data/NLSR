#ifndef NLSR_CERT_STORE_HPP
#define NLSR_CERT_STORE_HPP

#include<list>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include "nlsr_cse.hpp"
#include "nlsr_wl.hpp"

namespace nlsr
{
  class NlsrCertificateStore
  {
  public:
    NlsrCertificateStore()
        : m_certTable()
        , m_waitingList()
    {}

    bool addCertificate(NlsrCertificateStoreEntry& ncse);
    bool addCertificate(ndn::shared_ptr<ndn::IdentityCertificate> pcert
                        , uint32_t csn, bool isv);
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
                            getCertificateFromStore(const std::string certName);
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
           getCertificateFromStore(const std::string certName, int checkSeqNum);
    bool removeCertificateFromStroe(const std::string certName);
    bool isCertificateNewInStore(const std::string certName, int checkSeqNo);
    std::pair<uint32_t, bool> getCertificateSeqNum(std::string certName);
    void print();
    void setCertificateIsVerified(std::string certName, bool isVerified);
    bool getCertificateIsVerified(std::string certName);
  private:
    void updateWaitingList(NlsrCertificateStoreEntry& ncse);
    void updateWaitingList(std::string respCertName);
    
  private:
    std::list<NlsrCertificateStoreEntry> m_certTable;
    WaitingList m_waitingList;
  };
}

#endif
