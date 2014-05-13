#ifndef CONF_PARAMETER_HPP
#define CONF_PARAMETER_HPP

#include <iostream>
#include <boost/cstdint.hpp>
#include <ndn-cxx/common.hpp>
#include <ndn-cxx/face.hpp>

namespace nlsr {

enum {
  LSA_REFRESH_TIME_MIN = 240,
  LSA_REFRESH_TIME_DEFAULT = 1800,
  LSA_REFRESH_TIME_MAX = 7200
};

enum {
  HELLO_RETRIES_MIN = 1,
  HELLO_RETRIES_DEFAULT = 3,
  HELLO_RETRIES_MAX = 15
};

enum {
  HELLO_TIMEOUT_MIN = 1,
  HELLO_TIMEOUT_DEFAULT = 3,
  HELLO_TIMEOUT_MAX = 15
};

enum {
  HELLO_INTERVAL_MIN = 30,
  HELLO_INTERVAL_DEFAULT = 60,
  HELLO_INTERVAL_MAX =90
};

enum {
  MAX_FACES_PER_PREFIX_MIN = 0,
  MAX_FACES_PER_PREFIX_MAX = 60
};

enum {
  HYPERBOLIC_STATE_OFF = 0,
  HYPERBOLIC_STATE_ON = 1,
  HYPERBOLIC_STATE_DRY_RUN = 2
};

class ConfParameter
{

public:
  ConfParameter()
    : m_lsaRefreshTime(LSA_REFRESH_TIME_DEFAULT)
    , m_routerDeadInterval(2*LSA_REFRESH_TIME_DEFAULT)
    , m_logLevel("INFO")
    , m_interestRetryNumber(HELLO_RETRIES_DEFAULT)
    , m_interestResendTime(HELLO_TIMEOUT_DEFAULT)
    , m_infoInterestInterval(HELLO_INTERVAL_DEFAULT)
    , m_hyperbolicState(HYPERBOLIC_STATE_OFF)
    , m_corR(0)
    , m_corTheta(0)
    , m_maxFacesPerPrefix(MAX_FACES_PER_PREFIX_MIN)
  {}

  void
  setNetwork(const ndn::Name& networkName)
  {
    m_network = networkName;
    m_chronosyncPrefix = m_network;
    m_chronosyncPrefix.append("nlsr");
    m_chronosyncPrefix.append("sync");
    
    m_lsaPrefix = m_network;
    m_lsaPrefix.append("nlsr");
    m_lsaPrefix.append("LSA");
  }

  const ndn::Name&
  getNetwork()
  {
    return m_network;
  }

  void
  setRouterName(const ndn::Name& routerName)
  {
    m_routerName = routerName;
  }

  const ndn::Name&
  getRouterName()
  {
    return m_routerName;
  }

  void
  setSiteName(const ndn::Name& siteName)
  {
    m_siteName = siteName;
  }

  const ndn::Name&
  getSiteName()
  {
    return m_siteName;
  }

  void
  buildRouterPrefix()
  {
    m_routerPrefix = m_network;
    m_routerPrefix.append(m_siteName);
    m_routerPrefix.append(m_routerName);
  }

  const ndn::Name&
  getRouterPrefix()
  {
    return m_routerPrefix;
  }


  const ndn::Name&
  getChronosyncPrefix()
  {
    return m_chronosyncPrefix;
  }

  const ndn::Name&
  getLsaPrefix()
  {
    return m_lsaPrefix;
  }

  void
  setLsaRefreshTime(int32_t lrt)
  {
    m_lsaRefreshTime = lrt;
    m_routerDeadInterval = 2 * m_lsaRefreshTime;
  }

  int32_t
  getLsaRefreshTime()
  {
    return m_lsaRefreshTime;
  }

  void
  setRouterDeadInterval(int64_t rdt)
  {
    m_routerDeadInterval = rdt;
  }

  int64_t
  getRouterDeadInterval()
  {
    return m_routerDeadInterval;
  }
  
  void 
  setLogLevel(const std::string& logLevel)
  {
    m_logLevel = logLevel;
  }
  
  const std::string& 
  getLogLevel()
  {
    return m_logLevel;
  }

  void
  setInterestRetryNumber(uint32_t irn)
  {
    m_interestRetryNumber = irn;
  }

  uint32_t
  getInterestRetryNumber()
  {
    return m_interestRetryNumber;
  }

  void
  setInterestResendTime(int32_t irt)
  {
    m_interestResendTime = irt;
  }

  int32_t
  getInterestResendTime()
  {
    return m_interestResendTime;
  }

  int32_t
  getInfoInterestInterval()
  {
    return m_infoInterestInterval;
  }

  void
  setInfoInterestInterval(int32_t iii)
  {
    m_infoInterestInterval = iii;
  }

  void
  setHyperbolicState(int32_t ihc)
  {
    m_hyperbolicState = ihc;
  }

  int32_t
  getHyperbolicState()
  {
    return m_hyperbolicState;
  }

  bool
  setCorR(double cr)
  {
    if ( cr >= 0 ) {
     m_corR = cr;
     return true;
    }
    return false;
  }

  double
  getCorR()
  {
    return m_corR;
  }

  void
  setCorTheta(double ct)
  {
    m_corTheta = ct;
  }

  double
  getCorTheta()
  {
    return m_corTheta;
  }
  
  void
  setMaxFacesPerPrefix(int32_t mfpp)
  {
    m_maxFacesPerPrefix = mfpp;
  }

  int32_t
  getMaxFacesPerPrefix()
  {
    return m_maxFacesPerPrefix;
  }

  void
  setSeqFileDir(const std::string& ssfd)
  {
    m_seqFileDir = ssfd;
  }

  const std::string&
  getSeqFileDir()
  {
    return m_seqFileDir;
  }


private:
  ndn::Name m_routerName;
  ndn::Name m_siteName;
  ndn::Name m_network;

  ndn::Name m_routerPrefix;
  ndn::Name m_lsaRouterPrefix;

  ndn::Name m_chronosyncPrefix;
  ndn::Name m_lsaPrefix;

  int32_t  m_lsaRefreshTime;
  int64_t  m_routerDeadInterval;
  std::string m_logLevel;

  uint32_t m_interestRetryNumber;
  int32_t  m_interestResendTime;
  
  
  int32_t  m_infoInterestInterval;
  
  int32_t m_hyperbolicState;
  double m_corR;
  double m_corTheta;

  int32_t m_maxFacesPerPrefix;
  
  std::string m_seqFileDir;

};

inline std::ostream&
operator<<(std::ostream& os, ConfParameter& cfp)
{
  os  << "Router Name: " << cfp.getRouterName() << std::endl;
  os  << "Site Name: " << cfp.getSiteName() << std::endl;
  os  << "Network: " << cfp.getNetwork() << std::endl;
  os  << "Router Prefix: " << cfp.getRouterPrefix() << std::endl;
  os  << "ChronoSync sync Prifex: " << cfp.getChronosyncPrefix() << std::endl;
  os  << "ChronoSync LSA prefix: " << cfp.getLsaPrefix() << std::endl;
  os  << "Interest Retry number: " << cfp.getInterestRetryNumber() << std::endl;
  os  << "Interest Resend second: " << cfp.getInterestResendTime() << std::endl;
  os  << "Info Interest Interval: " << cfp.getInfoInterestInterval() << std::endl;
  os  << "LSA refresh time: " << cfp.getLsaRefreshTime() << std::endl;
  os  << "Max Faces Per Prefix: " << cfp.getMaxFacesPerPrefix() << std::endl;
  os  << "Hyperbolic ROuting: " << cfp.getHyperbolicState() << std::endl;
  os  << "Hyp R: " << cfp.getCorR() << std::endl;
  os  << "Hyp theta: " << cfp.getCorTheta() << std::endl;
  return  os;
}

} // namespace nlsr

#endif //CONF_PARAMETER_HPP
