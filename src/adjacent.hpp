#include <string>
#include <boost/cstdint.hpp>
#include <ndn-cxx/face.hpp>

#ifndef NLSR_ADJACENT_HPP
#define NLSR_ADJACENT_HPP

namespace nlsr {
class Adjacent
{

public:
  Adjacent()
    : m_name()
    , m_connectingFace(0)
    , m_linkCost(10.0)
    , m_status(0)
    , m_interestTimedOutNo(0)
  {
  }

  Adjacent(const ndn::Name& an)
    : m_connectingFace(0)
    , m_linkCost(0.0)
    , m_status(0)
    , m_interestTimedOutNo(0)
  {
    m_name = an;
  }

  Adjacent(const ndn::Name& an, uint32_t cf, double lc, uint32_t s,
           uint32_t iton);

  const ndn::Name&
  getName() const
  {
    return m_name;
  }

  void
  setName(const ndn::Name& an)
  {
    m_name = an;
  }

  uint32_t
  getConnectingFace() const
  {
    return m_connectingFace;
  }

  void
  setConnectingFace(uint32_t cf)
  {
    m_connectingFace = cf;
  }

  double
  getLinkCost() const
  {
    return m_linkCost;
  }

  void
  setLinkCost(double lc)
  {
    m_linkCost = lc;
  }

  uint32_t
  getStatus() const
  {
    return m_status;
  }

  void
  setStatus(uint32_t s)
  {
    m_status = s;
  }

  uint32_t
  getInterestTimedOutNo() const
  {
    return m_interestTimedOutNo;
  }

  void
  setInterestTimedOutNo(uint32_t iton)
  {
    m_interestTimedOutNo = iton;
  }

  bool
  operator==(const Adjacent& adjacent) const;

  bool
  compare(const ndn::Name& adjacencyName);

private:
  ndn::Name m_name;
  uint32_t m_connectingFace;
  double m_linkCost;
  uint32_t m_status;
  uint32_t m_interestTimedOutNo;
};

std::ostream&
operator<<(std::ostream& os, const Adjacent& adj);

} // namespace nlsr

#endif //NLSR_ADJACENT_HPP
