#ifndef NLSR_LSDB_HPP
#define NLSR_LSDB_HPP

#include <utility>
#include "nlsr_lsa.hpp"

namespace nlsr
{

  using namespace std;

  class Nlsr;

  class Lsdb
  {
  public:
    Lsdb()
      : m_lsaRefreshTime(0)
    {
    }


    bool doesLsaExist(string key, int lsType);
    // function related to Name LSDB
    bool buildAndInstallOwnNameLsa(Nlsr& pnlsr);
    std::pair<NameLsa&, bool>  getNameLsa(string key);
    bool installNameLsa(Nlsr& pnlsr, NameLsa &nlsa);
    bool removeNameLsa(Nlsr& pnlsr, string& key);
    bool isNameLsaNew(string key, uint64_t seqNo);
    void printNameLsdb(); //debugging

    //function related to Cor LSDB
    bool buildAndInstallOwnCorLsa(Nlsr& pnlsr);
    std::pair<CorLsa&, bool> getCorLsa(string key);
    bool installCorLsa(Nlsr& pnlsr, CorLsa &clsa);
    bool removeCorLsa(Nlsr& pnlsr, string& key);
    bool isCorLsaNew(string key, uint64_t seqNo);
    void printCorLsdb(); //debugging

    //function related to Adj LSDB
    void scheduledAdjLsaBuild(Nlsr& pnlsr);
    bool buildAndInstallOwnAdjLsa(Nlsr& pnlsr);
    bool removeAdjLsa(Nlsr& pnlsr, string& key);
    bool isAdjLsaNew(string key, uint64_t seqNo);
    bool installAdjLsa(Nlsr& pnlsr, AdjLsa &alsa);
    std::pair<AdjLsa& , bool> getAdjLsa(string key);
    std::list<AdjLsa>& getAdjLsdb();
    void printAdjLsdb();

    //void scheduleRefreshLsdb(Nlsr& pnlsr, int interval);
    void setLsaRefreshTime(int lrt);
    void setThisRouterPrefix(string trp);

  private:
    bool addNameLsa(NameLsa &nlsa);
    bool doesNameLsaExist(string key);


    bool addCorLsa(CorLsa& clsa);
    bool doesCorLsaExist(string key);

    bool addAdjLsa(AdjLsa &alsa);
    bool doesAdjLsaExist(string key);

    ndn::EventId
    scheduleNameLsaExpiration(Nlsr& pnlsr, string key, int seqNo, int expTime);
    void exprireOrRefreshNameLsa(Nlsr& pnlsr, string lsaKey, int seqNo);
    ndn::EventId
    scheduleAdjLsaExpiration(Nlsr& pnlsr, string key, int seqNo, int expTime);
    void exprireOrRefreshAdjLsa(Nlsr& pnlsr, string lsaKey, int seqNo);
    ndn::EventId
    scheduleCorLsaExpiration(Nlsr& pnlsr, string key, int seqNo, int expTime);
    void exprireOrRefreshCorLsa(Nlsr& pnlsr, string lsaKey, int seqNo);


  private:
    void cancelScheduleLsaExpiringEvent(Nlsr& pnlsr, EventId eid);

    std::list<NameLsa> m_nameLsdb;
    std::list<AdjLsa> m_adjLsdb;
    std::list<CorLsa> m_corLsdb;

    int m_lsaRefreshTime;
    string m_thisRouterPrefix;

  };

}//namespace nlsr

#endif
