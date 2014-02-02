#include<iostream>
#include<cstdlib>



#include "nlsr.hpp"
#include "nlsr_im.hpp"
#include "nlsr_dm.hpp"
#include "nlsr_tokenizer.hpp"
#include "nlsr_lsdb.hpp"

using namespace std;
using namespace ndn;

void 
interestManager::processInterest( nlsr& pnlsr,
                                  const ptr_lib::shared_ptr<const Name> &name, 
                            const ptr_lib::shared_ptr<const Interest> &interest)
{

	cout << "<< I: " << *interest << endl;
	string intName=interest->getName().toUri();
	cout << "Interest Received for Name: "<< intName <<endl;
	nlsrTokenizer nt(intName,"/");
	string chkString("info");
	if( nt.doesTokenExist(chkString) ){
		string nbr=nt.getTokenString(nt.getTokenPosition(chkString)+1);
		cout <<"Neighbor: " << nbr <<endl;
		processInterestInfo(pnlsr,nbr,interest);
	}
	
  //Data data(ndn::Name(interest->getName()).append("testApp").appendVersion());
  //data.setFreshnessPeriod(1000); // 10 sec
  //data.setContent((const uint8_t*)"HELLO KITTY", sizeof("HELLO KITTY"));
  //pnlsr.getKeyChain().sign(data);
  //cout << ">> D: " << data << endl;
  //pnlsr.getNlsrFace().put(data);
}

void 
interestManager::processInterestInfo(nlsr& pnlsr, string& neighbor,
							            const ptr_lib::shared_ptr<const Interest> &interest)
{
	if ( pnlsr.getAdl().isNeighbor(neighbor) )
	{
		Data data(ndn::Name(interest->getName()).appendVersion());
  		data.setFreshnessPeriod(1000); // 10 sec
  		data.setContent((const uint8_t*)"info", sizeof("info"));
  		pnlsr.getKeyChain().sign(data);
  		cout << ">> D: " << data << endl;
  		pnlsr.getNlsrFace().put(data);

  		int status=pnlsr.getAdl().getStatusOfNeighbor(neighbor);
  		if ( status == 0 )
  		{
			string intName=neighbor +"/"+"info"+
                                     pnlsr.getConfParameter().getRouterPrefix();
    		expressInterest(	pnlsr,intName,2,
                              pnlsr.getConfParameter().getInterestResendTime());
  		}
	}
}

void 
interestManager::processInterestTimedOut(nlsr& pnlsr,
                 const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest)
{
  	cout << "Timed out interest : " << interest->getName().toUri() << endl;
	string intName=	interest->getName().toUri();
	nlsrTokenizer nt(intName,"/");
	string chkString("info");
	if( nt.doesTokenExist(chkString) ){
		string nbr="/" + nt.getFirstToken()
							+nt.getTokenString(0,nt.getTokenPosition(chkString)-1);
		processInterestTimedOutInfo( pnlsr , nbr , interest);
	}

}

void 
interestManager::processInterestTimedOutInfo(nlsr& pnlsr, string& neighbor,
                 const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest)
{
	pnlsr.getAdl().incrementTimedOutInterestCount(neighbor);
	int status=pnlsr.getAdl().getStatusOfNeighbor(neighbor);
	int infoIntTimedOutCount=pnlsr.getAdl().getTimedOutInterestCount(neighbor);
	cout<<"Neighbor: "<< neighbor << endl;
	cout<<"Status: "<< status << endl;
	cout<<"Info Interest Timed out: "<< infoIntTimedOutCount <<endl;

	if((infoIntTimedOutCount < pnlsr.getConfParameter().getInterestRetryNumber()))
	{
		string intName=neighbor +"/"+"info"+
                                     pnlsr.getConfParameter().getRouterPrefix();
    expressInterest(	pnlsr,intName,2,
                              pnlsr.getConfParameter().getInterestResendTime());
	}
	else if ( (status == 1) && 
	  (infoIntTimedOutCount == pnlsr.getConfParameter().getInterestRetryNumber()))
	{
		pnlsr.getAdl().setStatusOfNeighbor(neighbor,0);
		pnlsr.incrementAdjBuildCount();
		if ( pnlsr.getIsBuildAdjLsaSheduled() == 0 )
		{
			pnlsr.setIsBuildAdjLsaSheduled(1);
			// event here
			pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(5),
							ndn::bind(&Lsdb::scheduledAdjLsaBuild,pnlsr.getLsdb(), 
																									boost::ref(pnlsr)));
		}
	}
	
}

void 
interestManager::expressInterest(nlsr& pnlsr,const string& interestNamePrefix, 
                                  											int scope, int seconds)
{
	Interest i((ndn::Name(interestNamePrefix)));
  //i.setScope(scope);
  i.setInterestLifetime(seconds*1000);
	i.setMustBeFresh(true);

	pnlsr.getNlsrFace().expressInterest(i,
                  ndn::func_lib::bind(&DataManager::processContent, 
                  &pnlsr.getDm(), boost::ref(pnlsr),_1, _2),
                  ndn::func_lib::bind(&interestManager::processInterestTimedOut,
                                                    this,boost::ref(pnlsr),_1));
}


void 
interestManager::sendScheduledInfoInterest(nlsr& pnlsr, int seconds)
{
	std::list<Adjacent> adjList=pnlsr.getAdl().getAdjList();
	for(std::list<Adjacent>::iterator it=adjList.begin(); it!=adjList.end();++it)
  {
		string adjName=(*it).getAdjacentName()+"/"+"info"+
                                              pnlsr.getConfParameter().getRouterPrefix();
		expressInterest(	pnlsr,adjName,2,pnlsr.getConfParameter().getInterestResendTime());
	}

	scheduleInfoInterest(pnlsr, pnlsr.getConfParameter().getInfoInterestInterval());

}

void 
interestManager::scheduleInfoInterest(nlsr& pnlsr, int seconds)
{
	pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(seconds),
							ndn::bind(&interestManager::sendScheduledInfoInterest, this, 
																									boost::ref(pnlsr),seconds));
}

