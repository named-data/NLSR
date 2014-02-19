#ifndef NLSR_DM_HPP
#define NLSR_DM_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>


using namespace ndn;
using namespace std;

class Nlsr;

class DataManager
{
public:
  void processContent(Nlsr& pnlsr, const ndn::Interest &interest,
								               const ndn::Data& data);
	void processContentInfo(Nlsr& pnlsr, string& dataName,
                                                           string& dataContent);
private:
  
};


#endif
