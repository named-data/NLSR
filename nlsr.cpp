#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include <cstdlib>

#include "nlsr.hpp"
#include "conf_processor.hpp"
#include "conf_param.hpp"
#include "nlsr_tokenizer.hpp"
#include "adl.hpp"
#include "adjacent.hpp"


using namespace ndn;
using namespace std;

void 
nlsr::processInterest(const ptr_lib::shared_ptr<const Name> &name, 
							const ptr_lib::shared_ptr<const Interest> &interest)
{

	cout << "<< I: " << *interest << endl;
    
    Data data(ndn::Name(interest->getName()).append("testApp").appendVersion());
    data.setFreshnessPeriod(1000); // 10 sec

    data.setContent((const uint8_t*)"HELLO KITTY", sizeof("HELLO KITTY"));

    kChain.sign(data);

    cout << ">> D: " << data << endl;
    nlsrFace.put(data);
}

void
nlsr::nlsrRegistrationFailed(const ptr_lib::shared_ptr<const Name>&)
{
	cerr << "ERROR: Failed to register prefix in local hub's daemon" << endl;
    nlsrFace.shutdown();
}

void
nlsr::processContent(const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest,
								 const ndn::ptr_lib::shared_ptr<ndn::Data> &data)
{

	cout << "I: " << interest->toUri() << endl;
  	cout << "D: " << data->getName().toUri() << endl;
	cout << "Data Content: " << data->getContent() << endl;

}

void
nlsr::processInterestTimedOut(
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
nlsr::setInterestFilterNlsr(const string& name){
	nlsrFace.setInterestFilter(name,
                        	func_lib::bind(&nlsr::processInterest, this, _1, _2),
                        func_lib::bind(&nlsr::nlsrRegistrationFailed, this, _1));
}

void
nlsr::expressInterest(const string& interestNamePrefix, int scope, int seconds)
{
	Interest i((ndn::Name(interestNamePrefix)));
    //i.setScope(scope);
    i.setInterestLifetime(seconds*1000);
    i.setMustBeFresh(true);

	nlsrFace.expressInterest(i,
                          ndn::func_lib::bind(&nlsr::processContent, this, _1, _2),
                          ndn::func_lib::bind(&nlsr::processInterestTimedOut, this, _1));

}


void
nlsr::scheduleInfoInterest(int seconds){
	scheduler.scheduleEvent(ndn::time::seconds(seconds),
							ndn::bind(&nlsr::sendScheduledInfoInterest, this,seconds));
}

void
nlsr::sendScheduledInfoInterest(int seconds){

	std::list<Adjacent> adjList=adl.getAdjList();
	for(std::list<Adjacent>::iterator it	=adjList.begin(); it!= adjList.end(); ++it){
		string adjName=(*it).getAdjacentName()+"/"+"info"+confParam.getRouterPrefix();
		expressInterest(	adjName,2,confParam.getInterestResendTime());
	}

	scheduleInfoInterest(confParam.getInfoInterestInterval());
}

void
nlsr::startEventLoop(){
	nlsrFace.processEvents();
}

int 
nlsr::usage(const string& progname){

        cout << "Usage: " << progname << " [OPTIONS...]"<<endl;
        cout << "   NDN routing...." << endl;
        cout << "       -d, --daemon        Run in daemon mode" << endl;
        cout << "       -f, --config_file   Specify configuration file name" <<endl;
        cout << "       -p, --api_port      port where api client will connect" <<endl;
        cout << "       -h, --help          Display this help message" << endl;

        exit(EXIT_FAILURE);
}

int 
main(){

	nlsr nlsr;
	nlsr.setConfFileName("nlsr.conf");
	ConfFileProcessor cfp(nlsr.getConfFileName());
	cfp.processConfFile(nlsr);
	nlsr.confParam.buildRouterPrefix();
/* debugging purpose start */
	cout <<	nlsr.confParam ;
	nlsr.adl.printAdl();
	nlsr.npl.printNpl();
/* debugging purpose end */
	nlsr.setInterestFilterNlsr("/ndn/memphis.edu/cs/pollux/");
	
	nlsr.scheduleInfoInterest(1);

	
	

	try{
		nlsr.startEventLoop();
	}catch(std::exception &e) {
    		std::cerr << "ERROR: " << e.what() << std::endl;
	}

	return 0;
}
