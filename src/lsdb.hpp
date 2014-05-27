#ifndef NLSR_LSDB_HPP
#define NLSR_LSDB_HPP

#include <utility>
#include <boost/cstdint.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/time.hpp>

#include "lsa.hpp"

namespace nlsr {
class Nlsr;

class Lsdb
{
public:
  Lsdb(Nlsr& nlsr)
    : m_nlsr(nlsr)
    , m_lsaRefreshTime(0)
  {
  }


  bool
  doesLsaExist(const ndn::Name& key, const std::string& lsType);
  // function related to Name LSDB

  bool
  buildAndInstallOwnNameLsa();

  NameLsa*
  findNameLsa(const ndn::Name& key);

  bool
  installNameLsa(NameLsa& nlsa);

  bool
  removeNameLsa(const ndn::Name& key);

  bool
  isNameLsaNew(const ndn::Name& key, uint64_t seqNo);

  //debugging
  void
  printNameLsdb();

  //function related to Cor LSDB
  bool
  buildAndInstallOwnCoordinateLsa();

  CoordinateLsa*
  findCoordinateLsa(const ndn::Name& key);

  bool
  installCoordinateLsa(CoordinateLsa& clsa);

  bool
  removeCoordinateLsa(const ndn::Name& key);

  bool
  isCoordinateLsaNew(const ndn::Name& key, uint64_t seqNo);

  //debugging
  void
  printCorLsdb();

  //function related to Adj LSDB
  void
  scheduledAdjLsaBuild();

  bool
  buildAndInstallOwnAdjLsa();

  bool
  removeAdjLsa(const ndn::Name& key);

  bool
  isAdjLsaNew(const ndn::Name& key, uint64_t seqNo);
  bool
  installAdjLsa(AdjLsa& alsa);

  AdjLsa*
  findAdjLsa(const ndn::Name& key);

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
  doesNameLsaExist(const ndn::Name& key);


  bool
  addCoordinateLsa(CoordinateLsa& clsa);

  bool
  doesCoordinateLsaExist(const ndn::Name& key);

  bool
  addAdjLsa(AdjLsa& alsa);

  bool
  doesAdjLsaExist(const ndn::Name& key);

  ndn::EventId
  scheduleNameLsaExpiration(const ndn::Name& key, int seqNo,
                            const ndn::time::seconds& expTime);

  void
  exprireOrRefreshNameLsa(const ndn::Name& lsaKey, uint64_t seqNo);

  ndn::EventId
  scheduleAdjLsaExpiration(const ndn::Name& key, int seqNo,
                           const ndn::time::seconds& expTime);

  void
  exprireOrRefreshAdjLsa(const ndn::Name& lsaKey, uint64_t seqNo);

  ndn::EventId
  scheduleCoordinateLsaExpiration(const ndn::Name& key, int seqNo,
                                  const ndn::time::seconds& expTime);

  void
  exprireOrRefreshCoordinateLsa(const ndn::Name& lsaKey,
                                uint64_t seqNo);
public:
  void
  expressInterest(const ndn::Name& interestName, uint32_t interestLifeTime,
                  uint32_t timeoutCount);

  void
  processInterest(const ndn::Name& name, const ndn::Interest& interest);

private:
  void
  processInterestForNameLsa(const ndn::Interest& interest,
                            const ndn::Name& lsaKey,
                            uint32_t interestedlsSeqNo);

  void
  processInterestForAdjacencyLsa(const ndn::Interest& interest,
                                 const ndn::Name& lsaKey,
                                 uint32_t interestedlsSeqNo);

  void
  processInterestForCoordinateLsa(const ndn::Interest& interest,
                                  const ndn::Name& lsaKey,
                                  uint32_t interestedlsSeqNo);

  void
  processContent(const ndn::Interest& interest, const ndn::Data& data);

  void
  processContentNameLsa(const ndn::Name& lsaKey,
                        uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentAdjacencyLsa(const ndn::Name& lsaKey,
                             uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentCoordinateLsa(const ndn::Name& lsaKey,
                              uint32_t lsSeqNo, std::string& dataContent);

  void
  processInterestTimedOut(const ndn::Interest& interest, uint32_t timeoutCount);

  ndn::time::system_clock::TimePoint
  getLsaExpirationTimePoint();

private:
  void
  cancelScheduleLsaExpiringEvent(ndn::EventId eid);

  Nlsr& m_nlsr;
  ndn::KeyChain m_keyChain;
  std::list<NameLsa> m_nameLsdb;
  std::list<AdjLsa> m_adjLsdb;
  std::list<CoordinateLsa> m_corLsdb;

  int m_lsaRefreshTime;
  std::string m_thisRouterPrefix;

};

}//namespace nlsr

#endif //NLSR_LSDB_HPP
