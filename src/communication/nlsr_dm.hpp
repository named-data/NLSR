#ifndef NLSR_DM_HPP
#define NLSR_DM_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "nlsr_im.hpp"

namespace nlsr
{

  using namespace ndn;
  using namespace std;

  class Nlsr;

  class DataManager
  {
  public:
    void processContent(Nlsr& pnlsr, const ndn::Interest& interest,
                        const ndn::Data& data, InterestManager& im);
  private:
    void processContentInfo(Nlsr& pnlsr, string& dataName,
                            string& dataContent);
    void processContentLsa(Nlsr& pnlsr, string& dataName,
                           string& dataContent);
    void processContentNameLsa(Nlsr& pnlsr, string lsaKey,
                               uint32_t lsSeqNo, string& dataContent);
    void processContentAdjLsa(Nlsr& pnlsr, string lsaKey,
                              uint32_t lsSeqNo, string& dataContent);
    void processContentCorLsa(Nlsr& pnlsr, string lsaKey,
                              uint32_t lsSeqNo, string& dataContent);
    void processContentKeys(Nlsr& pnlsr, const ndn::Data& data);


  };

}//namespace nlsr
#endif
