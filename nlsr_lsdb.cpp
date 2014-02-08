#include<string>
#include<utility>
#include "nlsr_lsdb.hpp"
#include "nlsr.hpp"

using namespace std;

static bool
nameLsaCompareByKey(NameLsa& nlsa1, string& key){
	return nlsa1.getNameLsaKey()==key;
}


bool
Lsdb::buildAndInstallOwnNameLsa(nlsr& pnlsr)
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
Lsdb::installNameLsa(nlsr& pnlsr, NameLsa &nlsa)
{
	std::pair<NameLsa& , bool> chkNameLsa=getNameLsa(nlsa.getNameLsaKey());
	if ( !chkNameLsa.second )
	{
		addNameLsa(nlsa);
		printNameLsdb();
		if ( nlsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
		{
			pnlsr.getNpt().addNpte(nlsa.getOrigRouter(),nlsa.getOrigRouter(),pnlsr);
			std::list<string> nameList=nlsa.getNpl().getNameList();
			for(std::list<string>::iterator it=nameList.begin(); it!=nameList.end();it++)
			{
				pnlsr.getNpt().addNpte((*it),nlsa.getOrigRouter(),pnlsr);
			}
		} 
	}
	else
	{
		if ( chkNameLsa.first.getLsSeqNo() < nlsa.getLsSeqNo() )
		{
			chkNameLsa.first.setLsSeqNo(nlsa.getLsSeqNo());
			chkNameLsa.first.setLifeTime(nlsa.getLifeTime());

			chkNameLsa.first.getNpl().sortNpl();
			nlsa.getNpl().sortNpl();

			std::list<string> nameToAdd;
			std::set_difference(nlsa.getNpl().getNameList().begin(),
			                    nlsa.getNpl().getNameList().end(),
			                    chkNameLsa.first.getNpl().getNameList().begin(),
			                    chkNameLsa.first.getNpl().getNameList().end(),
                          std::inserter(nameToAdd, nameToAdd.begin()));
      for(std::list<string>::iterator it=nameToAdd.begin(); it!=nameToAdd.end();
                                                                           ++it)
      {
      		chkNameLsa.first.addNameToLsa((*it));
      		if ( nlsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
      		{
      			pnlsr.getNpt().addNpte((*it),nlsa.getOrigRouter(),pnlsr);
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
      		chkNameLsa.first.removeNameFromLsa((*it));
      		if ( nlsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
      		{
      			pnlsr.getNpt().removeNpte((*it),nlsa.getOrigRouter(),pnlsr);
      		}
      }  
			
		}
	}
	
	return true;
}

bool 
Lsdb::addNameLsa(NameLsa &nlsa)
{
	std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(), 
																		nameLsdb.end(),	
   													  bind(nameLsaCompareByKey, _1, nlsa.getNameLsaKey()));

	if( it == nameLsdb.end())
	{
		nameLsdb.push_back(nlsa);
		return true;
	}
	return false;
}

bool 
Lsdb::removeNameLsa(string& key)
{
	std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(), 
																		nameLsdb.end(),	
   																	bind(nameLsaCompareByKey, _1, key));
  if ( it != nameLsdb.end() )
  {
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

	if( it == nameLsdb.end()){
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
/*
static bool
corLsaCompare(CorLsa& clsa1, CorLsa& clsa2){
	return clsa1.getLsaKey()==clsa1.getLsaKey();
}
*/
static bool
corLsaCompareByKey(CorLsa& clsa, string& key){
	return clsa.getCorLsaKey()==key;
}

bool 
Lsdb::buildAndInstallOwnCorLsa(nlsr& pnlsr){
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

	if( it != corLsdb.end()){
		return std::make_pair(boost::ref((*it)), true);
	}

	CorLsa clsa;
	return std::make_pair(boost::ref(clsa),false);
}

bool 
Lsdb::installCorLsa(nlsr& pnlsr, CorLsa &clsa)
{
	std::pair<CorLsa& , bool> chkCorLsa=getCorLsa(clsa.getCorLsaKey());
	if ( !chkCorLsa.second )
	{
		// add cor LSA
		addCorLsa(clsa);
		printCorLsdb(); //debugging purpose
		if ( clsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
		{
			pnlsr.getNpt().addNpte(clsa.getOrigRouter(),clsa.getOrigRouter(),pnlsr);
		}
		//schedule routing table calculation only if 
		//hyperbolic calculation is scheduled
		if (pnlsr.getConfParameter().getIsHyperbolicCalc() >=1 )
		{
			if ( pnlsr.getIsRouteCalculationScheduled() != 1 )
			{
				pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(15),
								ndn::bind(&RoutingTable::calculate, 
								&pnlsr.getRoutingTable(),boost::ref(pnlsr)));
				pnlsr.setIsRouteCalculationScheduled(1);
			}	
		}
		
	}
	else
	{
		// check for newer cor LSA
		//CorLsa oldCorLsa=getCorLsa(clsa.getCorLsaKey());
		
	}
	
	return true;
}

bool 
Lsdb::addCorLsa(CorLsa& clsa)
{
	std::list<CorLsa >::iterator it = std::find_if( corLsdb.begin(), 
																		corLsdb.end(),	
   														bind(corLsaCompareByKey, _1, clsa.getCorLsaKey()));

	if( it == corLsdb.end())
	{
		corLsdb.push_back(clsa);
		return true;
	}
	return false;
}

bool 
Lsdb::removeCorLsa(string& key)
{
	std::list<CorLsa >::iterator it = std::find_if( corLsdb.begin(), 
																		corLsdb.end(),	
   																	bind(corLsaCompareByKey, _1, key));
  if ( it != corLsdb.end() )
  {
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

	if( it == corLsdb.end()){
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
/*
static bool
adjLsaCompare(AdjLsa& alsa1, AdjLsa& alsa2){
	return alsa1.getLsaKey()==alsa1.getLsaKey();
}
*/
static bool
adjLsaCompareByKey(AdjLsa& alsa, string& key){
	return alsa.getAdjLsaKey()==key;
}


void 
Lsdb::scheduledAdjLsaBuild(nlsr& pnlsr)
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
				//remove if there is any adj lsa in LSDB
				string key=pnlsr.getConfParameter().getRouterPrefix()+"/2";
				removeAdjLsa(key);
				// Remove alll fib entries as per NPT
			}
			pnlsr.setAdjBuildCount(pnlsr.getAdjBuildCount()-adjBuildCount);
		}		
	}
	else
	{
		pnlsr.setIsBuildAdjLsaSheduled(1);
		int schedulingTime=pnlsr.getConfParameter().getInterestRetryNumber()*
		                   pnlsr.getConfParameter().getInterestRetryNumber();
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
   														bind(adjLsaCompareByKey, _1, alsa.getAdjLsaKey()));

	if( it == adjLsdb.end()){
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

	if( it != adjLsdb.end()){
		return std::make_pair(boost::ref((*it)),true);
	}

	AdjLsa alsa;
	return std::make_pair(boost::ref(alsa),false);
}

bool 
Lsdb::installAdjLsa(nlsr& pnlsr, AdjLsa &alsa)
{
	//bool doesLsaExist_ = doesAdjLsaExist(alsa.getAdjLsaKey());
	//if ( !doesLsaExist_ )
	std::pair<AdjLsa& , bool> chkAdjLsa=getAdjLsa(alsa.getAdjLsaKey());
	if ( !chkAdjLsa.second )
	{
		// add Adj LSA
		addAdjLsa(alsa);
		// adding a NPT entry for router itself
		if ( alsa.getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
		{
			pnlsr.getNpt().addNpte(alsa.getOrigRouter(),alsa.getOrigRouter(),pnlsr);
		}
		// schedule routing table calculation
		if ( pnlsr.getIsRouteCalculationScheduled() != 1 )
		{
			pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(15),
								ndn::bind(&RoutingTable::calculate, 
								&pnlsr.getRoutingTable(),boost::ref(pnlsr)));
			pnlsr.setIsRouteCalculationScheduled(1);
		}
	}
	else
	{
		// check for newer name LSA
		//AdjLsa oldAdjLsa=getAdjLsa(alsa.getAdjLsaKey());
		
	}

	printAdjLsdb();
	
	return true;
}

bool 
Lsdb::buildAndInstallOwnAdjLsa(nlsr& pnlsr)
{
	AdjLsa adjLsa(pnlsr.getConfParameter().getRouterPrefix()
					, 2
					, pnlsr.getSm().getAdjLsaSeq()+1
					, pnlsr.getConfParameter().getRouterDeadInterval()
					, pnlsr.getAdl().getNumOfActiveNeighbor()
					, pnlsr.getAdl() );
	pnlsr.getSm().setAdjLsaSeq(pnlsr.getSm().getAdjLsaSeq()+1);
	return installAdjLsa(pnlsr, adjLsa);
}

bool 
Lsdb::removeAdjLsa(string& key)
{
	std::list<AdjLsa >::iterator it = std::find_if( adjLsdb.begin(), 
																		adjLsdb.end(),	
   																	bind(adjLsaCompareByKey, _1, key));
  if ( it != adjLsdb.end() )
  {
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

	if( it == adjLsdb.end()){
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


