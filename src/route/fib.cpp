#include <list>
#include <cmath>
#include <ndn-cxx/common.hpp>

#include "nlsr.hpp"
#include "nexthop-list.hpp"
#include "face-map.hpp"
#include "fib.hpp"




namespace nlsr {

using namespace std;
using namespace ndn;

static bool
fibEntryNameCompare(const FibEntry& fibEntry, const ndn::Name& name)
{
  return fibEntry.getName() == name ;
}

void
Fib::cancelScheduledExpiringEvent(EventId eid)
{
  m_nlsr.getScheduler().cancelEvent(eid);
}


ndn::EventId
Fib::scheduleEntryRefreshing(const ndn::Name& name, int32_t feSeqNum,
                             int32_t refreshTime)
{
  std::cout << "Fib::scheduleEntryRefreshing Called" << std::endl;
  std::cout << "Name: " << name << " Seq Num: " << feSeqNum << std::endl;
  return m_nlsr.getScheduler().scheduleEvent(ndn::time::seconds(refreshTime),
                                             ndn::bind(&Fib::refreshEntry, this,
                                                       name, feSeqNum));
}

void
Fib::refreshEntry(const ndn::Name& name, int32_t feSeqNum)
{
  std::cout << "Fib::refreshEntry Called" << std::endl;
  std::cout << "Name: " << name << " Seq Num: " << feSeqNum << std::endl;
  std::list<FibEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&fibEntryNameCompare, _1, name));
  if (it != m_table.end()) {
    std::cout << "Entry found with Seq Num: " << feSeqNum << std::endl;
    if (it->getSeqNo() == feSeqNum) {
      std::cout << "Refreshing the FIB entry" << std::endl;
      for (std::list<NextHop>::iterator nhit =
             (*it).getNexthopList().getNextHops().begin();
           nhit != (*it).getNexthopList().getNextHops().end(); nhit++) {
        // add entry to NDN-FIB
        registerPrefix(it->getName(), nhit->getConnectingFaceUri(),
                       std::ceil(nhit->getRouteCost()), m_refreshTime);
      }
      // increase sequence number and schedule refresh again
      it->setSeqNo(feSeqNum + 1);
      it->setExpiringEventId(scheduleEntryRefreshing(it->getName() ,
                                                     it->getSeqNo(),
                                                     m_refreshTime));
    }
  }
}

void
Fib::remove(const ndn::Name& name)
{
  std::list<FibEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&fibEntryNameCompare, _1, name));
  if (it != m_table.end()) {
    for (std::list<NextHop>::iterator nhit =
           (*it).getNexthopList().getNextHops().begin();
         nhit != (*it).getNexthopList().getNextHops().end(); nhit++) {
      //remove entry from NDN-FIB
      if (!m_nlsr.getAdjacencyList().isNeighbor(it->getName())) {
        unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
      }
      else
      {
        if (m_nlsr.getAdjacencyList().getAdjacent(it->getName()).getConnectingFaceUri() !=
            nhit->getConnectingFaceUri()) {
          unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
        }
      }
    }
    std::cout << "Cancellling Scheduled event" << std::endl;
    std::cout << "Name: " << name << "Seq num: " << it->getSeqNo() << std::endl;
    cancelScheduledExpiringEvent((*it).getExpiringEventId());
    m_table.erase(it);
  }
}


void
Fib::update(const ndn::Name& name, NexthopList& nextHopList)
{
  std::cout << "Fib::updateFib Called" << std::endl;
  int startFace = 0;
  int endFace = getNumberOfFacesForName(nextHopList,
                                        m_nlsr.getConfParameter().getMaxFacesPerPrefix());
  std::list<FibEntry>::iterator it = std::find_if(m_table.begin(),
                                                  m_table.end(),
                                                  bind(&fibEntryNameCompare, _1, name));
  if (it == m_table.end()) {
    if (nextHopList.getSize() > 0) {
      nextHopList.sort();
      FibEntry newEntry(name);
      std::list<NextHop> nhl = nextHopList.getNextHops();
      std::list<NextHop>::iterator nhit = nhl.begin();
      for (int i = startFace; i < endFace && nhit != nhl.end(); ++nhit, i++) {
        newEntry.getNexthopList().addNextHop((*nhit));
        //Add entry to NDN-FIB
        registerPrefix(name, nhit->getConnectingFaceUri(),
                       std::ceil(nhit->getRouteCost()), m_refreshTime);
      }
      newEntry.getNexthopList().sort();
      newEntry.setTimeToRefresh(m_refreshTime);
      newEntry.setSeqNo(1);
      newEntry.setExpiringEventId(scheduleEntryRefreshing(name , 1, m_refreshTime));
      m_table.push_back(newEntry);
    }
  }
  else {
    std::cout << "Old FIB Entry" << std::endl;
    if (nextHopList.getSize() > 0) {
      nextHopList.sort();
      if (!it->isEqualNextHops(nextHopList)) {
        std::list<NextHop> nhl = nextHopList.getNextHops();
        std::list<NextHop>::iterator nhit = nhl.begin();
        // Add first Entry to NDN-FIB
        registerPrefix(name, nhit->getConnectingFaceUri(),
                       std::ceil(nhit->getRouteCost()), m_refreshTime);
        removeHop(it->getNexthopList(), nhit->getConnectingFaceUri(), name);
        it->getNexthopList().reset();
        it->getNexthopList().addNextHop((*nhit));
        ++startFace;
        ++nhit;
        for (int i = startFace; i < endFace && nhit != nhl.end(); ++nhit, i++) {
          it->getNexthopList().addNextHop((*nhit));
          //Add Entry to NDN_FIB
          registerPrefix(name, nhit->getConnectingFaceUri(),
                         std::ceil(nhit->getRouteCost()), m_refreshTime);
        }
      }
      it->setTimeToRefresh(m_refreshTime);
      std::cout << "Cancellling Scheduled event" << std::endl;
      std::cout << "Name: " << name << "Seq num: " << it->getSeqNo() << std::endl;
      cancelScheduledExpiringEvent(it->getExpiringEventId());
      it->setSeqNo(it->getSeqNo() + 1);
      (*it).setExpiringEventId(scheduleEntryRefreshing(it->getName() ,
                                                       it->getSeqNo(), m_refreshTime));
    }
    else {
      remove(name);
    }
  }
}



void
Fib::clean()
{
  for (std::list<FibEntry>::iterator it = m_table.begin(); it != m_table.end();
       ++it) {
    std::cout << "Cancellling Scheduled event" << std::endl;
    std::cout << "Name: " << it->getName() << "Seq num: " << it->getSeqNo() <<
              std::endl;
    cancelScheduledExpiringEvent((*it).getExpiringEventId());
    for (std::list<NextHop>::iterator nhit =
           (*it).getNexthopList().getNextHops().begin();
         nhit != (*it).getNexthopList().getNextHops().end(); nhit++) {
      //Remove entry from NDN-FIB
      if (!m_nlsr.getAdjacencyList().isNeighbor(it->getName())) {
        unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
      }
      else {
        if (m_nlsr.getAdjacencyList().getAdjacent(it->getName()).getConnectingFaceUri() !=
            nhit->getConnectingFaceUri()) {
          unregisterPrefix(it->getName(), nhit->getConnectingFaceUri());
        }
      }
    }
  }
  if (m_table.size() > 0) {
    m_table.clear();
  }
}

int
Fib::getNumberOfFacesForName(NexthopList& nextHopList,
                             uint32_t maxFacesPerPrefix)
{
  int endFace = 0;
  if ((maxFacesPerPrefix == 0) || (nextHopList.getSize() <= maxFacesPerPrefix)) {
    return nextHopList.getSize();
  }
  else {
    return maxFacesPerPrefix;
  }
  return endFace;
}

void
Fib::removeHop(NexthopList& nl, const std::string& doNotRemoveHopFaceUri,
               const ndn::Name& name)
{
  for (std::list<NextHop>::iterator it = nl.getNextHops().begin();
       it != nl.getNextHops().end();   ++it) {
    if (it->getConnectingFaceUri() != doNotRemoveHopFaceUri) {
      //Remove FIB Entry from NDN-FIB
      if (!m_nlsr.getAdjacencyList().isNeighbor(name)) {
        unregisterPrefix(name, it->getConnectingFaceUri());
      }
      else {
        if (m_nlsr.getAdjacencyList().getAdjacent(name).getConnectingFaceUri() !=
            it->getConnectingFaceUri()) {
          unregisterPrefix(name, it->getConnectingFaceUri());
        }
      }
    }
  }
}

void
Fib::registerPrefix(const ndn::Name& namePrefix, const std::string& faceUri,
                    uint64_t faceCost, uint64_t timeout)
{
  ndn::nfd::ControlParameters faceParameters;
  faceParameters
  .setUri(faceUri);

  m_controller.start<ndn::nfd::FaceCreateCommand>(faceParameters,
                                                  ndn::bind(&Fib::registerPrefixInNfd, this,_1,
                                                            namePrefix, faceCost, timeout),
                                                  ndn::bind(&Fib::onFailure, this, _1, _2,
                                                             "Failed in name registration"));
  
}

void
Fib::registerPrefixInNfd(const ndn::nfd::ControlParameters& faceCreateResult, 
                         const ndn::Name& namePrefix, uint64_t faceCost, uint64_t timeout)
{
  ndn::nfd::ControlParameters controlParameters;
  controlParameters
    .setName(namePrefix)
    .setFaceId(faceCreateResult.getFaceId())
    .setCost(faceCost)
    .setExpirationPeriod(ndn::time::milliseconds(timeout * 1000))
    .setOrigin(128);
  m_controller.start<ndn::nfd::RibRegisterCommand>(controlParameters,
                                                   ndn::bind(&Fib::onRegistration, this, _1,
                                                             "Successful in name registration",
                                                             faceCreateResult.getUri()),
                                                   ndn::bind(&Fib::onFailure, this, _1, _2,
                                                             "Failed in name registration"));
}

void
Fib::unregisterPrefix(const ndn::Name& namePrefix, const std::string& faceUri)
{
  uint32_t faceId = m_faceMap.getFaceId(faceUri);
  if (faceId > 0) {
    ndn::nfd::ControlParameters controlParameters;
    controlParameters
      .setName(namePrefix)
      .setFaceId(faceId)
      .setOrigin(128);
    m_controller.start<ndn::nfd::RibUnregisterCommand>(controlParameters,
                                                     ndn::bind(&Fib::onSuccess, this, _1,
                                                               "Successful in unregistering name"),
                                                     ndn::bind(&Fib::onFailure, this, _1, _2,
                                                               "Failed in unregistering name"));
  }
}

void
Fib::setStrategy(const ndn::Name& name, const std::string& strategy)
{
  ndn::nfd::ControlParameters parameters;
  parameters
    .setName(name)
    .setStrategy(strategy);

  m_controller.start<ndn::nfd::StrategyChoiceSetCommand>(parameters,
                                                         bind(&Fib::onSuccess, this, _1,
                                                              "Successfully set strategy choice"),
                                                         bind(&Fib::onFailure, this, _1, _2,
                                                              "Failed to set strategy choice"));
}

void
Fib::onRegistration(const ndn::nfd::ControlParameters& commandSuccessResult,
                    const std::string& message, const std::string& faceUri)
{
  //std::cout << message << ": " << commandSuccessResult << std::endl;
  m_faceMap.update(faceUri, commandSuccessResult.getFaceId());
  m_faceMap.print();
}


void
Fib::onSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
               const std::string& message)
{
  //std::cout << message << ": " << commandSuccessResult << std::endl;
}

void
Fib::onFailure(uint32_t code, const std::string& error,
               const std::string& message)
{
  std::cout << message << ": " << error << " (code: " << code << ")";
}


void
Fib::print()
{
  cout << "-------------------FIB-----------------------------" << endl;
  for (std::list<FibEntry>::iterator it = m_table.begin(); it != m_table.end();
       ++it) {
    cout << (*it);
  }
}

} //namespace nlsr
