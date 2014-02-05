#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "nlsr.hpp"
#include "nlsr_test.hpp"

using namespace std;
using namespace ndn;

void 
nlsrTest::schedlueAddingLsas(nlsr& pnlsr)
{
	// scheduling adding two name lsas, two Cor Lsas and three Adj LSAs

	//Scheduling Adding LSAs for router altair
	string router("/ndn/memphis.edu/cs/altair");
	string name1("/ndn/memphis.edu/cs/altair/name1");
	string name2("/ndn/memphis.edu/cs/altair/name2");
	string name3("/ndn/memphis.edu/cs/altair/name3");
	Adjacent adj1("/ndn/memphis.edu/cs/pollux",7,17,1,0);
	Adjacent adj2("/ndn/memphis.edu/cs/maia",15,27,1,0);
	
	pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(30),
							ndn::bind(&nlsrTest::secheduledAddNameLsa,pnlsr.getNlsrTesting(), 
																									boost::ref(pnlsr)
							,router,name1,name2,name3));
	pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(37),
							ndn::bind(&nlsrTest::secheduledAddCorLsa,pnlsr.getNlsrTesting(), 
																									boost::ref(pnlsr)
							,router,123.098,1.875));
	pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(47),
							ndn::bind(&nlsrTest::scheduledAddAdjacentLsa,pnlsr.getNlsrTesting(), 
																									boost::ref(pnlsr)
							,router,adj1,adj2));

	//Scheduling Adding LSAs for router Maia
	string routerMaia("/ndn/memphis.edu/cs/maia");
	string maiaName1("/ndn/memphis.edu/maia/name1");
	string maiaName2("/ndn/memphis.edu/maia/name2");
	string maiaName3("/ndn/memphis.edu/maia/name3");
	Adjacent maiaAdj1("/ndn/memphis.edu/cs/pollux",8,25,1,0);
	Adjacent maiaAdj2("/ndn/memphis.edu/cs/altair",11,15,1,0);

	pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(55),
							ndn::bind(&nlsrTest::secheduledAddNameLsa,pnlsr.getNlsrTesting(), 
																									boost::ref(pnlsr)
							,routerMaia,maiaName1,maiaName2,maiaName3));
	pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(65),
							ndn::bind(&nlsrTest::secheduledAddCorLsa,pnlsr.getNlsrTesting(), 
																									boost::ref(pnlsr)
							,routerMaia,12.098,0.875));
	pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(75),
							ndn::bind(&nlsrTest::scheduledAddAdjacentLsa,pnlsr.getNlsrTesting(), 
																									boost::ref(pnlsr)
							,routerMaia,maiaAdj1,maiaAdj2));

	//sheduling Adding LSAs for Router itself
	string routerPollux("/ndn/memphis.edu/cs/pollux");
	Adjacent polluxAdj1("/ndn/memphis.edu/cs/maia",9,13,1,0);
	Adjacent polluxAdj2("/ndn/memphis.edu/cs/altair",12,23,1,0);

	pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(90),
							ndn::bind(&nlsrTest::scheduledAddAdjacentLsa,pnlsr.getNlsrTesting(), 
																									boost::ref(pnlsr)
							,routerPollux,polluxAdj1,polluxAdj2));
	
	
}



void 
nlsrTest::secheduledAddNameLsa(nlsr& pnlsr, string router,
																		 string name1, string name2, string name3)
{
	Npl npl;
	npl.insertIntoNpl(name1);
	npl.insertIntoNpl(name2);
	npl.insertIntoNpl(name3);
	NameLsa nameLsa(router,1,1,3600,npl);
	pnlsr.getLsdb().installNameLsa(nameLsa);
	
}
																		 
void 
nlsrTest::secheduledAddCorLsa(nlsr& pnlsr,string router, double r, double angle)
{
	CorLsa corLsa(router,3,1,3600,r,angle);
	pnlsr.getLsdb().installCorLsa(corLsa);
}

void 
nlsrTest::scheduledAddAdjacentLsa(nlsr& pnlsr, string router, 
	                                                Adjacent adj1, Adjacent adj2)
{
	Adl adl;
	adl.insert(adj1);
	adl.insert(adj2);
	AdjLsa adjLsa(router,2,1,3600,2,adl);
	pnlsr.getLsdb().installAdjLsa(pnlsr, adjLsa);
	
}
	                                                
