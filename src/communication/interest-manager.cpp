#include <iostream>
#include <cstdlib>


#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/util/io.hpp>

#include "nlsr.hpp"
#include "interest-manager.hpp"
#include "data-manager.hpp"
#include "utility/tokenizer.hpp"
#include "lsdb.hpp"

namespace nlsr {

using namespace std;
using namespace ndn;

void
InterestManager::processInterest(const ndn::Name& name,
                                 const ndn::Interest& interest)
{
  cout << "<< I: " << interest << endl;
  string intName = interest.getName().toUri();
  cout << "Interest Received for Name: " << intName << endl;
  Tokenizer nt(intName, "/");
  string chkString("info");
  if (nt.doesTokenExist(chkString))
  {
    string nbr = nt.getTokenString(nt.getTokenPosition(chkString) + 1);
    cout << "Neighbor: " << nbr << endl;
    processInterestInfo(nbr, interest);
  }
  chkString = "LSA";
  if (nt.doesTokenExist(chkString))
  {
    processInterestLsa(interest);
  }
  chkString = "keys";
  if (nt.doesTokenExist(chkString))
  {
    processInterestKeys(interest);
  }
}

void
InterestManager::processInterestInfo(const string& neighbor,
                                     const ndn::Interest& interest)
{
  if (m_nlsr.getAdl().isNeighbor(neighbor))
  {
    Data data(ndn::Name(interest.getName()).appendVersion());
    data.setFreshnessPeriod(time::seconds(10)); // 10 sec
    data.setContent((const uint8_t*)"info", sizeof("info"));
    // m_nlsr.getKeyManager().signData(data);
    m_keyChain.sign(data);
    cout << ">> D: " << data << endl;
    m_nlsr.getNlsrFace()->put(data);
    int status = m_nlsr.getAdl().getStatusOfNeighbor(neighbor);
    if (status == 0)
    {
      string intName = neighbor + "/" + "info" +
                       m_nlsr.getConfParameter().getRouterPrefix();
      expressInterest(intName, 2,
                      m_nlsr.getConfParameter().getInterestResendTime());
    }
  }
}

void
InterestManager::processInterestLsa(const ndn::Interest& interest)
{
  string intName = interest.getName().toUri();
  Tokenizer nt(intName, "/");
  string chkString("LSA");
  string origRouter = nt.getTokenString(nt.getTokenPosition(chkString) + 1,
                                        nt.getTokenNumber() - 3);
  string lsTypeString = nt.getToken(nt.getTokenNumber() - 2);
  string lsSeqString = nt.getToken(nt.getTokenNumber() - 1);
  std::cout << "Router Name: " << origRouter << std::endl;
  std::cout << "Ls Type    : " << lsTypeString << std::endl;
  std::cout << "Ls Seq     : " << lsSeqString << endl;
  uint8_t interestedLsType;
  uint32_t interestedLsSeqNo;
  try
  {
    interestedLsType = boost::lexical_cast<uint8_t>(lsTypeString);
    interestedLsSeqNo = boost::lexical_cast<uint32_t>(lsSeqString);
  }
  catch (std::exception& e)
  {
    return;
  }
  std::cout << "Ls Type: " << interestedLsType << std::endl;
  if (lsTypeString == "1") //Name Lsa
  {
    processInterestForNameLsa(interest,
                              origRouter + "/" + lsTypeString, interestedLsSeqNo);
  }
  else if (lsTypeString == "2") //Adj Lsa
  {
    processInterestForAdjLsa(interest,
                             origRouter + "/" + lsTypeString, interestedLsSeqNo);
  }
  else if (lsTypeString == "3") //Cor Lsa
  {
    processInterestForCorLsa(interest,
                             origRouter + "/" + lsTypeString, interestedLsSeqNo);
  }
  else
  {
    cout << "Unrecognized LSA Type :(" << endl;
  }
}

void
InterestManager::processInterestForNameLsa(const ndn::Interest& interest,
                                           const string& lsaKey, uint32_t interestedlsSeqNo)
{
  NameLsa*  nameLsa = m_nlsr.getLsdb().findNameLsa(lsaKey);
  if (nameLsa != 0)
  {
    if (nameLsa->getLsSeqNo() >= interestedlsSeqNo)
    {
      Data data(ndn::Name(interest.getName()).appendVersion());
      data.setFreshnessPeriod(time::seconds(10)); // 10 sec
      string content = nameLsa->getData();
      data.setContent((const uint8_t*)content.c_str(), content.size());
      // m_nlsr.getKeyManager().signData(data);
      m_keyChain.sign(data);
      std::cout << ">> D: " << data << std::endl;
      m_nlsr.getNlsrFace()->put(data);
    }
  }
}

void
InterestManager::processInterestForAdjLsa(const ndn::Interest& interest,
                                          const string& lsaKey, uint32_t interestedlsSeqNo)
{
  AdjLsa* adjLsa = m_nlsr.getLsdb().findAdjLsa(lsaKey);
  if (adjLsa != 0)
  {
    if (adjLsa->getLsSeqNo() >= interestedlsSeqNo)
    {
      Data data(ndn::Name(interest.getName()).appendVersion());
      data.setFreshnessPeriod(time::seconds(10)); // 10 sec
      string content = adjLsa->getData();
      data.setContent((const uint8_t*)content.c_str(), content.size());
      // m_nlsr.getKeyManager().signData(data);
      m_keyChain.sign(data);
      std::cout << ">> D: " << data << std::endl;
      m_nlsr.getNlsrFace()->put(data);
    }
  }
}

void
InterestManager::processInterestForCorLsa(const ndn::Interest& interest,
                                          const string& lsaKey, uint32_t interestedlsSeqNo)
{
  CoordinateLsa* corLsa = m_nlsr.getLsdb().findCoordinateLsa(lsaKey);
  if (corLsa != 0)
  {
    if (corLsa->getLsSeqNo() >= interestedlsSeqNo)
    {
      Data data(ndn::Name(interest.getName()).appendVersion());
      data.setFreshnessPeriod(time::seconds(10)); // 10 sec
      string content = corLsa->getData();
      data.setContent((const uint8_t*)content.c_str(), content.size());
      // m_nlsr.getKeyManager().signData(data);
      m_keyChain.sign(data);
      std::cout << ">> D: " << data << std::endl;
      m_nlsr.getNlsrFace()->put(data);
    }
  }
}

void
InterestManager::processInterestKeys(const ndn::Interest& interest)
{
  std::cout << "processInterestKeys called " << std::endl;
  // string intName = interest.getName().toUri();
  // std::cout << "Interest Name for Key: " << intName << std::endl;
  // Tokenizer nt(intName, "/");
  // std::string chkString("ID-CERT");
  // std::string certName;
  // uint32_t seqNum;
  // ndn::Name dataName;
  // std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool> chkCert;
  // if (nt.getTokenPosition(chkString) == nt.getTokenNumber() - 1)
  // {
  //   certName = nt.getTokenString(0, nt.getTokenNumber() - 1);
  //   cout << "Cert Name: " << certName << std::endl;
  //   chkCert = m_nlsr.getKeyManager().getCertificateFromStore(certName);
  // }
  // else
  // {
  //   certName = nt.getTokenString(0, nt.getTokenNumber() - 2);
  //   seqNum = boost::lexical_cast<uint32_t>(nt.getToken(nt.getTokenNumber() - 1));
  //   std::cout << "Cert Name: " << certName << " Seq Num: " << seqNum << std::endl;
  //   chkCert = m_nlsr.getKeyManager().getCertificateFromStore(certName, seqNum);
  // }
  // if (chkCert.second)
  // {
  //   if (nt.getTokenPosition(chkString) == nt.getTokenNumber() - 1)
  //   {
  //     std::string dn;
  //     dataName = ndn::Name(interest.getName()).appendVersion();
  //     std::pair<uint32_t, bool> seqChk =
  //       m_nlsr.getKeyManager().getCertificateSeqNum(certName);
  //     if (seqChk.second)
  //     {
  //       dn = dataName.toUri() + "/" + boost::lexical_cast<std::string>(seqChk.first);
  //       dataName = ndn::Name(dn);
  //     }
  //     else
  //     {
  //       dn = dataName.toUri() + "/" + boost::lexical_cast<std::string>(10);
  //       dataName = ndn::Name(dn);
  //     }
  //   }
  //   else
  //   {
  //     dataName = ndn::Name(interest.getName());
  //   }
  //   Data data(dataName.appendVersion());
  //   data.setFreshnessPeriod(time::seconds(10)); //10 sec
  //   data.setContent(chkCert.first->wireEncode());
  //   m_nlsr.getKeyManager().signData(data);
  //   m_nlsr.getNlsrFace()->put(data);
  // }
}


void
InterestManager::processInterestTimedOut(const ndn::Interest& interest)
{
  std::cout << "Timed out interest : " << interest.getName().toUri() << std::endl;
  string intName = interest.getName().toUri();
  Tokenizer nt(intName, "/");
  string chkString("info");
  if (nt.doesTokenExist(chkString))
  {
    string nbr = nt.getTokenString(0, nt.getTokenPosition(chkString) - 1);
    processInterestTimedOutInfo(nbr , interest);
  }
  chkString = "LSA";
  if (nt.doesTokenExist(chkString))
  {
    processInterestTimedOutLsa(interest);
  }
}

void
InterestManager::processInterestTimedOutInfo(const string& neighbor,
                                             const ndn::Interest& interest)
{
  m_nlsr.getAdl().incrementTimedOutInterestCount(neighbor);
  int status = m_nlsr.getAdl().getStatusOfNeighbor(neighbor);
  int infoIntTimedOutCount = m_nlsr.getAdl().getTimedOutInterestCount(neighbor);
  std::cout << "Neighbor: " << neighbor << std::endl;
  std::cout << "Status: " << status << std::endl;
  std::cout << "Info Interest Timed out: " << infoIntTimedOutCount << std::endl;
  if ((infoIntTimedOutCount < m_nlsr.getConfParameter().getInterestRetryNumber()))
  {
    string intName = neighbor + "/" + "info" +
                     m_nlsr.getConfParameter().getRouterPrefix();
    expressInterest(intName, 2,
                    m_nlsr.getConfParameter().getInterestResendTime());
  }
  else if ((status == 1) &&
           (infoIntTimedOutCount == m_nlsr.getConfParameter().getInterestRetryNumber()))
  {
    m_nlsr.getAdl().setStatusOfNeighbor(neighbor, 0);
    m_nlsr.incrementAdjBuildCount();
    if (m_nlsr.getIsBuildAdjLsaSheduled() == 0)
    {
      m_nlsr.setIsBuildAdjLsaSheduled(1);
      // event here
      m_nlsr.getScheduler().scheduleEvent(ndn::time::seconds(5),
                                          ndn::bind(&Lsdb::scheduledAdjLsaBuild,
                                                    &m_nlsr.getLsdb(),
                                                    boost::ref(m_nlsr)));
    }
  }
}

void
InterestManager::processInterestTimedOutLsa(const ndn::Interest& interest)
{
}

void
InterestManager::expressInterest(const string& interestNamePrefix,
                                 int scope, int seconds)
{
  std::cout << "Expressing Interest :" << interestNamePrefix << std::endl;
  ndn::Interest i((ndn::Name(interestNamePrefix)));
  i.setInterestLifetime(time::seconds(seconds));
  i.setMustBeFresh(true);
  m_nlsr.getNlsrFace()->expressInterest(i,
                                        ndn::bind(&DataManager::processContent,
                                                  &m_nlsr.getDm(),
                                                  _1, _2, boost::ref(*this)),
                                        ndn::bind(&InterestManager::processInterestTimedOut,
                                                  this, _1));
}


void
InterestManager::sendScheduledInfoInterest(int seconds)
{
  std::list<Adjacent> adjList = m_nlsr.getAdl().getAdjList();
  for (std::list<Adjacent>::iterator it = adjList.begin(); it != adjList.end();
       ++it)
  {
    string adjName = (*it).getName() + "/" + "info" +
                     m_nlsr.getConfParameter().getRouterPrefix();
    expressInterest(adjName, 2,
                    m_nlsr.getConfParameter().getInterestResendTime());
  }
  scheduleInfoInterest(m_nlsr.getConfParameter().getInfoInterestInterval());
}

void
InterestManager::scheduleInfoInterest(int seconds)
{
  EventId eid = m_nlsr.getScheduler().scheduleEvent(ndn::time::seconds(seconds),
                                                    ndn::bind(&InterestManager::sendScheduledInfoInterest, this,
                                                              seconds));
}


} //namespace nlsr
