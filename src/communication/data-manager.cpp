#include <iostream>
#include <cstdlib>

#include <ndn-cpp-dev/security/signature-sha256-with-rsa.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/util/io.hpp>

#include "nlsr.hpp"
#include "data-manager.hpp"
#include "utility/tokenizer.hpp"
#include "lsdb.hpp"
#include "security/key-manager.hpp"

namespace nlsr {

using namespace std;
using namespace ndn;

void
DataManager::processContent(Nlsr& pnlsr, const ndn::Interest& interest,
                            const ndn::Data& data, InterestManager& im)
{
  std::cout << "I: " << interest.toUri() << std::endl;
  string dataName(data.getName().toUri());
  Tokenizer nt(dataName, "/");
  std::string chkString("keys");
  if (nt.doesTokenExist(chkString))
  {
    processContentKeys(pnlsr, data);
  }
  else
  {
    if (pnlsr.getKeyManager().verify(data))
    {
      std::cout << "Verified Data Content" << std::endl;
      chkString = "info";
      if (nt.doesTokenExist(chkString))
      {
        string dataContent((char*)data.getContent().value());
        processContentInfo(pnlsr, dataName, dataContent);
      }
      chkString = "LSA";
      if (nt.doesTokenExist(chkString))
      {
        string dataContent((char*)data.getContent().value());
        processContentLsa(pnlsr, dataName, dataContent);
      }
    }
    else
    {
      std::cout << "Unverified Data Content. Discarded" << std::endl;
    }
  }
}

void
DataManager::processContentInfo(Nlsr& pnlsr, string& dataName,
                                string& dataContent)
{
  Tokenizer nt(dataName, "/");
  string chkString("info");
  string neighbor = nt.getTokenString(0, nt.getTokenPosition(chkString) - 1);
  int oldStatus = pnlsr.getAdl().getStatusOfNeighbor(neighbor);
  int infoIntTimedOutCount = pnlsr.getAdl().getTimedOutInterestCount(neighbor);
  //debugging purpose start
  std::cout << "Before Updates: " << std::endl;
  std::cout << "Neighbor : " << neighbor << std::endl;
  std::cout << "Status: " << oldStatus << std::endl;
  std::cout << "Info Interest Timed out: " << infoIntTimedOutCount << std::endl;
  //debugging purpose end
  pnlsr.getAdl().setStatusOfNeighbor(neighbor, 1);
  pnlsr.getAdl().setTimedOutInterestCount(neighbor, 0);
  int newStatus = pnlsr.getAdl().getStatusOfNeighbor(neighbor);
  infoIntTimedOutCount = pnlsr.getAdl().getTimedOutInterestCount(neighbor);
  //debugging purpose
  std::cout << "After Updates: " << std::endl;
  std::cout << "Neighbor : " << neighbor << std::endl;
  std::cout << "Status: " << newStatus << std::endl;
  std::cout << "Info Interest Timed out: " << infoIntTimedOutCount << std::endl;
  //debugging purpose end
  if ((oldStatus - newStatus) != 0)  // change in Adjacency list
  {
    pnlsr.incrementAdjBuildCount();
    /* Need to schedule event for Adjacency LSA building */
    if (pnlsr.getIsBuildAdjLsaSheduled() == 0)
    {
      pnlsr.setIsBuildAdjLsaSheduled(1);
      // event here
      pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(5),
                                         ndn::bind(&Lsdb::scheduledAdjLsaBuild, pnlsr.getLsdb(),
                                                   boost::ref(pnlsr)));
    }
  }
}

void
DataManager::processContentLsa(Nlsr& pnlsr, string& dataName,
                               string& dataContent)
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
    processContentNameLsa(pnlsr, origRouter + "/" + lsTypeString,
                          interestedLsSeqNo, dataContent);
  }
  else if (lsTypeString == "2")  //Adj Lsa
  {
    processContentAdjLsa(pnlsr, origRouter + "/" + lsTypeString,
                         interestedLsSeqNo, dataContent);
  }
  else if (lsTypeString == "3")  //Cor Lsa
  {
    processContentCorLsa(pnlsr, origRouter + "/" + lsTypeString,
                         interestedLsSeqNo, dataContent);
  }
  else
  {
    cout << "Unrecognized LSA Type :(" << endl;
  }
}

void
DataManager::processContentNameLsa(Nlsr& pnlsr, string lsaKey,
                                   uint32_t lsSeqNo, string& dataContent)
{
  if (pnlsr.getLsdb().isNameLsaNew(lsaKey, lsSeqNo))
  {
    NameLsa nameLsa;
    if (nameLsa.initializeFromContent(dataContent))
    {
      pnlsr.getLsdb().installNameLsa(pnlsr, nameLsa);
    }
    else
    {
      std::cout << "LSA data decoding error :(" << std::endl;
    }
  }
}

void
DataManager::processContentAdjLsa(Nlsr& pnlsr, string lsaKey,
                                  uint32_t lsSeqNo, string& dataContent)
{
  if (pnlsr.getLsdb().isAdjLsaNew(lsaKey, lsSeqNo))
  {
    AdjLsa adjLsa;
    if (adjLsa.initializeFromContent(dataContent))
    {
      pnlsr.getLsdb().installAdjLsa(pnlsr, adjLsa);
    }
    else
    {
      std::cout << "LSA data decoding error :(" << std::endl;
    }
  }
}

void
DataManager::processContentCorLsa(Nlsr& pnlsr, string lsaKey,
                                  uint32_t lsSeqNo, string& dataContent)
{
  if (pnlsr.getLsdb().isCorLsaNew(lsaKey, lsSeqNo))
  {
    CorLsa corLsa;
    if (corLsa.initializeFromContent(dataContent))
    {
      pnlsr.getLsdb().installCorLsa(pnlsr, corLsa);
    }
    else
    {
      std::cout << "LSA data decoding error :(" << std::endl;
    }
  }
}

void
DataManager::processContentKeys(Nlsr& pnlsr, const ndn::Data& data)
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
  if (pnlsr.getKeyManager().verify(pnlsr, *(cert)))
  {
    pnlsr.getKeyManager().addCertificate(cert, seqNum, true);
  }
  else
  {
    pnlsr.getKeyManager().addCertificate(cert, seqNum, false);
  }

  pnlsr.getKeyManager().printCertStore();
}
}//namespace nlsr
