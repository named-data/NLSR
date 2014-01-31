#ifndef NLSR_DM_HPP
#define NLSR_DM_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>


using namespace ndn;
using namespace std;

class nlsr;

class DataManager
{
public:
  void processContent(const nlsr& pnlsr, 
                  const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest,
								               const ndn::ptr_lib::shared_ptr<ndn::Data> &data);
private:
  
};


#endif
