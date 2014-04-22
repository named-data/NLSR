#ifndef NLSR_LSDB_HPP
#define NLSR_LSDB_HPP

#include <utility>
#include "lsa.hpp"

namespace nlsr {
class Nlsr;

class Lsdb
{
public:
  Lsdb()
    : m_lsaRefreshTime(0)
  {
  }


  bool
  doesLsaExist(std::string key, int lsType);
  // function related to Name LSDB

  bool
  buildAndInstallOwnNameLsa(Nlsr& pnlsr);

  std::pair<NameLsa&, bool>
  getNameLsa(std::string key);

  bool
  installNameLsa(Nlsr& pnlsr, NameLsa& nlsa);

  bool
  removeNameLsa(Nlsr& pnlsr, std::string& key);

  bool
  isNameLsaNew(std::string key, uint64_t seqNo);

  void
  printNameLsdb(); //debugging

  //function related to Cor LSDB
  bool
  buildAndInstallOwnCorLsa(Nlsr& pnlsr);

  std::pair<CorLsa&, bool>
  getCorLsa(std::string key);

  bool
  installCorLsa(Nlsr& pnlsr, CorLsa& clsa);

  bool
  removeCorLsa(Nlsr& pnlsr, std::string& key);

  bool
  isCorLsaNew(std::string key, uint64_t seqNo);

  void
  printCorLsdb(); //debugging

  //function related to Adj LSDB
  void
  scheduledAdjLsaBuild(Nlsr& pnlsr);

  bool
  buildAndInstallOwnAdjLsa(Nlsr& pnlsr);

  bool
  removeAdjLsa(Nlsr& pnlsr, std::string& key);

  bool
  isAdjLsaNew(std::string key, uint64_t seqNo);
  bool
  installAdjLsa(Nlsr& pnlsr, AdjLsa& alsa);

  std::pair<AdjLsa&, bool>
  getAdjLsa(std::string key);

  std::list<AdjLsa>&
  getAdjLsdb();

  void
  printAdjLsdb();

  //void scheduleRefreshLsdb(Nlsr& pnlsr, int interval);
  void
  setLsaRefreshTime(int lrt);

  void
  setThisRouterPrefix(std::string trp);

private:
  bool
  addNameLsa(NameLsa& nlsa);

  bool
  doesNameLsaExist(std::string key);


  bool
  addCorLsa(CorLsa& clsa);

  bool
  doesCorLsaExist(std::string key);

  bool
  addAdjLsa(AdjLsa& alsa);

  bool
  doesAdjLsaExist(std::string key);

  ndn::EventId
  scheduleNameLsaExpiration(Nlsr& pnlsr, std::string key, int seqNo, int expTime);

  void
  exprireOrRefreshNameLsa(Nlsr& pnlsr, std::string lsaKey, uint64_t seqNo);

  ndn::EventId
  scheduleAdjLsaExpiration(Nlsr& pnlsr, std::string key, int seqNo, int expTime);

  void
  exprireOrRefreshAdjLsa(Nlsr& pnlsr, std::string lsaKey, uint64_t seqNo);

  ndn::EventId
  scheduleCorLsaExpiration(Nlsr& pnlsr, std::string key, int seqNo, int expTime);

  void
  exprireOrRefreshCorLsa(Nlsr& pnlsr, std::string lsaKey, uint64_t seqNo);


private:
  void
  cancelScheduleLsaExpiringEvent(Nlsr& pnlsr, ndn::EventId eid);

  std::list<NameLsa> m_nameLsdb;
  std::list<AdjLsa> m_adjLsdb;
  std::list<CorLsa> m_corLsdb;

  int m_lsaRefreshTime;
  std::string m_thisRouterPrefix;

};

}//namespace nlsr

#endif //NLSR_LSDB_HPP
