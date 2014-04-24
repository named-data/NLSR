#include <iostream>
#include <cstdlib>

#include <ndn-cpp-dev/security/signature-sha256-with-rsa.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/util/io.hpp>

#include "nlsr.hpp"
#include "data-manager.hpp"
#include "utility/tokenizer.hpp"
#include "lsdb.hpp"
// #include "security/key-manager.hpp"

namespace nlsr {

using namespace std;
using namespace ndn;

void
DataManager::processContent(const ndn::Interest& interest,
                            const ndn::Data& data, InterestManager& im)
{
  std::cout << "I: " << interest.toUri() << std::endl;
  string dataName(data.getName().toUri());
  Tokenizer nt(dataName, "/");
  std::string chkString("keys");
  if (nt.doesTokenExist(chkString))
  {
    processContentKeys(data);
  }
  else
  {
    // if (m_nlsr.getKeyManager().verify(data))
    {
      std::cout << "Verified Data Content" << std::endl;
      chkString = "info";
      if (nt.doesTokenExist(chkString))
      {
        string dataContent((char*)data.getContent().value());
        processContentInfo(dataName, dataContent);
      }
      chkString = "LSA";
      if (nt.doesTokenExist(chkString))
      {
        string dataContent((char*)data.getContent().value());
        processContentLsa(dataName, dataContent);
      }
    }
    // else
    // {
    //   std::cout << "Unverified Data Content. Discarded" << std::endl;
    // }
  }
}

void
DataManager::processContentInfo(const string& dataName,
                                string& dataContent)
{
  Tokenizer nt(dataName, "/");
  string chkString("info");
  string neighbor = nt.getTokenString(0, nt.getTokenPosition(chkString) - 1);
  int oldStatus = m_nlsr.getAdl().getStatusOfNeighbor(neighbor);
  int infoIntTimedOutCount = m_nlsr.getAdl().getTimedOutInterestCount(neighbor);
  //debugging purpose start
  std::cout << "Before Updates: " << std::endl;
  std::cout << "Neighbor : " << neighbor << std::endl;
  std::cout << "Status: " << oldStatus << std::endl;
  std::cout << "Info Interest Timed out: " << infoIntTimedOutCount << std::endl;
  //debugging purpose end
  m_nlsr.getAdl().setStatusOfNeighbor(neighbor, 1);
  m_nlsr.getAdl().setTimedOutInterestCount(neighbor, 0);
  int newStatus = m_nlsr.getAdl().getStatusOfNeighbor(neighbor);
  infoIntTimedOutCount = m_nlsr.getAdl().getTimedOutInterestCount(neighbor);
  //debugging purpose
  std::cout << "After Updates: " << std::endl;
  std::cout << "Neighbor : " << neighbor << std::endl;
  std::cout << "Status: " << newStatus << std::endl;
  std::cout << "Info Interest Timed out: " << infoIntTimedOutCount << std::endl;
  //debugging purpose end
  if ((oldStatus - newStatus) != 0)  // change in Adjacency list
  {
    m_nlsr.incrementAdjBuildCount();
    /* Need to schedule event for Adjacency LSA building */
    if (m_nlsr.getIsBuildAdjLsaSheduled() == 0)
    {
      m_nlsr.setIsBuildAdjLsaSheduled(1);
      // event here
      m_nlsr.getScheduler().scheduleEvent(ndn::time::seconds(5),
                                          ndn::bind(&Lsdb::scheduledAdjLsaBuild, m_nlsr.getLsdb(),
                                                    boost::ref(m_nlsr)));
    }
  }
}

void
DataManager::processContentLsa(const string& dataName, string& dataContent)
{
  Tokenizer nt(dataName, "/");
  string chkString("LSA");
  string origRouter = nt.getTokenString(nt.getTokenPosition(chkString) + 1,
                                        nt.getTokenNumber() - 4);
  string lsTypeString = nt.getToken(nt.getTokenNumber() - 3);
  string lsSeNoString = nt.getToken(nt.getTokenNumber() - 2);
  uint32_t interestedLsSeqNo;
  try
  {
    interestedLsSeqNo = boost::lexical_cast<uint32_t>(lsSeNoString);
  }
  catch (std::exception& e)
  {
    return;
  }
  if (lsTypeString == "1")  //Name Lsa
  {
    processContentNameLsa(origRouter + "/" + lsTypeString,
                          interestedLsSeqNo, dataContent);
  }
  else if (lsTypeString == "2")  //Adj Lsa
  {
    processContentAdjLsa(origRouter + "/" + lsTypeString,
                         interestedLsSeqNo, dataContent);
  }
  else if (lsTypeString == "3")  //Cor Lsa
  {
    processContentCorLsa(origRouter + "/" + lsTypeString,
                         interestedLsSeqNo, dataContent);
  }
  else
  {
    cout << "Unrecognized LSA Type :(" << endl;
  }
}

void
DataManager::processContentNameLsa(const string& lsaKey,
                                   uint32_t lsSeqNo, string& dataContent)
{
  if (m_nlsr.getLsdb().isNameLsaNew(lsaKey, lsSeqNo))
  {
    NameLsa nameLsa;
    if (nameLsa.initializeFromContent(dataContent))
    {
      m_nlsr.getLsdb().installNameLsa(m_nlsr, nameLsa);
    }
    else
    {
      std::cout << "LSA data decoding error :(" << std::endl;
    }
  }
}

void
DataManager::processContentAdjLsa(const string& lsaKey,
                                  uint32_t lsSeqNo, string& dataContent)
{
  if (m_nlsr.getLsdb().isAdjLsaNew(lsaKey, lsSeqNo))
  {
    AdjLsa adjLsa;
    if (adjLsa.initializeFromContent(dataContent))
    {
      m_nlsr.getLsdb().installAdjLsa(m_nlsr, adjLsa);
    }
    else
    {
      std::cout << "LSA data decoding error :(" << std::endl;
    }
  }
}

void
DataManager::processContentCorLsa(const string& lsaKey,
                                  uint32_t lsSeqNo, string& dataContent)
{
  if (m_nlsr.getLsdb().isCoordinateLsaNew(lsaKey, lsSeqNo))
  {
    CoordinateLsa corLsa;
    if (corLsa.initializeFromContent(dataContent))
    {
      m_nlsr.getLsdb().installCoordinateLsa(m_nlsr, corLsa);
    }
    else
    {
      std::cout << "LSA data decoding error :(" << std::endl;
    }
  }
}

void
DataManager::processContentKeys(const ndn::Data& data)
{
  std::cout << " processContentKeys called " << std::endl;
  ndn::shared_ptr<ndn::IdentityCertificate> cert =
    ndn::make_shared<ndn::IdentityCertificate>();
  cert->wireDecode(data.getContent().blockFromValue());
  std::cout << *(cert) << std::endl;
  std::string dataName = data.getName().toUri();
  Tokenizer nt(dataName, "/");
  std::string certName = nt.getTokenString(0, nt.getTokenNumber() - 3);
  uint32_t seqNum = boost::lexical_cast<uint32_t>(nt.getToken(
                                                    nt.getTokenNumber() - 2));
  std::cout << "Cert Name: " << certName << " Seq Num: " << seqNum << std::endl;
  // if (m_nlsr.getKeyManager().verify(m_nlsr, *(cert)))
  // {
  //   m_nlsr.getKeyManager().addCertificate(cert, seqNum, true);
  // }
  // else
  // {
  //   m_nlsr.getKeyManager().addCertificate(cert, seqNum, false);
  // }
  // m_nlsr.getKeyManager().printCertStore();
}
}//namespace nlsr
