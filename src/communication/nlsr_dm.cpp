#include<iostream>
#include<cstdlib>

#include <ndn-cpp-dev/security/signature-sha256-with-rsa.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/util/io.hpp>

#include "nlsr.hpp"
#include "nlsr_dm.hpp"
#include "utility/nlsr_tokenizer.hpp"
#include "nlsr_lsdb.hpp"
#include "security/nlsr_km.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_dm.cpp"

namespace nlsr
{

  using namespace std;
  using namespace ndn;

  void
  DataManager::processContent(Nlsr& pnlsr, const ndn::Interest &interest,
                              const ndn::Data & data, InterestManager& im)
  {
    cout << "I: " << interest.toUri() << endl;
    string dataName(data.getName().toUri());
    nlsrTokenizer nt(dataName,"/");
    std::string chkString("keys");
    if( nt.doesTokenExist(chkString) )
    {
      processContentKeys(pnlsr, data);
    }
    else
    {
      if ( pnlsr.getKeyManager().verify(data))
      {
        std::cout<<"Verified Data Content"<<std::endl;
        chkString="info";
        if( nt.doesTokenExist(chkString) )
        {
          string dataContent((char *)data.getContent().value());
          processContentInfo(pnlsr,dataName,dataContent);
        }
        chkString="LSA";
        if( nt.doesTokenExist(chkString) )
        {
          string dataContent((char *)data.getContent().value());
          processContentLsa(pnlsr, dataName, dataContent);
        }
      }
      else
      {
        std::cout<<"Unverified Data Content. Discarded"<<std::endl;
      }
    }
  }

  void
  DataManager::processContentInfo(Nlsr& pnlsr, string& dataName,
                                  string& dataContent)
  {
    nlsrTokenizer nt(dataName,"/");
    string chkString("info");
    string neighbor=nt.getTokenString(0,nt.getTokenPosition(chkString)-1);
    int oldStatus=pnlsr.getAdl().getStatusOfNeighbor(neighbor);
    int infoIntTimedOutCount=pnlsr.getAdl().getTimedOutInterestCount(neighbor);
    //debugging purpose start
    cout <<"Before Updates: " <<endl;
    cout <<"Neighbor : "<<neighbor<<endl;
    cout<<"Status: "<< oldStatus << endl;
    cout<<"Info Interest Timed out: "<< infoIntTimedOutCount <<endl;
    //debugging purpose end
    pnlsr.getAdl().setStatusOfNeighbor(neighbor,1);
    pnlsr.getAdl().setTimedOutInterestCount(neighbor,0);
    int newStatus=pnlsr.getAdl().getStatusOfNeighbor(neighbor);
    infoIntTimedOutCount=pnlsr.getAdl().getTimedOutInterestCount(neighbor);
    //debugging purpose
    cout <<"After Updates: " <<endl;
    cout <<"Neighbor : "<<neighbor<<endl;
    cout<<"Status: "<< newStatus << endl;
    cout<<"Info Interest Timed out: "<< infoIntTimedOutCount <<endl;
    //debugging purpose end
    if ( ( oldStatus-newStatus)!= 0 ) // change in Adjacency list
    {
      pnlsr.incrementAdjBuildCount();
      /* Need to schedule event for Adjacency LSA building */
      if ( pnlsr.getIsBuildAdjLsaSheduled() == 0 )
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
    nlsrTokenizer nt(dataName,"/");
    string chkString("LSA");
    string origRouter=nt.getTokenString(nt.getTokenPosition(chkString)+1,
                                        nt.getTokenNumber()-4);
    string lsTypeString=nt.getToken(nt.getTokenNumber()-3);
    string lsSeNoString=nt.getToken(nt.getTokenNumber()-2);
    uint32_t interestedLsSeqNo;
    try
    {
      interestedLsSeqNo=boost::lexical_cast<uint32_t>(lsSeNoString);
    }
    catch(std::exception &e)
    {
      return;
    }
    if( lsTypeString == "1" ) //Name Lsa
    {
      processContentNameLsa(pnlsr, origRouter+"/"+lsTypeString,
                            interestedLsSeqNo, dataContent);
    }
    else if( lsTypeString == "2" ) //Adj Lsa
    {
      processContentAdjLsa(pnlsr, origRouter+"/"+lsTypeString,
                           interestedLsSeqNo, dataContent);
    }
    else if( lsTypeString == "3" ) //Cor Lsa
    {
      processContentCorLsa(pnlsr, origRouter+"/"+lsTypeString,
                           interestedLsSeqNo, dataContent);
    }
    else
    {
      cout<<"Unrecognized LSA Type :("<<endl;
    }
  }

  void
  DataManager::processContentNameLsa(Nlsr& pnlsr, string lsaKey,
                                     uint32_t lsSeqNo, string& dataContent)
  {
    if ( pnlsr.getLsdb().isNameLsaNew(lsaKey,lsSeqNo))
    {
      NameLsa nameLsa;
      if( nameLsa.initializeFromContent(dataContent) )
      {
        pnlsr.getLsdb().installNameLsa(pnlsr, nameLsa);
      }
      else
      {
        cout<<"LSA data decoding error :("<<endl;
      }
    }
  }

  void
  DataManager::processContentAdjLsa(Nlsr& pnlsr, string lsaKey,
                                    uint32_t lsSeqNo, string& dataContent)
  {
    if ( pnlsr.getLsdb().isAdjLsaNew(lsaKey,lsSeqNo))
    {
      AdjLsa adjLsa;
      if( adjLsa.initializeFromContent(dataContent) )
      {
        pnlsr.getLsdb().installAdjLsa(pnlsr, adjLsa);
      }
      else
      {
        cout<<"LSA data decoding error :("<<endl;
      }
    }
  }

  void
  DataManager::processContentCorLsa(Nlsr& pnlsr, string lsaKey,
                                    uint32_t lsSeqNo, string& dataContent)
  {
    if ( pnlsr.getLsdb().isCorLsaNew(lsaKey,lsSeqNo))
    {
      CorLsa corLsa;
      if( corLsa.initializeFromContent(dataContent) )
      {
        pnlsr.getLsdb().installCorLsa(pnlsr, corLsa);
      }
      else
      {
        cout<<"LSA data decoding error :("<<endl;
      }
    }
  }

  void
  DataManager::processContentKeys(Nlsr& pnlsr, const ndn::Data& data)
  {
    std::cout<<" processContentKeys called "<<endl;
    ndn::shared_ptr<ndn::IdentityCertificate> cert=
      ndn::make_shared<ndn::IdentityCertificate>();
    cert->wireDecode(data.getContent().blockFromValue());
    std::cout<<*(cert)<<endl;
    std::string dataName=data.getName().toUri();
    nlsrTokenizer nt(dataName,"/");
    std::string certName=nt.getTokenString(0,nt.getTokenNumber()-3);
    uint32_t seqNum=boost::lexical_cast<uint32_t>(nt.getToken(
                      nt.getTokenNumber()-2));
    cout<<"Cert Name: "<<certName<<" Seq Num: "<<seqNum<<std::endl;
    if ( pnlsr.getKeyManager().verify(pnlsr, *(cert)))
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
