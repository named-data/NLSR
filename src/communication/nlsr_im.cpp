#include<iostream>
#include<cstdlib>


#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/util/io.hpp>

#include "nlsr.hpp"
#include "nlsr_im.hpp"
#include "nlsr_dm.hpp"
#include "utility/nlsr_tokenizer.hpp"
#include "nlsr_lsdb.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_im.cpp"

namespace nlsr
{

  using namespace std;
  using namespace ndn;

  void
  InterestManager::processInterest( Nlsr& pnlsr,
                                    const ndn::Name& name,
                                    const ndn::Interest& interest)
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
  }

  void
  InterestManager::processInterestInfo(Nlsr& pnlsr, string& neighbor,
                                       const ndn::Interest& interest)
  {
    if ( pnlsr.getAdl().isNeighbor(neighbor) )
    {
      Data data(ndn::Name(interest.getName()).appendVersion());
      data.setFreshnessPeriod(time::seconds(10)); // 10 sec
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
  InterestManager::processInterestLsa(Nlsr& pnlsr,const ndn::Interest& interest)
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
  InterestManager::processInterestForNameLsa(Nlsr& pnlsr,
      const ndn::Interest& interest,
      string lsaKey, uint32_t interestedlsSeqNo)
  {
    std::pair<NameLsa&, bool>  nameLsa=pnlsr.getLsdb().getNameLsa(lsaKey);
    if( nameLsa.second )
    {
      if ( nameLsa.first.getLsSeqNo() >= interestedlsSeqNo )
      {
        Data data(ndn::Name(interest.getName()).appendVersion());
        data.setFreshnessPeriod(time::seconds(10)); // 10 sec
        string content=nameLsa.first.getData();
        data.setContent((const uint8_t*)content.c_str(),content.size());
        pnlsr.getKeyManager().signData(data);
        cout << ">> D: " << data << endl;
        pnlsr.getNlsrFace()->put(data);
      }
    }
  }

  void
  InterestManager::processInterestForAdjLsa(Nlsr& pnlsr,
      const ndn::Interest& interest,
      string lsaKey, uint32_t interestedlsSeqNo)
  {
    std::pair<AdjLsa&, bool>  adjLsa=pnlsr.getLsdb().getAdjLsa(lsaKey);
    if( adjLsa.second )
    {
      if ( adjLsa.first.getLsSeqNo() >= interestedlsSeqNo )
      {
        Data data(ndn::Name(interest.getName()).appendVersion());
        data.setFreshnessPeriod(time::seconds(10)); // 10 sec
        string content=adjLsa.first.getData();
        data.setContent((const uint8_t*)content.c_str(),content.size());
        pnlsr.getKeyManager().signData(data);
        cout << ">> D: " << data << endl;
        pnlsr.getNlsrFace()->put(data);
      }
    }
  }

  void
  InterestManager::processInterestForCorLsa(Nlsr& pnlsr,
      const ndn::Interest& interest,
      string lsaKey, uint32_t interestedlsSeqNo)
  {
    std::pair<CorLsa&, bool>  corLsa=pnlsr.getLsdb().getCorLsa(lsaKey);
    if( corLsa.second )
    {
      if ( corLsa.first.getLsSeqNo() >= interestedlsSeqNo )
      {
        Data data(ndn::Name(interest.getName()).appendVersion());
        data.setFreshnessPeriod(time::seconds(10)); // 10 sec
        string content=corLsa.first.getData();
        data.setContent((const uint8_t*)content.c_str(),content.size());
        pnlsr.getKeyManager().signData(data);
        cout << ">> D: " << data << endl;
        pnlsr.getNlsrFace()->put(data);
      }
    }
  }

  void
  InterestManager::processInterestKeys(Nlsr& pnlsr,const ndn::Interest& interest)
  {
    cout<<"processInterestKeys called "<<endl;
    string intName=interest.getName().toUri();
    cout<<"Interest Name for Key: "<<intName<<std::endl;
    nlsrTokenizer nt(intName,"/");
    std::string chkString("ID-CERT");
    std::string certName;
    uint32_t seqNum;
    ndn::Name dataName;
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool> chkCert;
    if( nt.getTokenPosition(chkString) == nt.getTokenNumber()-1 )
    {
      certName=nt.getTokenString(0,nt.getTokenNumber()-1);
      cout<<"Cert Name: "<<certName<<std::endl;
      chkCert=pnlsr.getKeyManager().getCertificateFromStore(certName);
    }
    else
    {
      certName=nt.getTokenString(0,nt.getTokenNumber()-2);
      seqNum=boost::lexical_cast<uint32_t>(nt.getToken(nt.getTokenNumber()-1));
      cout<<"Cert Name: "<<certName<<" Seq Num: "<<seqNum<<std::endl;
      chkCert=pnlsr.getKeyManager().getCertificateFromStore(certName,seqNum);
    }
    if( chkCert.second )
    {
      if(nt.getTokenPosition(chkString) == nt.getTokenNumber()-1)
      {
        std::string dn;
        dataName=ndn::Name(interest.getName()).appendVersion();
        std::pair<uint32_t, bool> seqChk = 
                           pnlsr.getKeyManager().getCertificateSeqNum(certName);
        if( seqChk.second )
        {
          dn=dataName.toUri()+"/"+boost::lexical_cast<std::string>(seqChk.first);
          dataName=ndn::Name(dn);
        }
        else
        {
          dn=dataName.toUri()+"/"+boost::lexical_cast<std::string>(10);
          dataName=ndn::Name(dn);
        }
        
      }
      else
      {
        dataName=ndn::Name(interest.getName());
      }
      Data data(dataName.appendVersion());
      data.setFreshnessPeriod(time::seconds(10)); //10 sec
      data.setContent(chkCert.first->wireEncode());
      pnlsr.getKeyManager().signData(data);
      pnlsr.getNlsrFace()->put(data);
    }
  }


  void
  InterestManager::processInterestTimedOut(Nlsr& pnlsr,
      const ndn::Interest& interest)
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
  InterestManager::processInterestTimedOutInfo(Nlsr& pnlsr, string& neighbor,
      const ndn::Interest& interest)
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
  InterestManager::processInterestTimedOutLsa(Nlsr& pnlsr,
      const ndn::Interest& interest)
  {
  }

  void
  InterestManager::expressInterest(Nlsr& pnlsr,const string& interestNamePrefix,
                                   int scope, int seconds)
  {
    cout<<"Expressing Interest :"<<interestNamePrefix<<endl;
    ndn::Interest i((ndn::Name(interestNamePrefix)));
    //i.setScope(scope);
    i.setInterestLifetime(time::seconds(seconds));
    i.setMustBeFresh(true);
    pnlsr.getNlsrFace()->expressInterest(i,
                                         ndn::func_lib::bind(&DataManager::processContent,
                                             &pnlsr.getDm(), boost::ref(pnlsr),_1, _2,boost::ref(*this)),
                                         ndn::func_lib::bind(&InterestManager::processInterestTimedOut,
                                             this,boost::ref(pnlsr),_1));
  }


  void
  InterestManager::sendScheduledInfoInterest(Nlsr& pnlsr, int seconds)
  {
    std::list<Adjacent> adjList=pnlsr.getAdl().getAdjList();
    for(std::list<Adjacent>::iterator it=adjList.begin(); it!=adjList.end(); ++it)
    {
      string adjName=(*it).getName()+"/"+"info"+
                     pnlsr.getConfParameter().getRouterPrefix();
      expressInterest(	pnlsr,adjName,2,
                        pnlsr.getConfParameter().getInterestResendTime());
    }
    scheduleInfoInterest(pnlsr, pnlsr.getConfParameter().getInfoInterestInterval());
  }

  void
  InterestManager::scheduleInfoInterest(Nlsr& pnlsr, int seconds)
  {
    EventId eid=pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(seconds),
                ndn::bind(&InterestManager::sendScheduledInfoInterest, this,
                          boost::ref(pnlsr),seconds));
  }


} //namespace nlsr
