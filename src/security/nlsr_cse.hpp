#ifndef NLSR_CERT_STORE_ENTRY_HPP
#define NLSR_CERT_STORE_ENTRY_HPP

#include <iostream>
#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>

namespace nlsr
{
  class NlsrCertificateStoreEntry
  {
  public:
    NlsrCertificateStoreEntry()
      : m_cert(ndn::make_shared<ndn::IdentityCertificate>())
      , m_certSeqNum(0)
      , m_isSignerVerified(false)
    {}

    NlsrCertificateStoreEntry(ndn::shared_ptr<ndn::IdentityCertificate> pcert
                              , uint32_t csn, bool isv)
      : m_cert(pcert)
      , m_certSeqNum(csn)
      , m_isSignerVerified(isv)
    {}

    ndn::shared_ptr<ndn::IdentityCertificate> getCert() const
    {
      return m_cert;
    }

    void setCert(ndn::shared_ptr<ndn::IdentityCertificate> pcert)
    {
      m_cert=pcert;
    }

    uint32_t getCertSeqNum() const
    {
      return m_certSeqNum;
    }

    void setCertSeqNum(uint32_t csn)
    {
      m_certSeqNum=csn;
    }

    bool getIsSignerVerified() const
    {
      return m_isSignerVerified;
    }

    void setIsSignerVerified(bool isv)
    {
      m_isSignerVerified=isv;
    }

  private:
    ndn::shared_ptr<ndn::IdentityCertificate> m_cert;
    uint32_t m_certSeqNum;
    bool m_isSignerVerified;
  };
  /* Debugging Purpose */
  std::ostream&
  operator <<(std::ostream& os, const NlsrCertificateStoreEntry& ncse);
}

#endif
