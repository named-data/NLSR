#ifndef NLSR_CERT_STORE_HPP
#define NLSR_CERT_STORE_HPP

#include <list>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include "certificate-store-entry.hpp"
#include "waiting-list.hpp"

namespace nlsr {
class CertificateStore
{
public:
  CertificateStore()
    : m_certTable()
    , m_waitingList()
  {}

  bool
  addCertificate(CertificateStoreEntry& ncse);

  bool
  addCertificate(ndn::shared_ptr<ndn::IdentityCertificate> pcert
                 , uint32_t csn, bool isv);

  std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
  getCertificateFromStore(const std::string certName);

  std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
  getCertificateFromStore(const std::string certName, uint64_t checkSeqNum);

  bool
  removeCertificateFromStroe(const std::string certName);

  bool
  isCertificateNewInStore(const std::string certName, int checkSeqNo);

  std::pair<uint32_t, bool>
  getCertificateSeqNum(std::string certName);

  void
  print();

  void
  setCertificateIsVerified(std::string certName, bool isVerified);

  bool
  getCertificateIsVerified(std::string certName);

private:
  void
  updateWaitingList(CertificateStoreEntry& ncse);

  void
  updateWaitingList(std::string respCertName);

private:
  std::list<CertificateStoreEntry> m_certTable;
  WaitingList m_waitingList;
};

} //namespace nlsr

#endif // NLSR_CERT_STORE_HPP
