#include<iostream>
#include<cstdlib>



#include "nlsr.hpp"
#include "nlsr_im.hpp"
#include "nlsr_dm.hpp"
#include "nlsr_tokenizer.hpp"

using namespace std;
using namespace ndn;

void 
interestManager::processInterest( nlsr& pnlsr,
                                  const ptr_lib::shared_ptr<const Name> &name, 
                            const ptr_lib::shared_ptr<const Interest> &interest)
{

	cout << "<< I: " << *interest << endl;
  Data data(ndn::Name(interest->getName()).append("testApp").appendVersion());
  data.setFreshnessPeriod(1000); // 10 sec
  data.setContent((const uint8_t*)"HELLO KITTY", sizeof("HELLO KITTY"));
  pnlsr.kChain.sign(data);
  cout << ">> D: " << data << endl;
  pnlsr.nlsrFace.put(data);
}

void 
interestManager::processInterestTimedOut(nlsr& pnlsr,
                 const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest)
{
  	cout << "Timed out interest : " << interest->getName().toUri() << endl;
	string intName=	interest->getName().toUri();
	cout << intName <<endl;
	nlsrTokenizer nt(intName,"/");
	string chkString("info");
	if( nt.doesTokenExist(chkString) ){
		string nbr=nt.getTokenString(0,nt.getTokenPosition(chkString)-1);
		cout<<"Neighbor :"<<nbr<<endl;
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

	pnlsr.nlsrFace.expressInterest(i,
                  ndn::func_lib::bind(&DataManager::processContent, 
                  &pnlsr.dm, boost::ref(pnlsr),_1, _2),
                  ndn::func_lib::bind(&interestManager::processInterestTimedOut,
                                                    this,boost::ref(pnlsr),_1));
}


void 
interestManager::sendScheduledInfoInterest(nlsr& pnlsr, int seconds)
{
	std::list<Adjacent> adjList=pnlsr.adl.getAdjList();
	for(std::list<Adjacent>::iterator it=adjList.begin(); it!=adjList.end();++it)
  {
		string adjName=(*it).getAdjacentName()+"/"+"info"+
                                              pnlsr.confParam.getRouterPrefix();
		expressInterest(	pnlsr,adjName,2,pnlsr.confParam.getInterestResendTime());
	}

	scheduleInfoInterest(pnlsr, pnlsr.confParam.getInfoInterestInterval());

}

void 
interestManager::scheduleInfoInterest(nlsr& pnlsr, int seconds)
{
	pnlsr.scheduler.scheduleEvent(ndn::time::seconds(seconds),
							ndn::bind(&interestManager::sendScheduledInfoInterest, this, 
																									boost::ref(pnlsr),seconds));
}

