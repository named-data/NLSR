#ifndef NLSR_DATA_MANAGER_HPP
#define NLSR_DATA_MANAGER_HPP

#include <boost/cstdint.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/scheduler.hpp>

#include "interest-manager.hpp"

namespace nlsr {
class Nlsr;

class DataManager
{
public:
  DataManager(Nlsr& nlsr)
    : m_nlsr(nlsr)
  {}
  void
  processContent(const ndn::Interest& interest,
                 const ndn::Data& data, InterestManager& im);
private:
  void
  processContentInfo(const std::string& dataName,
                     std::string& dataContent);

  void
  processContentLsa(const std::string& dataName, std::string& dataContent);

  void
  processContentNameLsa(const std::string& lsaKey,
                        uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentAdjLsa(const std::string& lsaKey,
                       uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentCorLsa(const std::string& lsaKey,
                       uint32_t lsSeqNo, std::string& dataContent);

  void
  processContentKeys(const ndn::Data& data);

private:
  Nlsr& m_nlsr;


};

}//namespace nlsr

#endif //NLSR_DATA_MANAGER_HPP
