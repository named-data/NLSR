#ifndef NLSR_CERT_STORE_HPP
#define NLSR_CERT_STORE_HPP

#include<list>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include "nlsr_cse.hpp"

namespace nlsr
{
  class NlsrCertificateStore
  {
  public:
    NlsrCertificateStore()
    {}

    bool addCertificate(NlsrCertificateStoreEntry & ncse);
    bool addCertificate(ndn::shared_ptr<ndn::IdentityCertificate> pcert
                        , uint32_t csn, bool isv);
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
    getCertificateFromStore(const std::string certName);
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
    getCertificateFromStore(const std::string certName, int checkSeqNum);
    bool removeCertificateFromStroe(const std::string certName);
    bool isCertificateNewInStore(const std::string certName, int checkSeqNo);
    void printCertStore();
  private:
    std::list<NlsrCertificateStoreEntry> certTable;
  };
}

#endif
