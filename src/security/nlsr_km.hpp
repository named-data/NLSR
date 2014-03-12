#ifndef NLSR_KM_HPP
#define NLSR_KM_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/data.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/security/validator.hpp>
#include <ndn-cpp-dev/util/random.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <list>
#include "nlsr_conf_param.hpp"
#include "nlsr_cert_store.hpp"
#include "utility/nlsr_tokenizer.hpp"

namespace nlsr
{
  enum nlsrKeyType
  {
    KEY_TYPE_ROOT,
    KEY_TYPE_SITE,
    KEY_TYPE_OPERATOR,
    KEY_TYPE_ROUTER,
    KEY_TYPE_PROCESS,
    KEY_TYPE_UNKNOWN
  };

  enum nlsrContentType
  {
    CONTENT_TYPE_DATA,
    CONTENT_TYPE_CERT
  };

  class KeyManager: public ndn::KeyChain, public ndn::Validator
  {
    typedef SecPublicInfo::Error InfoError;
    typedef SecTpm::Error TpmError;
  public:
    KeyManager()
      : certSeqNo(1)
      , certStore()
      , nlsrRootKeyPrefix()
    {
    }

    bool initKeyManager(ConfParameter &cp);



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
      ndn::KeyChain::signByIdentity(data,processIdentity);
    }

    template<typename T>
    void signByIdentity(T& packet, ndn::Name signeeIdentity)
    {
      ndn::KeyChain::signByIdentity(packet,signeeIdentity);
    }

    ndn::shared_ptr<ndn::IdentityCertificate>
    getCertificate(ndn::Name certificateName)
    {
      return ndn::KeyChain::getCertificate(certificateName);
    }

    ndn::shared_ptr<ndn::IdentityCertificate>
    getCertificate()
    {
      return getCertificate(processCertName);
    }

    ndn::Name
    createIdentity(const ndn::Name identityName)
    {
      return ndn::KeyChain::createIdentity(identityName);
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

    void printCertStore()
    {
      certStore.printCertStore();
    }

  private:
    bool
    verifyDataPacket(ndn::Data packet)
    {
      std::cout<<"KeyManager::verifyDataPacket Called"<<std::endl;
      ndn::SignatureSha256WithRsa signature(packet.getSignature());
      std::string signingCertName=signature.getKeyLocator().getName().toUri();
      std::string packetName=packet.getName().toUri();
      std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool> signee=
        certStore.getCertificateFromStore(signingCertName);
      if( signee.second )
      {
        std::string routerNameFromPacketName=getRouterName(packetName);
        std::string routerNameFromCertName=getRouterName(signingCertName);
        return ( (routerNameFromPacketName== routerNameFromCertName) &&
                 verifySignature(packet, signee.first->getPublicKeyInfo()));
      }
      return false;
    }

    bool
    verifyCertPacket(ndn::IdentityCertificate packet)
    {
      std::cout<<"KeyManager::verifyCertPacket Called"<<std::endl;
      return true;
    }


  public:
    template<typename T>
    bool
    verify(T& packet )
    {
      std::cout<<"KeyManager::verify Called"<<std::endl;
      std::string packetName=packet.getName().toUri();
      nlsrTokenizer nt(packetName,"/");
      std::string keyHandle("keys");
      if ( nt.doesTokenExist(keyHandle) )
      {
        return verifyCertPacket(packet);
      }
      else
      {
        return verifyDataPacket(packet);
      }
      return false;
    }

    ndn::Name getProcessCertName();
    ndn::Name getRouterCertName();
    ndn::Name getOperatorCertName();
    ndn::Name getSiteCertName();
    ndn::Name getRootCertName();

    uint32_t getCertSeqNo();
    void setCerSeqNo(uint32_t csn);
    void initCertSeqFromFile(string certSeqFileDir);
    void writeCertSeqToFile();
    bool isNewCertificate(std::string certName, int checkSeqNum);
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
    getCertificateFromStore(const std::string certName, int checkSeqNum);
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
    getCertificateFromStore(const std::string certName);
    bool addCertificate(ndn::shared_ptr<ndn::IdentityCertificate> pcert
                        , uint32_t csn, bool isv);


  private:
    bool loadAllCertificates(std::string certDirPath);
    bool loadCertificate(std::string inputFile, nlsrKeyType keyType);
    nlsrKeyType getKeyTypeFromName(const std::string keyName);
    std::string getRouterName(const std::string name);
    std::string getSiteName(const std::string name);

  private:
    ndn::Name processIdentity;
    ndn::Name routerIdentity;
    ndn::Name processCertName;
    ndn::Name routerCertName;
    ndn::Name opCertName;
    ndn::Name siteCertName;
    ndn::Name rootCertName;
    ndn::Name processKeyName;
    uint32_t certSeqNo;
    string certSeqFileNameWithPath;
    string nlsrRootKeyPrefix;
    NlsrCertificateStore certStore;

  };
}

#endif
