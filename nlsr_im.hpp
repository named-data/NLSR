#ifndef NLSR_IM_HPP
#define NLSR_IM_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>


using namespace ndn;
using namespace std;

class nlsr;

class interestManager{
public:	
	interestManager()
	{
	}
  void processInterest(nlsr& pnlsr, const ptr_lib::shared_ptr<const Name> &name, 
							            const ptr_lib::shared_ptr<const Interest> &interest);
	void processInterestInfo(nlsr& pnlsr, string& neighbor, 
							            const ptr_lib::shared_ptr<const Interest> &interest);
  void processInterestTimedOut(nlsr& pnlsr,
                 const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest);
  void processInterestTimedOutInfo(nlsr& pnlsr, string& neighbor,
                 const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest);
  void expressInterest(nlsr& pnlsr,const string& interestNamePrefix, int scope, 
                                                                   int seconds);
  void sendScheduledInfoInterest(nlsr& pnlsr, int seconds);
	void scheduleInfoInterest(nlsr& pnlsr, int seconds);

private:	


};


#endif
