#ifndef NLSR_DM_HPP
#define NLSR_DM_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "interest-manager.hpp"

namespace nlsr {
class Nlsr;

class DataManager
{
public:
  void
  processContent(Nlsr& pnlsr, const ndn::Interest& interest,
                 const ndn::Data& data, InterestManager& im);
private:
  void
  processContentInfo(Nlsr& pnlsr, std::string& dataName,
                     std::string& dataContent);

  void
  processContentLsa(Nlsr& pnlsr, std::string& dataName,
                    std::string& dataContent);

  void
  processContentNameLsa(Nlsr& pnlsr, std::string lsaKey,
                        uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentAdjLsa(Nlsr& pnlsr, std::string lsaKey,
                       uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentCorLsa(Nlsr& pnlsr, std::string lsaKey,
                       uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentKeys(Nlsr& pnlsr, const ndn::Data& data);


};

}//namespace nlsr

#endif //NLSR_DM_HPP
