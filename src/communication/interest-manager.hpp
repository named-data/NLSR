#ifndef NLSR_IM_HPP
#define NLSR_IM_HPP

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/scheduler.hpp>

namespace nlsr {

class Nlsr;

class InterestManager
{
public:
  InterestManager(Nlsr& nlsr)
    : m_nlsr(nlsr)
  {
  }
  void
  processInterest(const ndn::Name& name, const ndn::Interest& interest);

  void
  processInterestInfo(const std::string& neighbor, const ndn::Interest& interest);

  void
  processInterestLsa(const ndn::Interest& interest);

  void
  processInterestForNameLsa(const ndn::Interest& interest,
                            const std::string& lsaKey, uint32_t interestedlsSeqNo);

  void
  processInterestForAdjLsa(const ndn::Interest& interest,
                           const std::string& lsaKey, uint32_t interestedlsSeqNo);

  void
  processInterestForCorLsa(const ndn::Interest& interest,
                           const std::string& lsaKey, uint32_t interestedlsSeqNo);

  void
  processInterestKeys(const ndn::Interest& interest);

  void
  processInterestTimedOut(const ndn::Interest& interest);

  void
  processInterestTimedOutInfo(const std::string& neighbor,
                              const ndn::Interest& interest);

  void
  processInterestTimedOutLsa(const ndn::Interest& interest);

  void
  expressInterest(const std::string& interestNamePrefix, int scope, int seconds);

  void
  sendScheduledInfoInterest(int seconds);

  void
  scheduleInfoInterest(int seconds);

private:
  Nlsr& m_nlsr;

  ndn::KeyChain m_keyChain;
};

}//namespace nlsr

#endif //NLSR_IM_HPP
