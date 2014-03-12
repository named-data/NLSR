#include<iostream>
#include<cstdlib>


#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/util/io.hpp>

#include "nlsr.hpp"
#include "nlsr_im.hpp"
#include "nlsr_dm.hpp"
#include "utility/nlsr_tokenizer.hpp"
#include "nlsr_lsdb.hpp"

namespace nlsr
{

  using namespace std;
  using namespace ndn;

  void
  interestManager::processInterest( Nlsr& pnlsr,
                                    const ndn::Name &name,
                                    const ndn::Interest &interest)
  {
    cout << "<< I: " << interest << endl;
    string intName=interest.getName().toUri();
    cout << "Interest Received for Name: "<< intName <<endl;
    nlsrTokenizer nt(intName,"/");
    string chkString("info");
    if( nt.doesTokenExist(chkString) )
    {
      string nbr=nt.getTokenString(nt.getTokenPosition(chkString)+1);
      cout <<"Neighbor: " << nbr <<endl;
      processInterestInfo(pnlsr,nbr,interest);
    }
    chkString="LSA";
    if( nt.doesTokenExist(chkString) )
    {
      processInterestLsa(pnlsr,interest);
    }
    chkString="keys";
    if( nt.doesTokenExist(chkString) )
    {
      processInterestKeys(pnlsr,interest);
    }
    //Data data(ndn::Name(interest->getName()).append("testApp").appendVersion());
    //data.setFreshnessPeriod(1000); // 10 sec
    //data.setContent((const uint8_t*)"HELLO KITTY", sizeof("HELLO KITTY"));
    //pnlsr.getKeyChain().sign(data);
    //cout << ">> D: " << data << endl;
    //pnlsr.getNlsrFace().put(data);
  }

  void
  interestManager::processInterestInfo(Nlsr& pnlsr, string& neighbor,
                                       const ndn::Interest &interest)
  {
    if ( pnlsr.getAdl().isNeighbor(neighbor) )
    {
      Data data(ndn::Name(interest.getName()).appendVersion());
      data.setFreshnessPeriod(1000); // 10 sec
      data.setContent((const uint8_t*)"info", sizeof("info"));
      pnlsr.getKeyManager().signData(data);
      cout << ">> D: " << data << endl;
      pnlsr.getNlsrFace()->put(data);
      int status=pnlsr.getAdl().getStatusOfNeighbor(neighbor);
      if ( status == 0 )
      {
        string intName=neighbor +"/"+"info"+
                       pnlsr.getConfParameter().getRouterPrefix();
        expressInterest(	pnlsr,intName,2,
                          pnlsr.getConfParameter().getInterestResendTime());
      }
    }
  }

  void
  interestManager::processInterestLsa(Nlsr& pnlsr,const ndn::Interest &interest)
  {
    string intName=interest.getName().toUri();
    nlsrTokenizer nt(intName,"/");
    string chkString("LSA");
    string origRouter=nt.getTokenString(nt.getTokenPosition(chkString)+1,
                                        nt.getTokenNumber()-3);
    string lsTypeString=nt.getToken(nt.getTokenNumber()-2);
    string lsSeqString=nt.getToken(nt.getTokenNumber()-1);
    cout<<"Router Name: "<<origRouter<<endl;
    cout<<"Ls Type    : "<<lsTypeString<<endl;
    cout<<"Ls Seq     : "<<lsSeqString<<endl;
    uint8_t interestedLsType;
    uint32_t interestedLsSeqNo;
    try
    {
      interestedLsType=boost::lexical_cast<uint8_t>(lsTypeString);
      interestedLsSeqNo=boost::lexical_cast<uint32_t>(lsSeqString);
    }
    catch(std::exception &e)
    {
      return;
    }
    cout <<"Ls Type: "<<interestedLsType<<endl;
    if( lsTypeString == "1" ) //Name Lsa
    {
      processInterestForNameLsa(pnlsr, interest,
                                origRouter+"/"+lsTypeString, interestedLsSeqNo);
    }
    else if( lsTypeString == "2" ) //Adj Lsa
    {
      processInterestForAdjLsa(pnlsr, interest,
                               origRouter+"/"+lsTypeString, interestedLsSeqNo);
    }
    else if( lsTypeString == "3" ) //Cor Lsa
    {
      processInterestForCorLsa(pnlsr, interest,
                               origRouter+"/"+lsTypeString, interestedLsSeqNo);
    }
    else
    {
      cout<<"Unrecognized LSA Type :("<<endl;
    }
  }

  void
  interestManager::processInterestForNameLsa(Nlsr& pnlsr,
      const ndn::Interest &interest,
      string lsaKey, uint32_t interestedlsSeqNo)
  {
    std::pair<NameLsa&, bool>  nameLsa=pnlsr.getLsdb().getNameLsa(lsaKey);
    if( nameLsa.second )
    {
      if ( nameLsa.first.getLsSeqNo() >= interestedlsSeqNo )
      {
        Data data(ndn::Name(interest.getName()).appendVersion());
        data.setFreshnessPeriod(1000); // 10 sec
        string content=nameLsa.first.getNameLsaData();
        data.setContent((const uint8_t*)content.c_str(),content.size());
        pnlsr.getKeyManager().signData(data);
        cout << ">> D: " << data << endl;
        pnlsr.getNlsrFace()->put(data);
      }
    }
  }

  void
  interestManager::processInterestForAdjLsa(Nlsr& pnlsr,
      const ndn::Interest &interest,
      string lsaKey, uint32_t interestedlsSeqNo)
  {
    std::pair<AdjLsa&, bool>  adjLsa=pnlsr.getLsdb().getAdjLsa(lsaKey);
    if( adjLsa.second )
    {
      if ( adjLsa.first.getLsSeqNo() >= interestedlsSeqNo )
      {
        Data data(ndn::Name(interest.getName()).appendVersion());
        data.setFreshnessPeriod(1000); // 10 sec
        string content=adjLsa.first.getAdjLsaData();
        data.setContent((const uint8_t*)content.c_str(),content.size());
        pnlsr.getKeyManager().signData(data);
        cout << ">> D: " << data << endl;
        pnlsr.getNlsrFace()->put(data);
      }
    }
  }

  void
  interestManager::processInterestForCorLsa(Nlsr& pnlsr,
      const ndn::Interest &interest,
      string lsaKey, uint32_t interestedlsSeqNo)
  {
    std::pair<CorLsa&, bool>  corLsa=pnlsr.getLsdb().getCorLsa(lsaKey);
    if( corLsa.second )
    {
      if ( corLsa.first.getLsSeqNo() >= interestedlsSeqNo )
      {
        Data data(ndn::Name(interest.getName()).appendVersion());
        data.setFreshnessPeriod(1000); // 10 sec
        string content=corLsa.first.getCorLsaData();
        data.setContent((const uint8_t*)content.c_str(),content.size());
        pnlsr.getKeyManager().signData(data);
        cout << ">> D: " << data << endl;
        pnlsr.getNlsrFace()->put(data);
      }
    }
  }

  void
  interestManager::processInterestKeys(Nlsr& pnlsr,const ndn::Interest &interest)
  {
    cout<<"processInterestKeys called "<<endl;
    string intName=interest.getName().toUri();
    cout<<"Interest Name for Key: "<<intName<<std::endl;
    nlsrTokenizer nt(intName,"/");
    std::string certName=nt.getTokenString(0,nt.getTokenNumber()-2);
    uint32_t seqNum=boost::lexical_cast<uint32_t>(nt.getToken(
                      nt.getTokenNumber()-1));
    cout<<"Cert Name: "<<certName<<" Seq Num: "<<seqNum<<std::endl;
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool> chkCert=
      pnlsr.getKeyManager().getCertificateFromStore(certName,seqNum);
    if( chkCert.second )
    {
      Data data(ndn::Name(interest.getName()).appendVersion());
      data.setFreshnessPeriod(1000); //10 sec
      data.setContent(chkCert.first->wireEncode());
      pnlsr.getKeyManager().signData(data);
      pnlsr.getNlsrFace()->put(data);
    }
    //std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool> chkCert=
    /*
    ndn::shared_ptr<ndn::IdentityCertificate> cert=pnlsr.getKeyManager().getCertificate();
    Data data(ndn::Name(interest.getName()).appendVersion());
    data.setFreshnessPeriod(1000); // 10 sec
    data.setContent(cert->wireEncode());
    pnlsr.getKeyManager().signData(data);
    //std::ofstream outFile("data_sent");
    //ndn::io::save(data,outFile,ndn::io::NO_ENCODING);
    pnlsr.getNlsrFace()->put(data);
    */
  }


  void
  interestManager::processInterestTimedOut(Nlsr& pnlsr,
      const ndn::Interest &interest)
  {
    cout << "Timed out interest : " << interest.getName().toUri() << endl;
    string intName=	interest.getName().toUri();
    nlsrTokenizer nt(intName,"/");
    string chkString("info");
    if( nt.doesTokenExist(chkString) )
    {
      string nbr=nt.getTokenString(0,nt.getTokenPosition(chkString)-1);
      processInterestTimedOutInfo( pnlsr , nbr , interest);
    }
    chkString="LSA";
    if( nt.doesTokenExist(chkString) )
    {
      processInterestTimedOutLsa(pnlsr, interest);
    }
  }

  void
  interestManager::processInterestTimedOutInfo(Nlsr& pnlsr, string& neighbor,
      const ndn::Interest &interest)
  {
    pnlsr.getAdl().incrementTimedOutInterestCount(neighbor);
    int status=pnlsr.getAdl().getStatusOfNeighbor(neighbor);
    int infoIntTimedOutCount=pnlsr.getAdl().getTimedOutInterestCount(neighbor);
    cout<<"Neighbor: "<< neighbor << endl;
    cout<<"Status: "<< status << endl;
    cout<<"Info Interest Timed out: "<< infoIntTimedOutCount <<endl;
    if((infoIntTimedOutCount < pnlsr.getConfParameter().getInterestRetryNumber()))
    {
      string intName=neighbor +"/"+"info"+
                     pnlsr.getConfParameter().getRouterPrefix();
      expressInterest(	pnlsr,intName,2,
                        pnlsr.getConfParameter().getInterestResendTime());
    }
    else if ( (status == 1) &&
              (infoIntTimedOutCount == pnlsr.getConfParameter().getInterestRetryNumber()))
    {
      pnlsr.getAdl().setStatusOfNeighbor(neighbor,0);
      pnlsr.incrementAdjBuildCount();
      if ( pnlsr.getIsBuildAdjLsaSheduled() == 0 )
      {
        pnlsr.setIsBuildAdjLsaSheduled(1);
        // event here
        pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(5),
                                           ndn::bind(&Lsdb::scheduledAdjLsaBuild,&pnlsr.getLsdb(),
                                               boost::ref(pnlsr)));
      }
    }
  }

  void
  interestManager::processInterestTimedOutLsa(Nlsr& pnlsr,
      const ndn::Interest &interest)
  {
  }

  void
  interestManager::expressInterest(Nlsr& pnlsr,const string& interestNamePrefix,
                                   int scope, int seconds)
  {
    cout<<"Expressing Interest :"<<interestNamePrefix<<endl;
    ndn::Interest i((ndn::Name(interestNamePrefix)));
    //i.setScope(scope);
    i.setInterestLifetime(seconds*1000);
    i.setMustBeFresh(true);
    pnlsr.getNlsrFace()->expressInterest(i,
                                         ndn::func_lib::bind(&DataManager::processContent,
                                             &pnlsr.getDm(), boost::ref(pnlsr),_1, _2,boost::ref(*this)),
                                         ndn::func_lib::bind(&interestManager::processInterestTimedOut,
                                             this,boost::ref(pnlsr),_1));
  }


  void
  interestManager::sendScheduledInfoInterest(Nlsr& pnlsr, int seconds)
  {
    std::list<Adjacent> adjList=pnlsr.getAdl().getAdjList();
    for(std::list<Adjacent>::iterator it=adjList.begin(); it!=adjList.end(); ++it)
    {
      string adjName=(*it).getAdjacentName()+"/"+"info"+
                     pnlsr.getConfParameter().getRouterPrefix();
      expressInterest(	pnlsr,adjName,2,
                        pnlsr.getConfParameter().getInterestResendTime());
    }
    scheduleInfoInterest(pnlsr, pnlsr.getConfParameter().getInfoInterestInterval());
  }

  void
  interestManager::scheduleInfoInterest(Nlsr& pnlsr, int seconds)
  {
    EventId eid=pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(seconds),
                ndn::bind(&interestManager::sendScheduledInfoInterest, this,
                          boost::ref(pnlsr),seconds));
  }


} //namespace nlsr
