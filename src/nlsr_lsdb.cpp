#include<string>
#include<utility>
#include "nlsr_lsdb.hpp"
#include "nlsr.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_lsdb.cpp"

namespace nlsr
{

  using namespace std;

  void
  Lsdb::cancelScheduleLsaExpiringEvent(Nlsr& pnlsr, EventId eid)
  {
    pnlsr.getScheduler().cancelEvent(eid);
  }

  static bool
  nameLsaCompareByKey(NameLsa& nlsa1, string& key)
  {
    return nlsa1.getKey()==key;
  }


  bool
  Lsdb::buildAndInstallOwnNameLsa(Nlsr& pnlsr)
  {
    NameLsa nameLsa(pnlsr.getConfParameter().getRouterPrefix()
                    , 1
                    , pnlsr.getSm().getNameLsaSeq()+1
                    , pnlsr.getConfParameter().getRouterDeadInterval()
                    , pnlsr.getNpl() );
    pnlsr.getSm().setNameLsaSeq(pnlsr.getSm().getNameLsaSeq()+1);
    return installNameLsa(pnlsr,nameLsa);
  }

  std::pair<NameLsa&, bool>
  Lsdb::getNameLsa(string key)
  {
    std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(),
                                       nameLsdb.end(),
                                       bind(nameLsaCompareByKey, _1, key));
    if( it != nameLsdb.end())
    {
      return std::make_pair(boost::ref((*it)),true);
    }
    NameLsa nlsa;
    return std::make_pair(boost::ref(nlsa),false);
  }

  bool
  Lsdb::isNameLsaNew(string key, uint64_t seqNo)
  {
    std::pair<NameLsa& , bool>  nameLsaCheck=getNameLsa(key);
    if(nameLsaCheck.second)
    {
      if(nameLsaCheck.first.getLsSeqNo() < seqNo)
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
                     this,boost::ref(pnlsr), key, seqNo));
  }

  bool
  Lsdb::installNameLsa(Nlsr& pnlsr, NameLsa &nlsa)
  {
    src::logger lg;
    int timeToExpire=lsaRefreshTime;
    std::pair<NameLsa& , bool> chkNameLsa=getNameLsa(nlsa.getKey());
    if ( !chkNameLsa.second )
    {
      BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"Adding name lsa";
      addNameLsa(nlsa);
      nlsa.writeLog();
      printNameLsdb();
      if ( nlsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
      {
        pnlsr.getNpt().addNpteByDestName(nlsa.getOrigRouter(),nlsa.getOrigRouter(),
                                         pnlsr);
        std::list<string> nameList=nlsa.getNpl().getNameList();
        for(std::list<string>::iterator it=nameList.begin(); it!=nameList.end(); it++)
        {
          if ( (*it) !=pnlsr.getConfParameter().getRouterPrefix())
          {
            pnlsr.getNpt().addNpteByDestName((*it),nlsa.getOrigRouter(),pnlsr);
          }
        }
      }
      if(nlsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
      {
        timeToExpire=nlsa.getLifeTime();
      }
      nlsa.setExpiringEventId(scheduleNameLsaExpiration( pnlsr,
                                 nlsa.getKey(), nlsa.getLsSeqNo(), timeToExpire));
    }
    else
    {
      if ( chkNameLsa.first.getLsSeqNo() < nlsa.getLsSeqNo() )
      {
        BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"Deleting name lsa";
        chkNameLsa.first.writeLog();
        chkNameLsa.first.setLsSeqNo(nlsa.getLsSeqNo());
        chkNameLsa.first.setLifeTime(nlsa.getLifeTime());
        chkNameLsa.first.getNpl().sort();
        nlsa.getNpl().sort();
        std::list<string> nameToAdd;
        std::set_difference(nlsa.getNpl().getNameList().begin(),
                            nlsa.getNpl().getNameList().end(),
                            chkNameLsa.first.getNpl().getNameList().begin(),
                            chkNameLsa.first.getNpl().getNameList().end(),
                            std::inserter(nameToAdd, nameToAdd.begin()));
        for(std::list<string>::iterator it=nameToAdd.begin(); it!=nameToAdd.end();
            ++it)
        {
          chkNameLsa.first.addName((*it));
          if ( nlsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
          {
            if ( (*it) !=pnlsr.getConfParameter().getRouterPrefix())
            {
              pnlsr.getNpt().addNpteByDestName((*it),nlsa.getOrigRouter(),pnlsr);
            }
          }
        }
        std::list<string> nameToRemove;
        std::set_difference(chkNameLsa.first.getNpl().getNameList().begin(),
                            chkNameLsa.first.getNpl().getNameList().end(),
                            nlsa.getNpl().getNameList().begin(),
                            nlsa.getNpl().getNameList().end(),
                            std::inserter(nameToRemove, nameToRemove.begin()));
        for(std::list<string>::iterator it=nameToRemove.begin();
            it!=nameToRemove.end(); ++it)
        {
          chkNameLsa.first.removeName((*it));
          if ( nlsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
          {
            if ( (*it) !=pnlsr.getConfParameter().getRouterPrefix())
            {
              pnlsr.getNpt().removeNpte((*it),nlsa.getOrigRouter(),pnlsr);
            }
          }
        }
        if(nlsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
        {
          timeToExpire=nlsa.getLifeTime();
        }
        cancelScheduleLsaExpiringEvent(pnlsr,
                                       chkNameLsa.first.getExpiringEventId());
        chkNameLsa.first.setExpiringEventId(scheduleNameLsaExpiration( pnlsr,
                                               nlsa.getKey(), nlsa.getLsSeqNo(), timeToExpire));
        BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"Adding name lsa";
        chkNameLsa.first.writeLog();
      }
    }
    return true;
  }

  bool
  Lsdb::addNameLsa(NameLsa &nlsa)
  {
    std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(),
                                       nameLsdb.end(), bind(nameLsaCompareByKey, _1, nlsa.getKey()));
    if( it == nameLsdb.end())
    {
      nameLsdb.push_back(nlsa);
      return true;
    }
    return false;
  }

  bool
  Lsdb::removeNameLsa(Nlsr& pnlsr, string& key)
  {
    src::logger lg;
    std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(),
                                       nameLsdb.end(),
                                       bind(nameLsaCompareByKey, _1, key));
    if ( it != nameLsdb.end() )
    {
      BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"Deleting name lsa";
      (*it).writeLog();
      if ( (*it).getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix()  )
      {
        pnlsr.getNpt().removeNpte((*it).getOrigRouter(),(*it).getOrigRouter(),pnlsr);
        for( std::list<string>::iterator nit=(*it).getNpl().getNameList().begin();
             nit!=(*it).getNpl().getNameList().end(); ++nit)
        {
          if ( (*nit) !=pnlsr.getConfParameter().getRouterPrefix())
          {
            pnlsr.getNpt().removeNpte((*nit),(*it).getOrigRouter(),pnlsr);
          }
        }
      }
      nameLsdb.erase(it);
      return true;
    }
    return false;
  }

  bool
  Lsdb::doesNameLsaExist(string key)
  {
    std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(),
                                       nameLsdb.end(),
                                       bind(nameLsaCompareByKey, _1, key));
    if( it == nameLsdb.end())
    {
      return false;
    }
    return true;
  }

  void
  Lsdb::printNameLsdb()
  {
    cout<<"---------------Name LSDB-------------------"<<endl;
    for( std::list<NameLsa>::iterator it=nameLsdb.begin();
         it!= nameLsdb.end() ; it++)
    {
      cout<< (*it) <<endl;
    }
  }

// Cor LSA and LSDB related Functions start here


  static bool
  corLsaCompareByKey(CorLsa& clsa, string& key)
  {
    return clsa.getKey()==key;
  }

  bool
  Lsdb::buildAndInstallOwnCorLsa(Nlsr& pnlsr)
  {
    CorLsa corLsa(pnlsr.getConfParameter().getRouterPrefix()
                  , 3
                  , pnlsr.getSm().getCorLsaSeq()+1
                  , pnlsr.getConfParameter().getRouterDeadInterval()
                  , pnlsr.getConfParameter().getCorR()
                  , pnlsr.getConfParameter().getCorTheta() );
    pnlsr.getSm().setCorLsaSeq(pnlsr.getSm().getCorLsaSeq()+1);
    installCorLsa(pnlsr, corLsa);
    return true;
  }

  std::pair<CorLsa&, bool>
  Lsdb::getCorLsa(string key)
  {
    std::list< CorLsa >::iterator it = std::find_if( corLsdb.begin(),
                                       corLsdb.end(),
                                       bind(corLsaCompareByKey, _1, key));
    if( it != corLsdb.end())
    {
      return std::make_pair(boost::ref((*it)), true);
    }
    CorLsa clsa;
    return std::make_pair(boost::ref(clsa),false);
  }

  bool
  Lsdb::isCorLsaNew(string key, uint64_t seqNo)
  {
    std::pair<CorLsa& , bool>  corLsaCheck=getCorLsa(key);
    if(corLsaCheck.second)
    {
      if(corLsaCheck.first.getLsSeqNo() < seqNo)
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
  Lsdb::scheduleCorLsaExpiration(Nlsr& pnlsr, string key, int seqNo, int expTime)
  {
    return pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(expTime),
           ndn::bind(&Lsdb::exprireOrRefreshCorLsa,
                     this,boost::ref(pnlsr),key,seqNo));
  }

  bool
  Lsdb::installCorLsa(Nlsr& pnlsr, CorLsa &clsa)
  {
    int timeToExpire=lsaRefreshTime;
    std::pair<CorLsa& , bool> chkCorLsa=getCorLsa(clsa.getKey());
    if ( !chkCorLsa.second )
    {
      addCorLsa(clsa);
      printCorLsdb(); //debugging purpose
      if ( clsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
      {
        pnlsr.getNpt().addNpteByDestName(clsa.getOrigRouter(),clsa.getOrigRouter(),
                                         pnlsr);
      }
      if (pnlsr.getConfParameter().getIsHyperbolicCalc() >=1 )
      {
        pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
      }
      if(clsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
      {
        timeToExpire=clsa.getLifeTime();
      }
      scheduleCorLsaExpiration(pnlsr,clsa.getKey(),
                               clsa.getLsSeqNo(), timeToExpire);
    }
    else
    {
      if ( chkCorLsa.first.getLsSeqNo() < clsa.getLsSeqNo() )
      {
        chkCorLsa.first.setLsSeqNo(clsa.getLsSeqNo());
        chkCorLsa.first.setLifeTime(clsa.getLifeTime());
        if ( !chkCorLsa.first.isEqual(clsa) )
        {
          chkCorLsa.first.setCorRadius(clsa.getCorRadius());
          chkCorLsa.first.setCorTheta(clsa.getCorTheta());
          if (pnlsr.getConfParameter().getIsHyperbolicCalc() >=1 )
          {
            pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
          }
        }
        if(clsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
        {
          timeToExpire=clsa.getLifeTime();
        }
        cancelScheduleLsaExpiringEvent(pnlsr,
                                       chkCorLsa.first.getExpiringEventId());
        chkCorLsa.first.setExpiringEventId(scheduleCorLsaExpiration(pnlsr,
                                              clsa.getKey(),
                                              clsa.getLsSeqNo(), timeToExpire));
      }
    }
    return true;
  }

  bool
  Lsdb::addCorLsa(CorLsa& clsa)
  {
    std::list<CorLsa >::iterator it = std::find_if( corLsdb.begin(),
                                      corLsdb.end(),
                                      bind(corLsaCompareByKey, _1, clsa.getKey()));
    if( it == corLsdb.end())
    {
      corLsdb.push_back(clsa);
      return true;
    }
    return false;
  }

  bool
  Lsdb::removeCorLsa(Nlsr& pnlsr, string& key)
  {
    std::list<CorLsa >::iterator it = std::find_if( corLsdb.begin(),
                                      corLsdb.end(),
                                      bind(corLsaCompareByKey, _1, key));
    if ( it != corLsdb.end() )
    {
      if ( (*it).getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
      {
        pnlsr.getNpt().removeNpte((*it).getOrigRouter(),(*it).getOrigRouter(),pnlsr);
      }
      corLsdb.erase(it);
      return true;
    }
    return false;
  }

  bool
  Lsdb::doesCorLsaExist(string key)
  {
    std::list<CorLsa >::iterator it = std::find_if( corLsdb.begin(),
                                      corLsdb.end(),
                                      bind(corLsaCompareByKey, _1, key));
    if( it == corLsdb.end())
    {
      return false;
    }
    return true;
  }

  void
  Lsdb::printCorLsdb() //debugging
  {
    cout<<"---------------Cor LSDB-------------------"<<endl;
    for( std::list<CorLsa>::iterator it=corLsdb.begin();
         it!= corLsdb.end() ; it++)
    {
      cout<< (*it) <<endl;
    }
  }


// Adj LSA and LSDB related function starts here

  static bool
  adjLsaCompareByKey(AdjLsa& alsa, string& key)
  {
    return alsa.getKey()==key;
  }


  void
  Lsdb::scheduledAdjLsaBuild(Nlsr& pnlsr)
  {
    cout<<"scheduledAdjLsaBuild Called"<<endl;
    pnlsr.setIsBuildAdjLsaSheduled(0);
    if( pnlsr.getAdl().isAdjLsaBuildable(pnlsr))
    {
      int adjBuildCount=pnlsr.getAdjBuildCount();
      if(adjBuildCount>0 )
      {
        if (pnlsr.getAdl().getNumOfActiveNeighbor()>0)
        {
          buildAndInstallOwnAdjLsa(pnlsr);
        }
        else
        {
          string key=pnlsr.getConfParameter().getRouterPrefix()+"/2";
          removeAdjLsa(pnlsr,key);
          pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
        }
        pnlsr.setAdjBuildCount(pnlsr.getAdjBuildCount()-adjBuildCount);
      }
    }
    else
    {
      pnlsr.setIsBuildAdjLsaSheduled(1);
      int schedulingTime=pnlsr.getConfParameter().getInterestRetryNumber()*
                         pnlsr.getConfParameter().getInterestResendTime();
      pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(schedulingTime),
                                         ndn::bind(&Lsdb::scheduledAdjLsaBuild, pnlsr.getLsdb(),
                                             boost::ref(pnlsr)));
    }
  }


  bool
  Lsdb::addAdjLsa(AdjLsa &alsa)
  {
    std::list<AdjLsa >::iterator it = std::find_if( adjLsdb.begin(),
                                      adjLsdb.end(),
                                      bind(adjLsaCompareByKey, _1, alsa.getKey()));
    if( it == adjLsdb.end())
    {
      adjLsdb.push_back(alsa);
      return true;
    }
    return false;
  }

  std::pair<AdjLsa& , bool>
  Lsdb::getAdjLsa(string key)
  {
    std::list<AdjLsa >::iterator it = std::find_if( adjLsdb.begin(),
                                      adjLsdb.end(),
                                      bind(adjLsaCompareByKey, _1, key));
    if( it != adjLsdb.end())
    {
      return std::make_pair(boost::ref((*it)),true);
    }
    AdjLsa alsa;
    return std::make_pair(boost::ref(alsa),false);
  }


  bool
  Lsdb::isAdjLsaNew(string key, uint64_t seqNo)
  {
    std::pair<AdjLsa& , bool>  adjLsaCheck=getAdjLsa(key);
    if(adjLsaCheck.second)
    {
      if(adjLsaCheck.first.getLsSeqNo() < seqNo)
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
                     this,boost::ref(pnlsr),key,seqNo));
  }

  bool
  Lsdb::installAdjLsa(Nlsr& pnlsr, AdjLsa &alsa)
  {
    int timeToExpire=lsaRefreshTime;
    std::pair<AdjLsa& , bool> chkAdjLsa=getAdjLsa(alsa.getKey());
    if ( !chkAdjLsa.second )
    {
      addAdjLsa(alsa);
      alsa.addNptEntries(pnlsr);
      pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
      if(alsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
      {
        timeToExpire=alsa.getLifeTime();
      }
      scheduleAdjLsaExpiration(pnlsr,alsa.getKey(),
                               alsa.getLsSeqNo(),timeToExpire);
    }
    else
    {
      if ( chkAdjLsa.first.getLsSeqNo() < alsa.getLsSeqNo() )
      {
        chkAdjLsa.first.setLsSeqNo(alsa.getLsSeqNo());
        chkAdjLsa.first.setLifeTime(alsa.getLifeTime());
        if ( !	chkAdjLsa.first.isEqual(alsa))
        {
          chkAdjLsa.first.getAdl().reset();
          chkAdjLsa.first.getAdl().addAdjacentsFromAdl(alsa.getAdl());
          pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
        }
        if(alsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
        {
          timeToExpire=alsa.getLifeTime();
        }
        cancelScheduleLsaExpiringEvent(pnlsr,
                                       chkAdjLsa.first.getExpiringEventId());
        chkAdjLsa.first.setExpiringEventId(scheduleAdjLsaExpiration(pnlsr,
                                              alsa.getKey(), alsa.getLsSeqNo(),timeToExpire));
      }
    }
    return true;
  }

  bool
  Lsdb::buildAndInstallOwnAdjLsa(Nlsr& pnlsr)
  {
    AdjLsa adjLsa(pnlsr.getConfParameter().getRouterPrefix()
                  , 2
                  , pnlsr.getSm().getAdjLsaSeq()+1
                  , pnlsr.getConfParameter().getRouterDeadInterval()
                  , pnlsr.getAdl().getNumOfActiveNeighbor()
                  , pnlsr.getAdl() );
    pnlsr.getSm().setAdjLsaSeq(pnlsr.getSm().getAdjLsaSeq()+1);
    string lsaPrefix=pnlsr.getConfParameter().getChronosyncLsaPrefix()
                     + pnlsr.getConfParameter().getRouterPrefix();
    pnlsr.getSlh().publishRoutingUpdate(pnlsr.getSm(),lsaPrefix);
    return pnlsr.getLsdb().installAdjLsa(pnlsr, adjLsa);
  }

  bool
  Lsdb::removeAdjLsa(Nlsr& pnlsr, string& key)
  {
    std::list<AdjLsa >::iterator it = std::find_if( adjLsdb.begin(),
                                      adjLsdb.end(),
                                      bind(adjLsaCompareByKey, _1, key));
    if ( it != adjLsdb.end() )
    {
      (*it).removeNptEntries(pnlsr);
      adjLsdb.erase(it);
      return true;
    }
    return false;
  }

  bool
  Lsdb::doesAdjLsaExist(string key)
  {
    std::list< AdjLsa >::iterator it = std::find_if( adjLsdb.begin(),
                                       adjLsdb.end(),
                                       bind(adjLsaCompareByKey, _1, key));
    if( it == adjLsdb.end())
    {
      return false;
    }
    return true;
  }

  std::list<AdjLsa>&
  Lsdb::getAdjLsdb()
  {
    return adjLsdb;
  }

  void
  Lsdb::setLsaRefreshTime(int lrt)
  {
    lsaRefreshTime=lrt;
  }

  void
  Lsdb::setThisRouterPrefix(string trp)
  {
    thisRouterPrefix=trp;
  }

  void
  Lsdb::exprireOrRefreshNameLsa(Nlsr& pnlsr, string lsaKey, int seqNo)
  {
    cout<<"Lsdb::exprireOrRefreshNameLsa Called "<<endl;
    cout<<"LSA Key : "<<lsaKey<<" Seq No: "<<seqNo<<endl;
    std::pair<NameLsa& , bool> chkNameLsa=getNameLsa(lsaKey);
    if( chkNameLsa.second )
    {
      cout<<" LSA Exists with seq no: "<<chkNameLsa.first.getLsSeqNo()<<endl;
      if ( chkNameLsa.first.getLsSeqNo() == seqNo )
      {
        if(chkNameLsa.first.getOrigRouter() == thisRouterPrefix )
        {
          src::logger lg;
          BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"Deleting name lsa";
          chkNameLsa.first.writeLog();
          cout<<"Own Name LSA, so refreshing name LSA"<<endl;
          chkNameLsa.first.setLsSeqNo(chkNameLsa.first.getLsSeqNo()+1);
          pnlsr.getSm().setNameLsaSeq(chkNameLsa.first.getLsSeqNo());
          BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"Adding name lsa";
          chkNameLsa.first.writeLog();
          // publish routing update
          string lsaPrefix=pnlsr.getConfParameter().getChronosyncLsaPrefix()
                           + pnlsr.getConfParameter().getRouterPrefix();
          pnlsr.getSlh().publishRoutingUpdate(pnlsr.getSm(),lsaPrefix);
        }
        else
        {
          cout<<"Other's Name LSA, so removing form LSDB"<<endl;
          removeNameLsa(pnlsr, lsaKey);
        }
      }
    }
  }

  void
  Lsdb::exprireOrRefreshAdjLsa(Nlsr& pnlsr, string lsaKey, int seqNo)
  {
    cout<<"Lsdb::exprireOrRefreshAdjLsa Called "<<endl;
    cout<<"LSA Key : "<<lsaKey<<" Seq No: "<<seqNo<<endl;
    std::pair<AdjLsa& , bool> chkAdjLsa=getAdjLsa(lsaKey);
    if( chkAdjLsa.second )
    {
      cout<<" LSA Exists with seq no: "<<chkAdjLsa.first.getLsSeqNo()<<endl;
      if ( chkAdjLsa.first.getLsSeqNo() == seqNo )
      {
        if(chkAdjLsa.first.getOrigRouter() == thisRouterPrefix )
        {
          cout<<"Own Adj LSA, so refreshing Adj LSA"<<endl;
          chkAdjLsa.first.setLsSeqNo(chkAdjLsa.first.getLsSeqNo()+1);
          pnlsr.getSm().setAdjLsaSeq(chkAdjLsa.first.getLsSeqNo());
          // publish routing update
          string lsaPrefix=pnlsr.getConfParameter().getChronosyncLsaPrefix()
                           + pnlsr.getConfParameter().getRouterPrefix();
          pnlsr.getSlh().publishRoutingUpdate(pnlsr.getSm(),lsaPrefix);
        }
        else
        {
          cout<<"Other's Adj LSA, so removing form LSDB"<<endl;
          removeAdjLsa(pnlsr, lsaKey);
        }
        // schedule Routing table calculaiton
        pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
      }
    }
  }

  void
  Lsdb::exprireOrRefreshCorLsa(Nlsr& pnlsr, string lsaKey, int seqNo)
  {
    cout<<"Lsdb::exprireOrRefreshCorLsa Called "<<endl;
    cout<<"LSA Key : "<<lsaKey<<" Seq No: "<<seqNo<<endl;
    std::pair<CorLsa& , bool> chkCorLsa=getCorLsa(lsaKey);
    if( chkCorLsa.second )
    {
      cout<<" LSA Exists with seq no: "<<chkCorLsa.first.getLsSeqNo()<<endl;
      if ( chkCorLsa.first.getLsSeqNo() == seqNo )
      {
        if(chkCorLsa.first.getOrigRouter() == thisRouterPrefix )
        {
          cout<<"Own Cor LSA, so refreshing Cor LSA"<<endl;
          chkCorLsa.first.setLsSeqNo(chkCorLsa.first.getLsSeqNo()+1);
          pnlsr.getSm().setCorLsaSeq(chkCorLsa.first.getLsSeqNo());
          // publish routing update
          string lsaPrefix=pnlsr.getConfParameter().getChronosyncLsaPrefix()
                           + pnlsr.getConfParameter().getRouterPrefix();
          pnlsr.getSlh().publishRoutingUpdate(pnlsr.getSm(),lsaPrefix);
        }
        else
        {
          cout<<"Other's Cor LSA, so removing form LSDB"<<endl;
          removeCorLsa(pnlsr, lsaKey);
        }
        if (pnlsr.getConfParameter().getIsHyperbolicCalc() >=1 )
        {
          pnlsr.getRoutingTable().scheduleRoutingTableCalculation(pnlsr);
        }
      }
    }
  }


  void
  Lsdb::printAdjLsdb()
  {
    cout<<"---------------Adj LSDB-------------------"<<endl;
    for( std::list<AdjLsa>::iterator it=adjLsdb.begin();
         it!= adjLsdb.end() ; it++)
    {
      cout<< (*it) <<endl;
    }
  }

//-----utility function -----
  bool
  Lsdb::doesLsaExist(string key, int lsType)
  {
    if ( lsType == 1)
    {
      return doesNameLsaExist(key);
    }
    else if ( lsType == 2)
    {
      return doesAdjLsaExist(key);
    }
    else if ( lsType == 3)
    {
      return doesCorLsaExist(key);
    }
    return false;
  }

}//namespace nlsr

