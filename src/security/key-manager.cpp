#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/encoding/block.hpp>
#include <ndn-cpp-dev/util/io.hpp>
#include <stdexcept>

#include "sequencing-manager.hpp"
#include "key-manager.hpp"
#include "nlsr.hpp"


namespace nlsr {
bool
KeyManager::initialize(ConfParameter& cp)
{
  initCertSeqFromFile(cp.getSeqFileDir());
  if (!loadAllCertificates(cp.getCertDir()))
  {
    return false;
  }
  m_nlsrRootKeyPrefix = cp.getRootKeyPrefix();
  string processIdentityName(cp.getRootKeyPrefix());
  processIdentityName += "/";
  processIdentityName += cp.getSiteName();
  processIdentityName += "/";
  processIdentityName += "R.Start";
  processIdentityName += "/";
  processIdentityName += cp.getRouterName();
  ndn::Name ri(processIdentityName);
  std::cout << "Router Identity: " << ri.toUri() << std::endl;
  m_routerIdentity = ri;
  processIdentityName += "/";
  processIdentityName += "nlsr";
  cout << "Proces Identity Name: " << processIdentityName << endl;
  ndn::Name identityName(processIdentityName);
  m_processIdentity = identityName;
  ndn::KeyChain::deleteIdentity(m_processIdentity);
  m_processCertName = ndn::KeyChain::createIdentity(m_processIdentity);
  cout << "Certificate Name: " << m_processCertName.toUri() << endl;
  m_processKeyName = m_processCertName.getPrefix(-2);
  cout << "Key Name: " << m_processKeyName.toUri() << endl;
  ndn::shared_ptr<ndn::IdentityCertificate> cert =
    getCertificate(m_processCertName);
  signByIdentity(*(cert), m_routerIdentity);
  m_certStore.addCertificate(cert, m_certSeqNo, true);
  m_certStore.print();
  return true;
}

bool
KeyManager::loadAllCertificates(std::string certDirPath)
{
  std::string filePath = certDirPath;
  if (filePath.empty())
  {
    SequencingManager sm;
    filePath = sm.getUserHomeDirectory();
    filePath += "/nlsrCertDir";
  }
  return loadCertificate(filePath + "/root.cert", KEY_TYPE_ROOT)
         && loadCertificate(filePath + "/site.cert", KEY_TYPE_SITE)
         && loadCertificate(filePath + "/operator.cert", KEY_TYPE_OPERATOR)
         && loadCertificate(filePath + "/router.cert", KEY_TYPE_ROUTER) ;
}

bool
KeyManager::loadCertificate(std::string inputFile, nlsrKeyType keyType)
{
  try
  {
    ndn::shared_ptr<ndn::IdentityCertificate> cert =
      ndn::io::load<ndn::IdentityCertificate>(inputFile, ndn::io::BASE_64);
    ndn::Name certName = cert->getName();
    switch (keyType)
      {
      case KEY_TYPE_ROOT:
        m_certStore.addCertificate(cert, 10, true);
        m_rootCertName = certName;
        std::cout << "Root Cert: " << m_rootCertName << std::endl;
        break;
      case KEY_TYPE_SITE:
        m_certStore.addCertificate(cert, 10, true);
        m_siteCertName = certName;
        std::cout << "Site Cert: " << m_siteCertName << std::endl;
        break;
      case KEY_TYPE_OPERATOR:
        m_certStore.addCertificate(cert, 10, true);
        m_opCertName = certName;
        std::cout << "Operator Cert: " << m_opCertName << std::endl;
        break;
      case KEY_TYPE_ROUTER:
        m_certStore.addCertificate(cert, m_certSeqNo, true);
        m_routerCertName = certName;
        std::cout << "Router Cert: " << m_routerCertName << std::endl;
        break;
      case KEY_TYPE_PROCESS:
        m_certStore.addCertificate(cert, m_certSeqNo, true);
        m_processCertName = certName;
        std::cout << "Process Cert: " << m_processCertName << std::endl;
        break;
      default:
        break;
      }
    return true;
  }
  catch (std::exception& e)
  {
    return false;
  }
  return false;
}

ndn::Name
KeyManager::getProcessCertName()
{
  return m_processCertName;
}

ndn::Name
KeyManager::getRouterCertName()
{
  return m_routerCertName;
}

ndn::Name
KeyManager::getOperatorCertName()
{
  return m_opCertName;
}

ndn::Name
KeyManager::getSiteCertName()
{
  return m_siteCertName;
}

ndn::Name
KeyManager::getRootCertName()
{
  return m_rootCertName;
}

uint32_t
KeyManager::getCertSeqNo()
{
  return m_certSeqNo;
}

void
KeyManager::setCerSeqNo(uint32_t csn)
{
  m_certSeqNo = csn;
}

void
KeyManager::initCertSeqFromFile(string certSeqFileDir)
{
  m_certSeqFileNameWithPath = certSeqFileDir;
  if (m_certSeqFileNameWithPath.empty())
  {
    SequencingManager sm;
    m_certSeqFileNameWithPath = sm.getUserHomeDirectory();
  }
  m_certSeqFileNameWithPath += "/nlsrCertSeqNo.txt";
  cout << "Key Seq File Name: " << m_certSeqFileNameWithPath << endl;
  std::ifstream inputFile(m_certSeqFileNameWithPath.c_str(), ios::binary);
  if (inputFile.good())
  {
    inputFile >> m_certSeqNo;
    m_certSeqNo++;
  }
  else
  {
    m_certSeqNo = 1;
  }
  writeCertSeqToFile();
}

void
KeyManager::writeCertSeqToFile()
{
  std::ofstream outputFile(m_certSeqFileNameWithPath.c_str(), ios::binary);
  outputFile << m_certSeqNo;
  outputFile.close();
}

bool
KeyManager::isNewCertificate(std::string certName, int checkSeqNum)
{
  return m_certStore.isCertificateNewInStore(certName, checkSeqNum);
}

std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
KeyManager::getCertificateFromStore(const std::string certName, int checkSeqNum)
{
  return m_certStore.getCertificateFromStore(certName, checkSeqNum);
}

std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
KeyManager::getCertificateFromStore(const std::string certName)
{
  return m_certStore.getCertificateFromStore(certName);
}

bool
KeyManager::addCertificate(ndn::shared_ptr<ndn::IdentityCertificate> pcert
                           , uint32_t csn, bool isv)
{
  return m_certStore.addCertificate(pcert, csn, isv);
}

std::pair<uint32_t, bool>
KeyManager::getCertificateSeqNum(std::string certName)
{
  return m_certStore.getCertificateSeqNum(certName);
}

nlsrKeyType
KeyManager::getKeyTypeFromName(const std::string keyName)
{
  Tokenizer nt(keyName, "/");
  std::string KEY("KEY");
  std::string opHandle("O.Start");
  std::string routerHandle("R.Start");
  std::string processHandle("nlsr");
  if (nt.getTokenString(0, nt.getTokenPosition(KEY) - 1) == m_nlsrRootKeyPrefix)
  {
    return KEY_TYPE_ROOT;
  }
  else if (nt.doesTokenExist(opHandle))
  {
    return KEY_TYPE_OPERATOR;
  }
  else if (nt.doesTokenExist(routerHandle) &&
           nt.doesTokenExist(processHandle))
  {
    return KEY_TYPE_PROCESS;
  }
  else if (nt.doesTokenExist(routerHandle) &&
           !nt.doesTokenExist(processHandle))
  {
    return KEY_TYPE_ROUTER;
  }
  else
  {
    return KEY_TYPE_SITE;
  }
}

std::string
KeyManager::getRouterName(const std::string name)
{
  std::string routerName;
  std::string rkp(m_nlsrRootKeyPrefix);
  Tokenizer ntRkp(rkp, "/");
  Tokenizer nt(name, "/");
  std::string KEY("KEY");
  std::string opHandle("O.Start");
  std::string routerHandle("R.Start");
  std::string processHandle("nlsr");
  std::string infoHandle("info");
  std::string lsaHandle("LSA");
  if (nt.doesTokenExist(processHandle) && nt.doesTokenExist(routerHandle))
  {
    routerName = "/ndn" +
                 nt.getTokenString(ntRkp.getTokenNumber(),
                                   nt.getTokenPosition(routerHandle) - 1) +
                 nt.getTokenString(nt.getTokenPosition(routerHandle) + 1,
                                   nt.getTokenPosition(processHandle) - 1);
  }
  else if (nt.doesTokenExist(routerHandle) && !nt.doesTokenExist(infoHandle)
           && !nt.doesTokenExist(lsaHandle))
  {
    routerName = "/ndn" +
                 nt.getTokenString(ntRkp.getTokenNumber(),
                                   nt.getTokenPosition(routerHandle) - 1) +
                 nt.getTokenString(nt.getTokenPosition(routerHandle) + 1,
                                   nt.getTokenPosition(KEY) - 1);
  }
  else
  {
    if (nt.doesTokenExist(infoHandle))
    {
      routerName = nt.getTokenString(0, nt.getTokenPosition(infoHandle) - 1);
    }
    else
    {
      routerName = nt.getTokenString(nt.getTokenPosition(lsaHandle) + 1,
                                     nt.getTokenNumber() - 4);
    }
  }
  return routerName;
}

std::string
KeyManager::getSiteName(const std::string name)
{
  std::string siteName;
  std::string routerName;
  std::string rkp(m_nlsrRootKeyPrefix);
  Tokenizer ntRkp(rkp, "/");
  Tokenizer nt(name, "/");
  std::string KEY("KEY");
  std::string opHandle("O.Start");
  std::string routerHandle("R.Start");
  if (nt.doesTokenExist(routerHandle))
  {
    siteName = "/ndn" + nt.getTokenString(ntRkp.getTokenNumber(),
                                          nt.getTokenPosition(routerHandle) - 1);
  }
  else if (nt.doesTokenExist(opHandle))
  {
    siteName = "/ndn" + nt.getTokenString(ntRkp.getTokenNumber(),
                                          nt.getTokenPosition(opHandle) - 1);
  }
  else
  {
    siteName = "/ndn" + nt.getTokenString(ntRkp.getTokenNumber(),
                                          nt.getTokenPosition(KEY) - 1);
  }
  return siteName;
}

std::string
KeyManager::getRootName(const std::string name)
{
  std::string rName;
  Tokenizer nt(name, "/");
  std::string rkp(m_nlsrRootKeyPrefix);
  Tokenizer ntRkp(rkp, "/");
  rName = nt.getTokenString(0, ntRkp.getTokenNumber() - 1);
  return rName;
}


bool
KeyManager::verifyCertPacket(Nlsr& pnlsr, ndn::IdentityCertificate& packet)
{
  std::cout << "KeyManager::verifyCertPacket Called" << std::endl;
  ndn::SignatureSha256WithRsa signature(packet.getSignature());
  std::string signingCertName = signature.getKeyLocator().getName().toUri();
  std::string packetName = packet.getName().toUri();

  std::cout << "Packet Name: " << packetName << std::endl;
  std::cout << "Signee Name: " << signingCertName << std::endl;

  int paketCertType = getKeyTypeFromName(packetName);
  int signingCertType = getKeyTypeFromName(signingCertName);

  if (signingCertType > paketCertType)  //lower level Cert can not sign
  {
    //upper level Cert
    return false;
  }

  if ((signingCertType == paketCertType) && (paketCertType != KEY_TYPE_ROOT))
  {
    return false;
  }

  std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool> signee =
    m_certStore.getCertificateFromStore(signingCertName);

  if (signee.second)
  {
    switch (paketCertType)
      {
      case KEY_TYPE_ROOT:
        return ((getRootName(packetName) == m_nlsrRootKeyPrefix) &&
                verifySignature(packet, signee.first->getPublicKeyInfo()));
        break;
      case KEY_TYPE_SITE:
        return ((getRootName(packetName) == getRootName(signingCertName)) &&
                verifySignature(packet, signee.first->getPublicKeyInfo()) &&
                m_certStore.getCertificateIsVerified(signingCertName));
        break;
      case KEY_TYPE_OPERATOR:
        return ((getSiteName(packetName) == getSiteName(signingCertName)) &&
                verifySignature(packet, signee.first->getPublicKeyInfo()) &&
                m_certStore.getCertificateIsVerified(signingCertName));
        break;
      case KEY_TYPE_ROUTER:
        return ((getSiteName(packetName) == getSiteName(signingCertName)) &&
                verifySignature(packet, signee.first->getPublicKeyInfo()) &&
                m_certStore.getCertificateIsVerified(signingCertName));
        break;
      case KEY_TYPE_PROCESS:
        return ((getRouterName(packetName) == getRouterName(signingCertName)) &&
                verifySignature(packet, signee.first->getPublicKeyInfo()) &&
                m_certStore.getCertificateIsVerified(signingCertName));
        break;
      }
  }
  else
  {
    std::cout << "Certificate Not Found in store. Sending Interest" << std::endl;
    pnlsr.getIm().expressInterest(pnlsr, signingCertName, 3,
                                  pnlsr.getConfParameter().getInterestResendTime());
    return false;
  }
  return false;
}

}//namespace nlsr



