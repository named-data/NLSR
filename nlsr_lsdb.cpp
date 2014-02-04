#include<string>
#include "nlsr_lsdb.hpp"
#include "nlsr.hpp"

using namespace std;





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

//Name LSA and LSDB related functions start here


static bool
nameLsaCompareByKey(NameLsa& nlsa1, string& key){
	return nlsa1.getLsaKey()==key;
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
	return installNameLsa(nameLsa);

}

NameLsa& 
Lsdb::getNameLsa(string key)
{
	std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(), 
																		nameLsdb.end(),	
   																	bind(nameLsaCompareByKey, _1, key));

	if( it != nameLsdb.end())
	{
		return (*it);
	}

}



bool 
Lsdb::installNameLsa(NameLsa &nlsa)
{
	bool doesLsaExist_ = doesNameLsaExist(nlsa.getLsaKey());
	if ( !doesLsaExist_ )
	{
		// add name LSA
		addNameLsa(nlsa);
		// update NPT and FIB
		// if its not own LSA
		printNameLsdb();
	}
	else
	{
		// check for newer name LSA
		NameLsa oldNameLsa=getNameLsa(nlsa.getLsaKey());
		// Discard or Update Name lsa, NPT, FIB
		// if its not own LSA
	}
	
	return true;
}

bool 
Lsdb::addNameLsa(NameLsa &nlsa)
{
	std::list<NameLsa >::iterator it = std::find_if( nameLsdb.begin(), 
																		nameLsdb.end(),	
   													  bind(nameLsaCompareByKey, _1, nlsa.getLsaKey()));

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
	return clsa.getLsaKey()==key;
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
	installCorLsa(corLsa);

	return true;
}

CorLsa& 
Lsdb::getCorLsa(string key)
{
	std::list< CorLsa >::iterator it = std::find_if( corLsdb.begin(), 
																		corLsdb.end(),	
   																	bind(corLsaCompareByKey, _1, key));

	if( it != corLsdb.end()){
		return (*it);
	}
}

bool 
Lsdb::installCorLsa(CorLsa &clsa)
{
	bool doesLsaExist_ = doesCorLsaExist(clsa.getLsaKey());
	if ( !doesLsaExist_ )
	{
		// add cor LSA
		addCorLsa(clsa);
		//schedule routing table calculation only if 
		//hyperbolic calculation is scheduled
		printCorLsdb();
	}
	else
	{
		// check for newer cor LSA
		CorLsa oldCorLsa=getCorLsa(clsa.getLsaKey());
		
	}
	
	return true;
}

bool 
Lsdb::addCorLsa(CorLsa& clsa)
{
	std::list<CorLsa >::iterator it = std::find_if( corLsdb.begin(), 
																		corLsdb.end(),	
   														bind(corLsaCompareByKey, _1, clsa.getLsaKey()));

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
	return alsa.getLsaKey()==key;
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
   														bind(adjLsaCompareByKey, _1, alsa.getLsaKey()));

	if( it == adjLsdb.end()){
		adjLsdb.push_back(alsa);
		return true;
	}
	return false;
	
}

AdjLsa& 
Lsdb::getAdjLsa(string key)
{
	std::list<AdjLsa >::iterator it = std::find_if( adjLsdb.begin(), 
																		adjLsdb.end(),	
   																	bind(adjLsaCompareByKey, _1, key));

	if( it != adjLsdb.end()){
		return (*it);
	}
}

bool 
Lsdb::installAdjLsa(nlsr& pnlsr, AdjLsa &alsa)
{
	bool doesLsaExist_ = doesAdjLsaExist(alsa.getLsaKey());
	if ( !doesLsaExist_ )
	{
		// add Adj LSA
		addAdjLsa(alsa);
		// schedule routing table calculation
		pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(15),
							ndn::bind(&RoutingTable::calculate, &pnlsr.getRoutingTable()));
	}
	else
	{
		// check for newer name LSA
		AdjLsa oldAdjLsa=getAdjLsa(alsa.getLsaKey());
		
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

