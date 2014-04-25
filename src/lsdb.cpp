#include <string>
#include <utility>
#include "lsdb.hpp"
#include "nlsr.hpp"

namespace nlsr {

using namespace std;

void
Lsdb::cancelScheduleLsaExpiringEvent(Nlsr& pnlsr, EventId eid)
{
  pnlsr.getScheduler().cancelEvent(eid);
}

static bool
nameLsaCompareByKey(const NameLsa& nlsa1, const string& key)
{
  return nlsa1.getKey() == key;
}


bool
Lsdb::buildAndInstallOwnNameLsa(Nlsr& pnlsr)
{
  NameLsa nameLsa(pnlsr.getConfParameter().getRouterPrefix()
                  , 1
                  , pnlsr.getSequencingManager().getNameLsaSeq() + 1
                  , pnlsr.getConfParameter().getRouterDeadInterval()
                  , pnlsr.getNamePrefixList());
  pnlsr.getSequencingManager().setNameLsaSeq(
    pnlsr.getSequencingManager().getNameLsaSeq() + 1);
  return installNameLsa(pnlsr, nameLsa);
}

NameLsa*
Lsdb::findNameLsa(const string key)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 bind(nameLsaCompareByKey, _1, key));
  if (it != m_nameLsdb.end())
  {
    return &(*it);
  }
  return 0;
}

bool
Lsdb::isNameLsaNew(string key, uint64_t seqNo)
{
  NameLsa* nameLsaCheck = findNameLsa(key);
  if (nameLsaCheck != 0)
  {
    if (nameLsaCheck->getLsSeqNo() < seqNo)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  return true;
}

ndn::EventId
Lsdb::scheduleNameLsaExpiration(Nlsr& pnlsr, string key, int seqNo, int expTime)
{
  return pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(expTime),
                                            ndn::bind(&Lsdb::exprireOrRefreshNameLsa,
                                                      this, boost::ref(pnlsr), key, seqNo));
}

bool
Lsdb::installNameLsa(Nlsr& pnlsr, NameLsa& nlsa)
{
  int timeToExpire = m_lsaRefreshTime;
  NameLsa* chkNameLsa = findNameLsa(nlsa.getKey());
  if (chkNameLsa == 0)
  {
    addNameLsa(nlsa);
    nlsa.writeLog();
    printNameLsdb();
    if (nlsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
    {
      pnlsr.getNamePrefixTable().addNpteByDestName(nlsa.getOrigRouter(),
                                                   nlsa.getOrigRouter(),
                                                   pnlsr);
      std::list<string> nameList = nlsa.getNpl().getNameList();
      for (std::list<string>::iterator it = nameList.begin(); it != nameList.end();
           it++)
      {
        if ((*it) != pnlsr.getConfParameter().getRouterPrefix())
        {
          pnlsr.getNamePrefixTable().addNpteByDestName((*it), nlsa.getOrigRouter(),
                                                       pnlsr);
        }
      }
    }
    if (nlsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
    {
      timeToExpire = nlsa.getLifeTime();
    }
    nlsa.setExpiringEventId(scheduleNameLsaExpiration(pnlsr,
                                                      nlsa.getKey(),
                                                      nlsa.getLsSeqNo(),
                                                      timeToExpire));
  }
  else
  {
    if (chkNameLsa->getLsSeqNo() < nlsa.getLsSeqNo())
    {
      chkNameLsa->writeLog();
      chkNameLsa->setLsSeqNo(nlsa.getLsSeqNo());
      chkNameLsa->setLifeTime(nlsa.getLifeTime());
      chkNameLsa->getNpl().sort();
      nlsa.getNpl().sort();
      std::list<string> nameToAdd;
      std::set_difference(nlsa.getNpl().getNameList().begin(),
                          nlsa.getNpl().getNameList().end(),
                          chkNameLsa->getNpl().getNameList().begin(),
                          chkNameLsa->getNpl().getNameList().end(),
                          std::inserter(nameToAdd, nameToAdd.begin()));
      for (std::list<string>::iterator it = nameToAdd.begin(); it != nameToAdd.end();
           ++it)
      {
        chkNameLsa->addName((*it));
        if (nlsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
        {
          if ((*it) != pnlsr.getConfParameter().getRouterPrefix())
          {
            pnlsr.getNamePrefixTable().addNpteByDestName((*it), nlsa.getOrigRouter(),
                                                         pnlsr);
          }
        }
      }
      std::list<string> nameToRemove;
      std::set_difference(chkNameLsa->getNpl().getNameList().begin(),
                          chkNameLsa->getNpl().getNameList().end(),
                          nlsa.getNpl().getNameList().begin(),
                          nlsa.getNpl().getNameList().end(),
                          std::inserter(nameToRemove, nameToRemove.begin()));
      for (std::list<string>::iterator it = nameToRemove.begin();
           it != nameToRemove.end(); ++it)
      {
        chkNameLsa->removeName((*it));
        if (nlsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
        {
          if ((*it) != pnlsr.getConfParameter().getRouterPrefix())
          {
            pnlsr.getNamePrefixTable().removeNpte((*it), nlsa.getOrigRouter(), pnlsr);
          }
        }
      }
      if (nlsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
      {
        timeToExpire = nlsa.getLifeTime();
      }
      cancelScheduleLsaExpiringEvent(pnlsr,
                                     chkNameLsa->getExpiringEventId());
      chkNameLsa->setExpiringEventId(scheduleNameLsaExpiration(pnlsr,
                                                               nlsa.getKey(),
                                                               nlsa.getLsSeqNo(),
                                                               timeToExpire));
      chkNameLsa->writeLog();
    }
  }
  return true;
}

bool
Lsdb::addNameLsa(NameLsa& nlsa)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 bind(nameLsaCompareByKey, _1,
                                                      nlsa.getKey()));
  if (it == m_nameLsdb.end())
  {
    m_nameLsdb.push_back(nlsa);
    return true;
  }
  return false;
}

bool
Lsdb::removeNameLsa(Nlsr& pnlsr, string& key)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 bind(nameLsaCompareByKey, _1, key));
  if (it != m_nameLsdb.end())
  {
    (*it).writeLog();
    if ((*it).getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
    {
      pnlsr.getNamePrefixTable().removeNpte((*it).getOrigRouter(),
                                            (*it).getOrigRouter(), pnlsr);
      for (std::list<string>::iterator nit = (*it).getNpl().getNameList().begin();
           nit != (*it).getNpl().getNameList().end(); ++nit)
      {
        if ((*nit) != pnlsr.getConfParameter().getRouterPrefix())
        {
          pnlsr.getNamePrefixTable().removeNpte((*nit), (*it).getOrigRouter(), pnlsr);
        }
      }
    }
    m_nameLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesNameLsaExist(string key)
{
  std::list<NameLsa>::iterator it = std::find_if(m_nameLsdb.begin(),
                                                 m_nameLsdb.end(),
                                                 bind(nameLsaCompareByKey, _1, key));
  if (it == m_nameLsdb.end())
  {
    return false;
  }
  return true;
}

void
Lsdb::printNameLsdb()
{
  cout << "---------------Name LSDB-------------------" << endl;
  for (std::list<NameLsa>::iterator it = m_nameLsdb.begin();
       it != m_nameLsdb.end() ; it++)
  {
    cout << (*it) << endl;
  }
}

// Cor LSA and LSDB related Functions start here


static bool
corLsaCompareByKey(const CoordinateLsa& clsa, const string& key)
{
  return clsa.getKey() == key;
}

bool
Lsdb::buildAndInstallOwnCoordinateLsa(Nlsr& pnlsr)
{
  CoordinateLsa corLsa(pnlsr.getConfParameter().getRouterPrefix()
                       , 3
                       , pnlsr.getSequencingManager().getCorLsaSeq() + 1
                       , pnlsr.getConfParameter().getRouterDeadInterval()
                       , pnlsr.getConfParameter().getCorR()
                       , pnlsr.getConfParameter().getCorTheta());
  pnlsr.getSequencingManager().setCorLsaSeq(
    pnlsr.getSequencingManager().getCorLsaSeq() + 1);
  installCoordinateLsa(pnlsr, corLsa);
  return true;
}

CoordinateLsa*
Lsdb::findCoordinateLsa(const string& key)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       bind(corLsaCompareByKey, _1, key));
  if (it != m_corLsdb.end())
  {
    return &(*it);
  }
  return 0;
}

bool
Lsdb::isCoordinateLsaNew(const string& key, uint64_t seqNo)
{
  CoordinateLsa* clsa = findCoordinateLsa(key);
  if (clsa != 0)
  {
    if (clsa->getLsSeqNo() < seqNo)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  return true;
}

ndn::EventId
Lsdb::scheduleCoordinateLsaExpiration(Nlsr& pnlsr, const string& key, int seqNo,
                                      int expTime)
{
  return pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(expTime),
                                            ndn::bind(&Lsdb::exprireOrRefreshCoordinateLsa,
                                                      this, boost::ref(pnlsr),
                                                      key, seqNo));
}

bool
Lsdb::installCoordinateLsa(Nlsr& pnlsr, CoordinateLsa& clsa)
{
  int timeToExpire = m_lsaRefreshTime;
  CoordinateLsa* chkCorLsa = findCoordinateLsa(clsa.getKey());
  if (chkCorLsa == 0)
  {
    addCoordinateLsa(clsa);
    printCorLsdb(); //debugging purpose
    if (clsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
    {
      pnlsr.getNamePrefixTable().addNpteByDestName(clsa.getOrigRouter(),
                                                   clsa.getOrigRouter(),
                                                   pnlsr);
    }
    if (pnlsr.getConfParameter().getIsHyperbolicCalc() >= 1)
    {
      pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
    }
    if (clsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
    {
      timeToExpire = clsa.getLifeTime();
    }
    scheduleCoordinateLsaExpiration(pnlsr, clsa.getKey(),
                                    clsa.getLsSeqNo(), timeToExpire);
  }
  else
  {
    if (chkCorLsa->getLsSeqNo() < clsa.getLsSeqNo())
    {
      chkCorLsa->setLsSeqNo(clsa.getLsSeqNo());
      chkCorLsa->setLifeTime(clsa.getLifeTime());
      if (!chkCorLsa->isEqual(clsa))
      {
        chkCorLsa->setCorRadius(clsa.getCorRadius());
        chkCorLsa->setCorTheta(clsa.getCorTheta());
        if (pnlsr.getConfParameter().getIsHyperbolicCalc() >= 1)
        {
          pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
        }
      }
      if (clsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
      {
        timeToExpire = clsa.getLifeTime();
      }
      cancelScheduleLsaExpiringEvent(pnlsr,
                                     chkCorLsa->getExpiringEventId());
      chkCorLsa->setExpiringEventId(scheduleCoordinateLsaExpiration(pnlsr,
                                                                    clsa.getKey(),
                                                                    clsa.getLsSeqNo(),
                                                                    timeToExpire));
    }
  }
  return true;
}

bool
Lsdb::addCoordinateLsa(CoordinateLsa& clsa)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       bind(corLsaCompareByKey, _1,
                                                            clsa.getKey()));
  if (it == m_corLsdb.end())
  {
    m_corLsdb.push_back(clsa);
    return true;
  }
  return false;
}

bool
Lsdb::removeCoordinateLsa(Nlsr& pnlsr, const string& key)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       bind(corLsaCompareByKey, _1, key));
  if (it != m_corLsdb.end())
  {
    if ((*it).getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
    {
      pnlsr.getNamePrefixTable().removeNpte((*it).getOrigRouter(),
                                            (*it).getOrigRouter(), pnlsr);
    }
    m_corLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesCoordinateLsaExist(const string& key)
{
  std::list<CoordinateLsa>::iterator it = std::find_if(m_corLsdb.begin(),
                                                       m_corLsdb.end(),
                                                       bind(corLsaCompareByKey, _1, key));
  if (it == m_corLsdb.end())
  {
    return false;
  }
  return true;
}

void
Lsdb::printCorLsdb() //debugging
{
  cout << "---------------Cor LSDB-------------------" << endl;
  for (std::list<CoordinateLsa>::iterator it = m_corLsdb.begin();
       it != m_corLsdb.end() ; it++)
  {
    cout << (*it) << endl;
  }
}


// Adj LSA and LSDB related function starts here

static bool
adjLsaCompareByKey(AdjLsa& alsa, string& key)
{
  return alsa.getKey() == key;
}


void
Lsdb::scheduledAdjLsaBuild(Nlsr& pnlsr)
{
  cout << "scheduledAdjLsaBuild Called" << endl;
  pnlsr.setIsBuildAdjLsaSheduled(0);
  if (pnlsr.getAdjacencyList().isAdjLsaBuildable(pnlsr))
  {
    int adjBuildCount = pnlsr.getAdjBuildCount();
    if (adjBuildCount > 0)
    {
      if (pnlsr.getAdjacencyList().getNumOfActiveNeighbor() > 0)
      {
        buildAndInstallOwnAdjLsa(pnlsr);
      }
      else
      {
        string key = pnlsr.getConfParameter().getRouterPrefix() + "/2";
        removeAdjLsa(pnlsr, key);
        pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
      }
      pnlsr.setAdjBuildCount(pnlsr.getAdjBuildCount() - adjBuildCount);
    }
  }
  else
  {
    pnlsr.setIsBuildAdjLsaSheduled(1);
    int schedulingTime = pnlsr.getConfParameter().getInterestRetryNumber() *
                         pnlsr.getConfParameter().getInterestResendTime();
    pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(schedulingTime),
                                       ndn::bind(&Lsdb::scheduledAdjLsaBuild,
                                                 pnlsr.getLsdb(), boost::ref(pnlsr)));
  }
}


bool
Lsdb::addAdjLsa(AdjLsa& alsa)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                bind(adjLsaCompareByKey, _1,
                                                     alsa.getKey()));
  if (it == m_adjLsdb.end())
  {
    m_adjLsdb.push_back(alsa);
    return true;
  }
  return false;
}

AdjLsa*
Lsdb::findAdjLsa(const string key)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                bind(adjLsaCompareByKey, _1, key));
  if (it != m_adjLsdb.end())
  {
    return &(*it);
  }
  return 0;
}


bool
Lsdb::isAdjLsaNew(string key, uint64_t seqNo)
{
  AdjLsa*  adjLsaCheck = findAdjLsa(key);
  if (adjLsaCheck != 0)
  {
    if (adjLsaCheck->getLsSeqNo() < seqNo)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  return true;
}


ndn::EventId
Lsdb::scheduleAdjLsaExpiration(Nlsr& pnlsr, string key, int seqNo, int expTime)
{
  return pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(expTime),
                                            ndn::bind(&Lsdb::exprireOrRefreshAdjLsa,
                                                      this, boost::ref(pnlsr),
                                                      key, seqNo));
}

bool
Lsdb::installAdjLsa(Nlsr& pnlsr, AdjLsa& alsa)
{
  int timeToExpire = m_lsaRefreshTime;
  AdjLsa* chkAdjLsa = findAdjLsa(alsa.getKey());
  if (chkAdjLsa == 0)
  {
    addAdjLsa(alsa);
    alsa.addNptEntries(pnlsr);
    pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
    if (alsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
    {
      timeToExpire = alsa.getLifeTime();
    }
    scheduleAdjLsaExpiration(pnlsr, alsa.getKey(),
                             alsa.getLsSeqNo(), timeToExpire);
  }
  else
  {
    if (chkAdjLsa->getLsSeqNo() < alsa.getLsSeqNo())
    {
      chkAdjLsa->setLsSeqNo(alsa.getLsSeqNo());
      chkAdjLsa->setLifeTime(alsa.getLifeTime());
      if (!chkAdjLsa->isEqual(alsa))
      {
        chkAdjLsa->getAdl().reset();
        chkAdjLsa->getAdl().addAdjacentsFromAdl(alsa.getAdl());
        pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
      }
      if (alsa.getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
      {
        timeToExpire = alsa.getLifeTime();
      }
      cancelScheduleLsaExpiringEvent(pnlsr, chkAdjLsa->getExpiringEventId());
      chkAdjLsa->setExpiringEventId(scheduleAdjLsaExpiration(pnlsr,
                                                             alsa.getKey(),
                                                             alsa.getLsSeqNo(),
                                                             timeToExpire));
    }
  }
  return true;
}

bool
Lsdb::buildAndInstallOwnAdjLsa(Nlsr& pnlsr)
{
  AdjLsa adjLsa(pnlsr.getConfParameter().getRouterPrefix()
                , 2
                , pnlsr.getSequencingManager().getAdjLsaSeq() + 1
                , pnlsr.getConfParameter().getRouterDeadInterval()
                , pnlsr.getAdjacencyList().getNumOfActiveNeighbor()
                , pnlsr.getAdjacencyList());
  pnlsr.getSequencingManager().setAdjLsaSeq(
    pnlsr.getSequencingManager().getAdjLsaSeq() + 1);
  string lsaPrefix = pnlsr.getConfParameter().getChronosyncLsaPrefix()
                     + pnlsr.getConfParameter().getRouterPrefix();
  pnlsr.getSyncLogicHandler().publishRoutingUpdate(pnlsr.getSequencingManager(),
                                                   lsaPrefix);
  return pnlsr.getLsdb().installAdjLsa(pnlsr, adjLsa);
}

bool
Lsdb::removeAdjLsa(Nlsr& pnlsr, string& key)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                bind(adjLsaCompareByKey, _1, key));
  if (it != m_adjLsdb.end())
  {
    (*it).removeNptEntries(pnlsr);
    m_adjLsdb.erase(it);
    return true;
  }
  return false;
}

bool
Lsdb::doesAdjLsaExist(string key)
{
  std::list<AdjLsa>::iterator it = std::find_if(m_adjLsdb.begin(),
                                                m_adjLsdb.end(),
                                                bind(adjLsaCompareByKey, _1, key));
  if (it == m_adjLsdb.end())
  {
    return false;
  }
  return true;
}

std::list<AdjLsa>&
Lsdb::getAdjLsdb()
{
  return m_adjLsdb;
}

void
Lsdb::setLsaRefreshTime(int lrt)
{
  m_lsaRefreshTime = lrt;
}

void
Lsdb::setThisRouterPrefix(string trp)
{
  m_thisRouterPrefix = trp;
}

void
Lsdb::exprireOrRefreshNameLsa(Nlsr& pnlsr, string lsaKey, uint64_t seqNo)
{
  cout << "Lsdb::exprireOrRefreshNameLsa Called " << endl;
  cout << "LSA Key : " << lsaKey << " Seq No: " << seqNo << endl;
  NameLsa* chkNameLsa = findNameLsa(lsaKey);
  if (chkNameLsa != 0)
  {
    cout << " LSA Exists with seq no: " << chkNameLsa->getLsSeqNo() << endl;
    if (chkNameLsa->getLsSeqNo() == seqNo)
    {
      if (chkNameLsa->getOrigRouter() == m_thisRouterPrefix)
      {
        chkNameLsa->writeLog();
        cout << "Own Name LSA, so refreshing name LSA" << endl;
        chkNameLsa->setLsSeqNo(chkNameLsa->getLsSeqNo() + 1);
        pnlsr.getSequencingManager().setNameLsaSeq(chkNameLsa->getLsSeqNo());
        chkNameLsa->writeLog();
        // publish routing update
        string lsaPrefix = pnlsr.getConfParameter().getChronosyncLsaPrefix()
                           + pnlsr.getConfParameter().getRouterPrefix();
        pnlsr.getSyncLogicHandler().publishRoutingUpdate(pnlsr.getSequencingManager(),
                                                         lsaPrefix);
      }
      else
      {
        cout << "Other's Name LSA, so removing form LSDB" << endl;
        removeNameLsa(pnlsr, lsaKey);
      }
    }
  }
}

void
Lsdb::exprireOrRefreshAdjLsa(Nlsr& pnlsr, string lsaKey, uint64_t seqNo)
{
  cout << "Lsdb::exprireOrRefreshAdjLsa Called " << endl;
  cout << "LSA Key : " << lsaKey << " Seq No: " << seqNo << endl;
  AdjLsa* chkAdjLsa = findAdjLsa(lsaKey);
  if (chkAdjLsa != 0)
  {
    cout << " LSA Exists with seq no: " << chkAdjLsa->getLsSeqNo() << endl;
    if (chkAdjLsa->getLsSeqNo() == seqNo)
    {
      if (chkAdjLsa->getOrigRouter() == m_thisRouterPrefix)
      {
        cout << "Own Adj LSA, so refreshing Adj LSA" << endl;
        chkAdjLsa->setLsSeqNo(chkAdjLsa->getLsSeqNo() + 1);
        pnlsr.getSequencingManager().setAdjLsaSeq(chkAdjLsa->getLsSeqNo());
        // publish routing update
        string lsaPrefix = pnlsr.getConfParameter().getChronosyncLsaPrefix()
                           + pnlsr.getConfParameter().getRouterPrefix();
        pnlsr.getSyncLogicHandler().publishRoutingUpdate(pnlsr.getSequencingManager(),
                                                         lsaPrefix);
      }
      else
      {
        cout << "Other's Adj LSA, so removing form LSDB" << endl;
        removeAdjLsa(pnlsr, lsaKey);
      }
      // schedule Routing table calculaiton
      pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
    }
  }
}

void
Lsdb::exprireOrRefreshCoordinateLsa(Nlsr& pnlsr, const string& lsaKey,
                                    uint64_t seqNo)
{
  cout << "Lsdb::exprireOrRefreshCorLsa Called " << endl;
  cout << "LSA Key : " << lsaKey << " Seq No: " << seqNo << endl;
  CoordinateLsa* chkCorLsa = findCoordinateLsa(lsaKey);
  if (chkCorLsa != 0)
  {
    cout << " LSA Exists with seq no: " << chkCorLsa->getLsSeqNo() << endl;
    if (chkCorLsa->getLsSeqNo() == seqNo)
    {
      if (chkCorLsa->getOrigRouter() == m_thisRouterPrefix)
      {
        cout << "Own Cor LSA, so refreshing Cor LSA" << endl;
        chkCorLsa->setLsSeqNo(chkCorLsa->getLsSeqNo() + 1);
        pnlsr.getSequencingManager().setCorLsaSeq(chkCorLsa->getLsSeqNo());
        // publish routing update
        string lsaPrefix = pnlsr.getConfParameter().getChronosyncLsaPrefix()
                           + pnlsr.getConfParameter().getRouterPrefix();
        pnlsr.getSyncLogicHandler().publishRoutingUpdate(pnlsr.getSequencingManager(),
                                                         lsaPrefix);
      }
      else
      {
        cout << "Other's Cor LSA, so removing form LSDB" << endl;
        removeCoordinateLsa(pnlsr, lsaKey);
      }
      if (pnlsr.getConfParameter().getIsHyperbolicCalc() >= 1)
      {
        pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
      }
    }
  }
}


void
Lsdb::printAdjLsdb()
{
  cout << "---------------Adj LSDB-------------------" << endl;
  for (std::list<AdjLsa>::iterator it = m_adjLsdb.begin();
       it != m_adjLsdb.end() ; it++)
  {
    cout << (*it) << endl;
  }
}

//-----utility function -----
bool
Lsdb::doesLsaExist(string key, int lsType)
{
  if (lsType == 1)
  {
    return doesNameLsaExist(key);
  }
  else if (lsType == 2)
  {
    return doesAdjLsaExist(key);
  }
  else if (lsType == 3)
  {
    return doesCoordinateLsaExist(key);
  }
  return false;
}

}//namespace nlsr

