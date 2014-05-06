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
  Fib(Nlsr& nlsr, ndn::Face& face)
    : m_nlsr(nlsr)
    , m_table()
    , m_refreshTime(0)
    , m_controller(face)
  {
  }
  ~Fib()
  {
  }

  void
  remove(const ndn::Name& name);

  void
  update(const ndn::Name& name, NexthopList& nextHopList);

  void
  clean();

  void
  setEntryRefreshTime(int32_t fert)
  {
    m_refreshTime = fert;
  }

  void
  print();

private:
  void
  removeHop(NexthopList& nl, uint32_t doNotRemoveHopFaceId,
            const ndn::Name& name);

  int
  getNumberOfFacesForName(NexthopList& nextHopList, uint32_t maxFacesPerPrefix);

  ndn::EventId
  scheduleEntryRefreshing(const ndn::Name& name, int32_t feSeqNum,
                          int32_t refreshTime);

  void
  cancelScheduledExpiringEvent(ndn::EventId eid);

  void
  refreshEntry(const ndn::Name& name, int32_t feSeqNum);

  void
  registerPrefixInNfd(const ndn::Name& namePrefix, uint64_t faceId,
                      uint64_t faceCost);

  void
  unregisterPrefixFromNfd(const ndn::Name& namePrefix, uint64_t faceId);

  void
  onSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
            const std::string& message);

  void
  onFailure(uint32_t code, const std::string& error, const std::string& message);

private:
  Nlsr& m_nlsr;
  std::list<FibEntry> m_table;
  int32_t m_refreshTime;
  ndn::nfd::Controller m_controller;
};

}//namespace nlsr
#endif //NLSR_FIB_HPP
