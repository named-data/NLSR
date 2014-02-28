#ifndef NLSR_KM_HPP
#define NLSR_KM_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/data.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/security/validator.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>
#include <ndn-cpp-dev/util/random.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/security/signature-sha256-with-rsa.hpp>

#include <ndn-cpp-dev/security/sec-public-info-sqlite3.hpp>
#include <ndn-cpp-dev/security/sec-public-info-memory.hpp>
//TPM
#include <ndn-cpp-dev/security/sec-tpm-file.hpp>
#include <ndn-cpp-dev/security/sec-tpm-memory.hpp>

#ifdef NDN_CPP_HAVE_OSX_SECURITY
#include <ndn-cpp-dev/security/sec-tpm-osx.hpp>
#endif

#include <list>
#include "nlsr_conf_param.hpp"

namespace nlsr
{
    enum nlsrKeyType
    {
        KEY_TYPE_ROOT,
        KEY_TYPE_SITE,
        KEY_TYPE_OPERATOR,
        KEY_TYPE_ROUTER,
        KEY_TYPE_PROCESS
    };

    class KeyManager: public ndn::KeyChain, public ndn::Validator
    {
        typedef SecPublicInfo::Error InfoError;
        typedef SecTpm::Error TpmError;
    public:
        KeyManager()
            : certSeqNo(1)
        {
        }

        void initKeyManager(ConfParameter &cp);

        void
        checkPolicy (const ndn::Data& data,
                     int stepCount,
                     const ndn::OnDataValidated &onValidated,
                     const ndn::OnDataValidationFailed &onValidationFailed,
                     std::vector<ndn::shared_ptr<ndn::ValidationRequest> > &nextSteps)
        {}

        void
        checkPolicy (const ndn::Interest& interest,
                     int stepCount,
                     const ndn::OnInterestValidated &onValidated,
                     const ndn::OnInterestValidationFailed &onValidationFailed,
                     std::vector<ndn::shared_ptr<ndn::ValidationRequest> > &nextSteps)
        {}

        void signData(ndn::Data& data)
        {
            ndn::KeyChain::signByIdentity(data,routerIdentity);
            //ndn::SignatureSha256WithRsa signature(data.getSignature());
            //signature.setKeyLocator(routerCertName);
        }
        
        ndn::shared_ptr<ndn::IdentityCertificate>
        getCertificate(ndn::Name certificateName)
        {
            return ndn::KeyChain::getCertificate(routerCertName);
        }

        ndn::Name
        createIdentity(const ndn::Name identityName, const ndn::Name signee)
        {
            ndn::KeyChain::addIdentity(identityName);
            ndn::Name keyName;
            try
            {
                keyName = ndn::KeyChain::getDefaultKeyNameForIdentity(identityName);
            }
            catch(InfoError& e)
            {
                keyName = ndn::KeyChain::generateRSAKeyPairAsDefault(identityName, true);
            }
            ndn::shared_ptr<ndn::PublicKey> pubKey;
            try
            {
                pubKey = ndn::KeyChain::getPublicKey(keyName);
            }
            catch(InfoError& e)
            {
                //return ndn::shared_ptr<ndn::IdentityCertificate>()->getName();
                return identityName;
            }
            ndn::Name certName;
            try
            {
                certName = ndn::KeyChain::getDefaultCertificateNameForKey(keyName);
            }
            catch(InfoError& e)
            {
                ndn::shared_ptr<ndn::IdentityCertificate> certificate =
                    ndn::make_shared<ndn::IdentityCertificate>();
                ndn::Name certificateName = keyName.getPrefix(-1);
                certificateName.append("KEY").append(
                    keyName.get(-1)).append("ID-CERT").appendVersion();
                certificate->setName(certificateName);
                certificate->setNotBefore(ndn::getNow());
                certificate->setNotAfter(ndn::getNow() + 31536000 /* 1 year*/);
                certificate->setPublicKeyInfo(*pubKey);
                certificate->addSubjectDescription(
                    ndn::CertificateSubjectDescription("2.5.4.41",
                                                       keyName.toUri()));
                certificate->encode();
                try
                {
                    ndn::KeyChain::signByIdentity(*certificate,signee);
                }
                catch(InfoError& e)
                {
                    try
                    {
                        ndn::KeyChain::deleteIdentity(identityName);
                    }
                    catch(InfoError& e)
                    {
                    }
                    return identityName;
                }
                certName=certificate->getName();
            }
            return certName;
        }

        ndn::Name getRouterCertName();

        uint32_t getCertSeqNo();
        void setCerSeqNo(uint32_t csn);
        void initCertSeqFromFile(string certSeqFileDir);
        void writeCertSeqToFile();

    private:
        ndn::Name routerIdentity;
        ndn::Name routerCertName;
        ndn::Name routerKeyName;
        uint32_t certSeqNo;
        string certSeqFileNameWithPath;

    };
}

#endif
