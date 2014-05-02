#ifndef CONF_PARAMETER_HPP
#define CONF_PARAMETER_HPP

#include <iostream>
#include <boost/cstdint.hpp>

namespace nlsr {
class ConfParameter
{

public:
  ConfParameter()
    : m_chronosyncSyncPrefix("ndn/nlsr/sync")
    , m_chronosyncLsaPrefix("/ndn/nlsr/LSA")
    , m_rootKeyPrefix("/ndn/keys")
    , m_interestRetryNumber(3)
    , m_interestResendTime(5)
    , m_infoInterestInterval(60)
    , m_lsaRefreshTime(1800)
    , m_routerDeadInterval(3600)
    , m_maxFacesPerPrefix(0)
    , m_tunnelType(0)
    , m_detailedLogging(0)
    , m_certDir()
    , m_debugging(0)
    , m_seqFileDir()
    , m_isHyperbolicCalc(0)
    , m_corR(0)
    , m_corTheta(0)
  {}

  void
  setRouterName(const std::string& rn)
  {
    m_routerName = rn;
  }

  const std::string&
  getRouterName()
  {
    return m_routerName;
  }

  void
  setSiteName(const std::string& sn)
  {
    m_siteName = sn;
  }

  const std::string&
  getSiteName()
  {
    return m_siteName;
  }

  void
  setNetwork(const std::string& nn)
  {
    m_network = nn;
  }

  const std::string&
  getNetwork()
  {
    return m_network;
  }

  void
  buildRouterPrefix()
  {
    m_routerPrefix = "/" + m_network + "/" + m_siteName + "/" + m_routerName;
  }

  const std::string&
  getRouterPrefix()
  {
    return m_routerPrefix;
  }

  const std::string&
  getRootKeyPrefix()
  {
    return m_rootKeyPrefix;
  }

  void
  setRootKeyPrefix(std::string rkp)
  {
    m_rootKeyPrefix = rkp;
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
  setLogDir(const std::string& ld)
  {
    m_logDir = ld;
  }

  std::string
  getLogDir()
  {
    return m_logDir;
  }

  void
  setCertDir(const std::string& cd)
  {
    m_certDir = cd;
  }

  const std::string&
  getCertDir()
  {
    return m_certDir;
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

  void
  setDetailedLogging(int dl)
  {
    m_detailedLogging = dl;
  }

  int
  getDetailedLogging()
  {
    return m_detailedLogging;
  }

  void
  setDebugging(int32_t d)
  {
    m_debugging = d;
  }

  int32_t
  getDebugging()
  {
    return m_debugging;
  }

  void
  setIsHyperbolicCalc(int32_t ihc)
  {
    m_isHyperbolicCalc = ihc;
  }

  int32_t
  getIsHyperbolicCalc()
  {
    return m_isHyperbolicCalc;
  }

  void
  setCorR(double cr)
  {
    m_corR = cr;
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
  setTunnelType(int tt)
  {
    m_tunnelType = tt;
  }

  int32_t
  getTunnelType()
  {
    return m_tunnelType;
  }

  void
  setChronosyncSyncPrefix(const std::string& csp)
  {
    m_chronosyncSyncPrefix = csp;
  }

  const std::string&
  getChronosyncSyncPrefix()
  {
    return m_chronosyncSyncPrefix;
  }

  void
  setChronosyncLsaPrefix(const std::string& clp)
  {
    m_chronosyncLsaPrefix = clp;
  }

  const std::string&
  getChronosyncLsaPrefix()
  {
    return m_chronosyncLsaPrefix;
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

private:
  std::string m_routerName;
  std::string m_siteName;
  std::string m_network;

  std::string m_routerPrefix;
  std::string m_lsaRouterPrefix;

  std::string m_chronosyncSyncPrefix;
  std::string m_chronosyncLsaPrefix;
  std::string m_rootKeyPrefix;

  uint32_t m_interestRetryNumber;
  int32_t m_interestResendTime;
  int32_t m_infoInterestInterval;
  int32_t m_lsaRefreshTime;
  int64_t m_routerDeadInterval;

  int32_t m_maxFacesPerPrefix;
  int32_t m_tunnelType;
  int32_t m_detailedLogging;

  std::string m_certDir;
  int32_t m_debugging;
  std::string m_seqFileDir;

  int32_t m_isHyperbolicCalc;
  double m_corR;
  double m_corTheta;

  std::string m_logFile;
  std::string m_logDir;

};

std::ostream&
operator<<(std::ostream& os, ConfParameter& cfp);

} // namespace nlsr

#endif //CONF_PARAMETER_HPP
