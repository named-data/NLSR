#ifndef NLSR_HELLO_PROTOCOL_HPP
#define NLSR_HELLO_PROTOCOL_HPP

#include <boost/cstdint.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/util/scheduler.hpp>

namespace nlsr {

class Nlsr;

class HelloProtocol
{

public:
  HelloProtocol(Nlsr& nlsr)
    : m_nlsr(nlsr)
  {
  }

public:
  void
  scheduleInterest(uint32_t seconds);

  void
  expressInterest(const ndn::Name& interestNamePrefix, uint32_t seconds);

  void
  sendScheduledInterest(uint32_t seconds);

  void
  processInterest(const ndn::Name& name, const ndn::Interest& interest);

private:
  void
  processInterestTimedOut(const ndn::Interest& interest);

  void
  processContent(const ndn::Interest& interest, const ndn::Data& data);

private:
  Nlsr& m_nlsr;
  ndn::KeyChain m_keyChain;
  static const std::string INFO_COMPONENT;
};

} //namespace nlsr

#endif // NLSR_HELLO_PROTOCOL_HPP
