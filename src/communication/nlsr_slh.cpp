#include "nlsr.hpp"
#include "nlsr_slh.hpp"
#include "security/nlsr_km.hpp"
#include "utility/nlsr_tokenizer.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_slh.cpp"


namespace nlsr
{
  void
  SyncLogicHandler::createSyncSocket(Nlsr& pnlsr )
  {
    cout<<"Creating Sync socket ......"<<endl;
    cout<<"Sync prefix: "<<m_syncPrefix.toUri()<<endl;
    m_syncSocket=make_shared<SyncSocket>(m_syncPrefix, m_validator, m_syncFace,
                                       bind(&SyncLogicHandler::nsyncUpdateCallBack,this,
                                            _1, _2,boost::ref(pnlsr)),
                                       bind(&SyncLogicHandler::nsyncRemoveCallBack, this,
                                            _1,boost::ref(pnlsr)));
  }

  void
  SyncLogicHandler::nsyncUpdateCallBack(const vector<MissingDataInfo> &v,
                                        SyncSocket *socket, Nlsr& pnlsr)
  {
    cout<<"nsyncUpdateCallBack called ...."<<endl;
    int n = v.size();
    for(int i=0; i < n; i++)
    {
      std::cout<<"Data Name: "<<v[i].prefix<<" Seq: "<<v[i].high.getSeq()<<endl;
      processUpdateFromSync(v[i].prefix,v[i].high.getSeq(),pnlsr);
    }
  }

  void
  SyncLogicHandler::nsyncRemoveCallBack(const string& prefix, Nlsr& pnlsr)
  {
    cout<<"nsyncRemoveCallBack called ...."<<endl;
  }

  void
  SyncLogicHandler::removeRouterFromSyncing(string& routerPrefix)
  {
  }

  void
  SyncLogicHandler::processUpdateFromSync(std::string updateName,
                                          uint64_t seqNo,  Nlsr& pnlsr)
  {
    nlsrTokenizer nt(updateName,"/");
    string chkString("LSA");
    if( nt.doesTokenExist(chkString) )
    {
      //process LSA Update here
      string routerName=nt.getTokenString(nt.getTokenPosition(chkString)+1);
      processRoutingUpdateFromSync(routerName, seqNo, pnlsr);
    }
    chkString="keys";
    if( nt.doesTokenExist(chkString) )
    {
      //process keys update here
      std::string certName=nt.getTokenString(0);
      processKeysUpdateFromSync(certName,seqNo, pnlsr);
    }
  }

  void
  SyncLogicHandler::processRoutingUpdateFromSync(std::string routerName,
      uint64_t seqNo,  Nlsr& pnlsr)
  {
    if( routerName != pnlsr.getConfParameter().getRouterPrefix() )
    {
      SequencingManager sm(seqNo);
      cout<<sm;
      cout<<"Router Name: "<<routerName<<endl;
      if ( pnlsr.getLsdb().isNameLsaNew(routerName+"/1",sm.getNameLsaSeq()))
      {
        cout<<"Updated Name LSA. Need to fetch it"<<endl;
        string lsaPrefix=
          pnlsr.getConfParameter().getChronosyncLsaPrefix() +
          routerName + "/1/" +
          boost::lexical_cast<std::string>(sm.getNameLsaSeq());
        pnlsr.getIm().expressInterest(pnlsr, lsaPrefix, 3,
                                      pnlsr.getConfParameter().getInterestResendTime());
      }
      if ( pnlsr.getLsdb().isAdjLsaNew(routerName+"/2",sm.getAdjLsaSeq()))
      {
        cout<<"Updated Adj LSA. Need to fetch it"<<endl;
        string lsaPrefix=
          pnlsr.getConfParameter().getChronosyncLsaPrefix() +
          routerName + "/2/" +
          boost::lexical_cast<std::string>(sm.getAdjLsaSeq());
        pnlsr.getIm().expressInterest(pnlsr, lsaPrefix, 3,
                                      pnlsr.getConfParameter().getInterestResendTime());
      }
      if ( pnlsr.getLsdb().isCorLsaNew(routerName+"/3",sm.getCorLsaSeq()))
      {
        cout<<"Updated Cor LSA. Need to fetch it"<<endl;
        string lsaPrefix=
          pnlsr.getConfParameter().getChronosyncLsaPrefix() +
          routerName + "/3/" +
          boost::lexical_cast<std::string>(sm.getCorLsaSeq());
        pnlsr.getIm().expressInterest(pnlsr, lsaPrefix, 3,
                                      pnlsr.getConfParameter().getInterestResendTime());
      }
    }
  }

  void
  SyncLogicHandler::processKeysUpdateFromSync(std::string certName,
      uint64_t seqNo, Nlsr& pnlsr)
  {
    cout<<"Cert Name: "<<certName<<std::endl;
    if ( pnlsr.getKeyManager().isNewCertificate(certName,seqNo) )
    {
      string certNamePrefix=certName + "/" +
                            boost::lexical_cast<string>(seqNo);
      pnlsr.getIm().expressInterest(pnlsr, certNamePrefix, 3,
                                    pnlsr.getConfParameter().getInterestResendTime());
    }
  }

  void
  SyncLogicHandler::publishRoutingUpdate(SequencingManager& sm,
                                         string updatePrefix)
  {
    sm.writeSeqNoToFile();
    publishSyncUpdate(updatePrefix,sm.getCombinedSeqNo());
  }

  void
  SyncLogicHandler::publishKeyUpdate(KeyManager& km)
  {
    publishSyncUpdate(km.getProcessCertName().toUri(),km.getCertSeqNo());
  }

  void
  SyncLogicHandler::publishIdentityUpdate(string identityName)
  {
    publishSyncUpdate(identityName,0);
  }

  void
  SyncLogicHandler::publishSyncUpdate(string updatePrefix, uint64_t seqNo)
  {
    cout<<"Publishing Sync Update ......"<<endl;
    cout<<"Update in prefix: "<<updatePrefix<<endl;
    cout<<"Seq No: "<<seqNo<<endl;
    ndn::Name updateName(updatePrefix);
    string data("NoData");
    m_syncSocket->publishData(updateName,0,data.c_str(),data.size(),1000,seqNo);
  }

}
