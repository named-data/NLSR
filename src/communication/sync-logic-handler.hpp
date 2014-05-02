#ifndef NLSR_SYNC_LOGIC_HANDLER_HPP
#define NLSR_SYNC_LOGIC_HANDLER_HPP

#include <iostream>
#include <boost/cstdint.hpp>

#include <ndn-cxx/face.hpp>
#include <nsync/sync-socket.h>
#include <ndn-cxx/security/validator-null.hpp>
#include <ndn-cxx/util/scheduler.hpp>

#include "sequencing-manager.hpp"

// class KeyManager;

extern "C" {
#include <unistd.h>
}

class InterestManager;
class ConfParameter;

namespace nlsr {

class SyncLogicHandler
{
public:
  SyncLogicHandler(boost::asio::io_service& ioService)
    : m_validator(new ndn::ValidatorNull())
    , m_syncFace(new ndn::Face(ioService))
  {}


  void
  createSyncSocket(Nlsr& pnlsr);

  void
  nsyncUpdateCallBack(const std::vector<Sync::MissingDataInfo>& v,
                      Sync::SyncSocket* socket, Nlsr& pnlsr);

  void
  nsyncRemoveCallBack(const std::string& prefix, Nlsr& pnlsr);

  void
  removeRouterFromSyncing(const std::string& routerPrefix);

  void
  publishRoutingUpdate(SequencingManager& sm, const std::string& updatePrefix);

  // void
  // publishKeyUpdate(KeyManager& km);

  void
  publishIdentityUpdate(const std::string& identityName);

  void
  setSyncPrefix(const std::string& sp)
  {
    m_syncPrefix.clear();
    m_syncPrefix.set(sp);
  }

private:
  void
  processUpdateFromSync(const std::string& updateName, uint64_t seqNo, Nlsr& pnlsr);

  void
  processRoutingUpdateFromSync(const std::string& routerName, uint64_t seqNo,
                               Nlsr& pnlsr);

  // void
  // processKeysUpdateFromSync(std::string certName, uint64_t seqNo, Nlsr& pnlsr);

  void
  publishSyncUpdate(const std::string& updatePrefix, uint64_t seqNo);

private:
  ndn::shared_ptr<ndn::ValidatorNull> m_validator;
  ndn::shared_ptr<ndn::Face> m_syncFace;
  ndn::shared_ptr<Sync::SyncSocket> m_syncSocket;
  ndn::Name m_syncPrefix;
};

} //namespace nlsr

#endif //NLSR_SYNC_LOGIC_HANDLER_HPP
