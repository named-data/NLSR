#include <string>
#include <boost/cstdint.hpp>
#include <ndn-cxx/face.hpp>

#ifndef NLSR_ADJACENT_HPP
#define NLSR_ADJACENT_HPP

namespace nlsr {

enum {
  ADJACENT_STATUS_INACTIVE = 0,
  ADJACENT_STATUS_ACTIVE = 1
};

class Adjacent
{

public:
  Adjacent();

  Adjacent(const ndn::Name& an);

  Adjacent(const ndn::Name& an, const std::string& cfu,  double lc,
          uint32_t s, uint32_t iton);

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

  const std::string&
  getConnectingFaceUri() const
  {
    return m_connectingFaceUri;
  }

  void
  setConnectingFaceUri(const std::string& cfu)
  {
    m_connectingFaceUri = cfu;
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

public:
  static const float DEFAULT_LINK_COST;

private:
  ndn::Name m_name;
  std::string m_connectingFaceUri;
  double m_linkCost;
  uint32_t m_status;
  uint32_t m_interestTimedOutNo;
};

std::ostream&
operator<<(std::ostream& os, const Adjacent& adj);

} // namespace nlsr

#endif //NLSR_ADJACENT_HPP
