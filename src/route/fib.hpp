#ifndef NLSR_FIB_HPP
#define NLSR_FIB_HPP

#include <list>
#include <boost/cstdint.hpp>

#include <ndn-cxx/management/nfd-controller.hpp>

#include "fib-entry.hpp"

namespace nlsr {

class Nlsr;


class Fib
{
public:
  Fib(ndn::Face& face)
    : m_table()
    , m_refreshTime(0)
    , m_controller(face)
  {
  }
  ~Fib()
  {
  }

  void
  remove(Nlsr& pnlsr, const std::string& name);

  void
  update(Nlsr& pnlsr, const std::string& name, NexthopList& nextHopList);

  void
  clean(Nlsr& pnlsr);

  void
  setEntryRefreshTime(int32_t fert)
  {
    m_refreshTime = fert;
  }

  void
  print();

private:
  void
  removeHop(Nlsr& pnlsr, NexthopList& nl, uint32_t doNotRemoveHopFaceId,
            const std::string& name);

  int
  getNumberOfFacesForName(NexthopList& nextHopList, uint32_t maxFacesPerPrefix);

  ndn::EventId
  scheduleEntryRefreshing(Nlsr& pnlsr, const std::string& name, int32_t feSeqNum,
                          int32_t refreshTime);

  void
  cancelScheduledExpiringEvent(Nlsr& pnlsr, ndn::EventId eid);

  void
  refreshEntry(Nlsr& nlsr, const std::string& name, int32_t feSeqNum);

  void
  registerPrefixInNfd(const std::string& namePrefix, uint64_t faceId, uint64_t faceCost);

  void
  unregisterPrefixFromNfd(const std::string& namePrefix, uint64_t faceId);
  
  void
  onSuccess(const ndn::nfd::ControlParameters& commandSuccessResult, const std::string& message);

  void
  onFailure(uint32_t code, const std::string& error, const std::string& message);

private:
  std::list<FibEntry> m_table;
  int32_t m_refreshTime;
  ndn::nfd::Controller m_controller;
};

}//namespace nlsr
#endif //NLSR_FIB_HPP
