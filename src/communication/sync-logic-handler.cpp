#include "nlsr.hpp"
#include "sync-logic-handler.hpp"
// #include "security/key-manager.hpp"
#include "utility/tokenizer.hpp"


namespace nlsr {

void
SyncLogicHandler::createSyncSocket(Nlsr& pnlsr)
{
  std::cout << "Creating Sync socket ......" << std::endl;
  std::cout << "Sync prefix: " << m_syncPrefix.toUri() << std::endl;
  m_syncSocket = make_shared<Sync::SyncSocket>(m_syncPrefix, m_validator,
                                               m_syncFace,
                                               bind(&SyncLogicHandler::nsyncUpdateCallBack, this,
                                                    _1, _2, boost::ref(pnlsr)),
                                               bind(&SyncLogicHandler::nsyncRemoveCallBack, this,
                                                    _1, boost::ref(pnlsr)));
}

void
SyncLogicHandler::nsyncUpdateCallBack(const vector<Sync::MissingDataInfo>& v,
                                      Sync::SyncSocket* socket, Nlsr& pnlsr)
{
  std::cout << "nsyncUpdateCallBack called ...." << std::endl;
  int n = v.size();
  for (int i = 0; i < n; i++)
  {
    std::cout << "Data Name: " << v[i].prefix << " Seq: " << v[i].high.getSeq() <<
              endl;
    processUpdateFromSync(v[i].prefix, v[i].high.getSeq(), pnlsr);
  }
}

void
SyncLogicHandler::nsyncRemoveCallBack(const string& prefix, Nlsr& pnlsr)
{
  std::cout << "nsyncRemoveCallBack called ...." << std::endl;
}

void
SyncLogicHandler::removeRouterFromSyncing(string& routerPrefix)
{
}

void
SyncLogicHandler::processUpdateFromSync(std::string updateName,
                                        uint64_t seqNo,  Nlsr& pnlsr)
{
  Tokenizer nt(updateName, "/");
  string chkString("LSA");
  if (nt.doesTokenExist(chkString))
  {
    //process LSA Update here
    string routerName = nt.getTokenString(nt.getTokenPosition(chkString) + 1);
    processRoutingUpdateFromSync(routerName, seqNo, pnlsr);
  }
  chkString = "keys";
  if (nt.doesTokenExist(chkString))
  {
    //process keys update here
    std::string certName = nt.getTokenString(0);
    // processKeysUpdateFromSync(certName, seqNo, pnlsr);
  }
}

void
SyncLogicHandler::processRoutingUpdateFromSync(std::string routerName,
                                               uint64_t seqNo,  Nlsr& pnlsr)
{
  if (routerName != pnlsr.getConfParameter().getRouterPrefix())
  {
    SequencingManager sm(seqNo);
    std::cout << sm;
    std::cout << "Router Name: " << routerName << endl;
    if (pnlsr.getLsdb().isNameLsaNew(routerName + "/1", sm.getNameLsaSeq()))
    {
      std::cout << "Updated Name LSA. Need to fetch it" << std::endl;
      string lsaPrefix =
        pnlsr.getConfParameter().getChronosyncLsaPrefix() +
        routerName + "/1/" +
        boost::lexical_cast<std::string>(sm.getNameLsaSeq());
      pnlsr.getIm().expressInterest(lsaPrefix, 3,
                                    pnlsr.getConfParameter().getInterestResendTime());
    }
    if (pnlsr.getLsdb().isAdjLsaNew(routerName + "/2", sm.getAdjLsaSeq()))
    {
      std::cout << "Updated Adj LSA. Need to fetch it" << std::endl;
      string lsaPrefix =
        pnlsr.getConfParameter().getChronosyncLsaPrefix() +
        routerName + "/2/" +
        boost::lexical_cast<std::string>(sm.getAdjLsaSeq());
      pnlsr.getIm().expressInterest(lsaPrefix, 3,
                                    pnlsr.getConfParameter().getInterestResendTime());
    }
    if (pnlsr.getLsdb().isCoordinateLsaNew(routerName + "/3", sm.getCorLsaSeq()))
    {
      std::cout << "Updated Cor LSA. Need to fetch it" << std::endl;
      string lsaPrefix =
        pnlsr.getConfParameter().getChronosyncLsaPrefix() +
        routerName + "/3/" +
        boost::lexical_cast<std::string>(sm.getCorLsaSeq());
      pnlsr.getIm().expressInterest(lsaPrefix, 3,
                                    pnlsr.getConfParameter().getInterestResendTime());
    }
  }
}

// void
// SyncLogicHandler::processKeysUpdateFromSync(std::string certName,
//                                             uint64_t seqNo, Nlsr& pnlsr)
// {
//   std::cout << "Cert Name: " << certName << std::endl;
//   // if (pnlsr.getKeyManager().isNewCertificate(certName, seqNo))
//   {
//     string certNamePrefix = certName + "/" +
//                             boost::lexical_cast<string>(seqNo);
//     pnlsr.getIm().expressInterest(certNamePrefix, 3,
//                                   pnlsr.getConfParameter().getInterestResendTime());
//   }
// }

void
SyncLogicHandler::publishRoutingUpdate(SequencingManager& sm,
                                       string updatePrefix)
{
  sm.writeSeqNoToFile();
  publishSyncUpdate(updatePrefix, sm.getCombinedSeqNo());
}

// void
// SyncLogicHandler::publishKeyUpdate(KeyManager& km)
// {
//   publishSyncUpdate(km.getProcessCertName().toUri(), km.getCertSeqNo());
// }

void
SyncLogicHandler::publishIdentityUpdate(string identityName)
{
  publishSyncUpdate(identityName, 0);
}

void
SyncLogicHandler::publishSyncUpdate(string updatePrefix, uint64_t seqNo)
{
  std::cout << "Publishing Sync Update ......" << std::endl;
  std::cout << "Update in prefix: " << updatePrefix << std::endl;
  std::cout << "Seq No: " << seqNo << std::endl;
  ndn::Name updateName(updatePrefix);
  string data("NoData");
  m_syncSocket->publishData(updateName, 0, data.c_str(), data.size(), 1000,
                            seqNo);
}

}//namespace nlsr
