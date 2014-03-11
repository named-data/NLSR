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
            : cert(ndn::make_shared<ndn::IdentityCertificate>())
            , certSeqNum(0)
            , isSignerVerified(false)
        {}
        
        NlsrCertificateStoreEntry(ndn::shared_ptr<ndn::IdentityCertificate> pcert
                                                       , uint32_t csn, bool isv)
            : cert(pcert)
            , certSeqNum(csn)
            , isSignerVerified(isv)
        {}
        
        ndn::shared_ptr<ndn::IdentityCertificate> getCert() const
        {
            return cert;
        }
        
        void setCert(ndn::shared_ptr<ndn::IdentityCertificate> pcert)
        {
            cert=pcert;
        }
        
        uint32_t getCertSeqNum() const
        {
            return certSeqNum;
        }
        
        void setCertSeqNum(uint32_t csn)
        {
            certSeqNum=csn;
        }
        
        bool getIsSignerVerified() const
        {
            return isSignerVerified;
        }
        
        void setIsSignerVerified(bool isv)
        {
            isSignerVerified=isv;
        }
        
      private:
        ndn::shared_ptr<ndn::IdentityCertificate> cert;
        uint32_t certSeqNum;
        bool isSignerVerified;
    };
    /* Debugging Purpose */
    std::ostream&
    operator <<(std::ostream& os, const NlsrCertificateStoreEntry& ncse);
}

#endif
