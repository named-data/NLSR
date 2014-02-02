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
nameLsaCompare(NameLsa& nlsa1, NameLsa& nlsa2){
	return nlsa1.getLsaKey()==nlsa1.getLsaKey();
}

static bool
nameLsaCompareByKey(NameLsa& nlsa1, string& key){
	return nlsa1.getLsaKey()==key;
}


bool
Lsdb::buildAndInstallOwnNameLsa(nlsr& pnlsr)
{
	NameLsa nameLsa(pnlsr.getConfParameter().getRouterPrefix()
					, 1
					, pnlsr.getNameLsaSeq()+1
					, pnlsr.getConfParameter().getRouterDeadInterval()
					, pnlsr.getNpl() );
	pnlsr.setNameLsaSeq(pnlsr.getNameLsaSeq()+1);
	//cout<<nameLsa;
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
   																	bind(nameLsaCompare, _1, nlsa));

	if( it == nameLsdb.end()){
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
	for( std::list<NameLsa>::iterator it=nameLsdb.begin(); 
	                                                 it!= nameLsdb.end() ; it++)
	{
		cout<< (*it) <<endl;
	}
}

// Cor LSA and LSDB related Functions start here

static bool
corLsaCompare(CorLsa& clsa1, CorLsa& clsa2){
	return clsa1.getLsaKey()==clsa1.getLsaKey();
}

static bool
corLsaCompareByKey(CorLsa& clsa, string& key){
	return clsa.getLsaKey()==key;
}

bool 
Lsdb::buildAndInstallOwnCorLsa(nlsr& pnlsr){
	CorLsa corLsa(pnlsr.getConfParameter().getRouterPrefix()
					, 3
					, pnlsr.getCorLsaSeq()+1
					, pnlsr.getConfParameter().getRouterDeadInterval()
					, pnlsr.getConfParameter().getCorR()
					, pnlsr.getConfParameter().getCorTheta() );
	pnlsr.setCorLsaSeq(pnlsr.getCorLsaSeq()+1);
	//cout<<corLsa;
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
   																	bind(corLsaCompare, _1, clsa));

	if( it == corLsdb.end()){
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
	for( std::list<CorLsa>::iterator it=corLsdb.begin(); 
	                                                 it!= corLsdb.end() ; it++)
	{
		cout<< (*it) <<endl;
	}
}


// Adj LSA and LSDB related function starts here

static bool
adjLsaCompare(AdjLsa& alsa1, AdjLsa& alsa2){
	return alsa1.getLsaKey()==alsa1.getLsaKey();
}

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
   																	bind(adjLsaCompare, _1, alsa));

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
Lsdb::installAdjLsa(AdjLsa &alsa)
{
	bool doesLsaExist_ = doesAdjLsaExist(alsa.getLsaKey());
	if ( !doesLsaExist_ )
	{
		// add Adj LSA
		addAdjLsa(alsa);
		// schedule routing table calculation
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
					, pnlsr.getAdjLsaSeq()+1
					, pnlsr.getConfParameter().getRouterDeadInterval()
					, pnlsr.getAdl().getNumOfActiveNeighbor()
					, pnlsr.getAdl() );
	pnlsr.setAdjLsaSeq(pnlsr.getAdjLsaSeq()+1);
	return installAdjLsa(adjLsa);
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
	for( std::list<AdjLsa>::iterator it=adjLsdb.begin(); 
	                                                 it!= adjLsdb.end() ; it++)
	{
		cout<< (*it) <<endl;
	}
}

