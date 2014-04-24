#include <ndn-cpp-dev/security/signature-sha256-with-rsa.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include "certificate-store.hpp"
#include "waiting-list-entry.hpp"
#include "key-manager.hpp"


namespace nlsr {
static bool
nlsrCertificateStoreEntryCompare(CertificateStoreEntry& ncse1,
                                 CertificateStoreEntry& ncse2)

{
  int sizeDiff = ncse1.getCert()->getName().size() -
                 ncse2.getCert()->getName().size();
  return (ncse2.getCert()->getName().isPrefixOf(ncse1.getCert()->getName()) &&
          (sizeDiff <= 1 && sizeDiff >= 0));
}

static bool
nlsrCertificateStoreEntryCompareByName(CertificateStoreEntry& ncse1,
                                       std::string compCertName)

{
  ndn::Name ccn(compCertName);
  int sizeDiff = ncse1.getCert()->getName().size() - ccn.size();
  return (ccn.isPrefixOf(ncse1.getCert()->getName()) &&
          (sizeDiff <= 1 && sizeDiff >= 0));
}

void
CertificateStore::updateWaitingList(std::string respCertName)
{
  ndn::Name tmpName(respCertName);
  respCertName = tmpName.getPrefix(-1).toUri();
  std::pair<WaitingListEntry, bool> chkWle =
    m_waitingList.getWaitingListEntry(respCertName);
  if (chkWle.second)
  {
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool> sc =
      getCertificateFromStore(respCertName);
    std::list<std::string> waitees = (chkWle.first).getWaitingCerts();
    for (std::list<std::string>::iterator it = waitees.begin();
         it != waitees.end(); ++it)
    {
      KeyManager km;
      std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool> wc =
        getCertificateFromStore(*(it));
      if (wc.second && sc.second)
      {
        if (km.verifySignature(*(wc.first), sc.first->getPublicKeyInfo()))
        {
          //1. Update Certificate Store
          setCertificateIsVerified(*(it), true);
          //2. Call updateWaitingList for waitee ( *(it) )
          updateWaitingList(*(it));
        }
      }
    }
  }
  //remove that entry from waiting list
  m_waitingList.remove(respCertName);
}

void
CertificateStore::updateWaitingList(CertificateStoreEntry& ncse)
{
  if (ncse.getIsSignerVerified())
  {
    updateWaitingList(ncse.getCert()->getName().toUri());
  }
  else
  {
    ndn::SignatureSha256WithRsa signature(ncse.getCert()->getSignature());
    m_waitingList.add(signature.getKeyLocator().getName().toUri(),
                      ncse.getCert()->getName().toUri());
  }
}

bool
CertificateStore::addCertificate(CertificateStoreEntry& ncse)
{
  std::list<CertificateStoreEntry>::iterator it =
    std::find_if(m_certTable.begin(), m_certTable.end(),
                 bind(&nlsrCertificateStoreEntryCompare, _1, ncse));
  if (it == m_certTable.end())
  {
    m_certTable.push_back(ncse);
    updateWaitingList(ncse);
    return true;
  }
  else if (it !=  m_certTable.end())
  {
    if ((*it).getCertSeqNum() < ncse.getCertSeqNum())
    {
      m_certTable.erase(it);
      m_certTable.push_back(ncse);
      updateWaitingList(ncse);
      return true;
    }
  }
  return false;
}

bool
CertificateStore::addCertificate(
  ndn::shared_ptr<ndn::IdentityCertificate> pcert, uint32_t csn, bool isv)
{
  CertificateStoreEntry ncse(pcert, csn, isv);
  return addCertificate(ncse);
}

std::pair<uint32_t, bool>
CertificateStore::getCertificateSeqNum(std::string certName)
{
  std::list<CertificateStoreEntry>::iterator it =
    std::find_if(m_certTable.begin(), m_certTable.end(),
                 bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
  if (it == m_certTable.end())
  {
    return std::make_pair(0, false);
  }
  return std::make_pair((*it).getCertSeqNum(), true);
}



void
CertificateStore::setCertificateIsVerified(std::string certName,
                                           bool isVerified)
{
  std::list<CertificateStoreEntry>::iterator it =
    std::find_if(m_certTable.begin(), m_certTable.end(),
                 bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
  if (it != m_certTable.end())
  {
    it->setIsSignerVerified(true);
  }
}

bool
CertificateStore::getCertificateIsVerified(std::string certName)
{
  std::list<CertificateStoreEntry>::iterator it =
    std::find_if(m_certTable.begin(), m_certTable.end(),
                 bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
  if (it != m_certTable.end())
  {
    return it->getIsSignerVerified();
  }
  return false;
}

std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
CertificateStore::getCertificateFromStore(const std::string certName)
{
  std::list<CertificateStoreEntry>::iterator it =
    std::find_if(m_certTable.begin(), m_certTable.end(),
                 bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
  if (it == m_certTable.end())
  {
    ndn::shared_ptr<ndn::IdentityCertificate> cert =
      ndn::make_shared<ndn::IdentityCertificate>();
    return std::make_pair(cert, false);
  }
  return std::make_pair((*it).getCert(), true);
}

std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
CertificateStore::getCertificateFromStore(
  const std::string certName, uint64_t checkSeqNum)
{
  std::list<CertificateStoreEntry>::iterator it =
    std::find_if(m_certTable.begin(), m_certTable.end(),
                 bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
  if (it == m_certTable.end())
  {
    ndn::shared_ptr<ndn::IdentityCertificate> cert =
      ndn::make_shared<ndn::IdentityCertificate>();
    return std::make_pair(cert, false);
  }
  else
  {
    if ((*it).getCertSeqNum() == checkSeqNum)
    {
      return std::make_pair((*it).getCert(), true);
    }
  }
  return std::make_pair((*it).getCert(), false);
}

bool
CertificateStore::isCertificateNewInStore(const std::string certName,
                                          int checkSeqNo)
{
  std::list<CertificateStoreEntry>::iterator it =
    std::find_if(m_certTable.begin(), m_certTable.end(),
                 bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
  if (it != m_certTable.end())
  {
    return (*it).getCertSeqNum() < checkSeqNo ;
  }
  return true;
}

bool
CertificateStore::removeCertificateFromStroe(const std::string certName)
{
  std::list<CertificateStoreEntry>::iterator it =
    std::find_if(m_certTable.begin(), m_certTable.end(),
                 bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
  if (it != m_certTable.end())
  {
    m_certTable.erase(it);
    return true;
  }
  return false;
}

void
CertificateStore::print()
{
  std::list<CertificateStoreEntry>::iterator it;
  for (it = m_certTable.begin(); it != m_certTable.end(); ++it)
  {
    std::cout << (*it) << std::endl;
  }
  std::cout << m_waitingList << std::endl;
}

} //namespace nlsr
