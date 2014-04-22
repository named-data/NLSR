#ifndef CONF_PARAM_HPP
#define CONF_PARAM_HPP

#include <iostream>

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

  std::string
  getRouterName()
  {
    return m_routerName;
  }

  void
  setSiteName(const std::string& sn)
  {
    m_siteName = sn;
  }

  std::string
  getSiteName()
  {
    return m_siteName;
  }

  void
  setNetwork(const std::string& nn)
  {
    m_network = nn;
  }

  std::string
  getNetwork()
  {
    return m_network;
  }

  void
  buildRouterPrefix()
  {
    m_routerPrefix = "/" + m_network + "/" + m_siteName + "/" + m_routerName;
  }

  std::string
  getRouterPrefix()
  {
    return m_routerPrefix;
  }

  std::string
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
  setInterestRetryNumber(int irn)
  {
    m_interestRetryNumber = irn;
  }

  int
  getInterestRetryNumber()
  {
    return m_interestRetryNumber;
  }

  void
  setInterestResendTime(int irt)
  {
    m_interestResendTime = irt;
  }

  int
  getInterestResendTime()
  {
    return m_interestResendTime;
  }

  void
  setLsaRefreshTime(int lrt)
  {
    m_lsaRefreshTime = lrt;
    m_routerDeadInterval = 2 * m_lsaRefreshTime;
  }

  int
  getLsaRefreshTime()
  {
    return m_lsaRefreshTime;
  }

  void
  setRouterDeadInterval(int rdt)
  {
    m_routerDeadInterval = rdt;
  }

  long int
  getRouterDeadInterval()
  {
    return m_routerDeadInterval;
  }

  void
  setMaxFacesPerPrefix(int mfpp)
  {
    m_maxFacesPerPrefix = mfpp;
  }

  int
  getMaxFacesPerPrefix()
  {
    return m_maxFacesPerPrefix;
  }

  void
  setLogDir(std::string ld)
  {
    m_logDir = ld;
  }

  std::string
  getLogDir()
  {
    return m_logDir;
  }

  void
  setCertDir(std::string cd)
  {
    m_certDir = cd;
  }

  std::string
  getCertDir()
  {
    return m_certDir;
  }

  void
  setSeqFileDir(std::string ssfd)
  {
    m_seqFileDir = ssfd;
  }

  std::string
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
  setDebugging(int d)
  {
    m_debugging = d;
  }

  int
  getDebugging()
  {
    return m_debugging;
  }

  void
  setIsHyperbolicCalc(int ihc)
  {
    m_isHyperbolicCalc = ihc;
  }

  int
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

  int
  getTunnelType()
  {
    return m_tunnelType;
  }

  void
  setChronosyncSyncPrefix(const std::string& csp)
  {
    m_chronosyncSyncPrefix = csp;
  }

  std::string
  getChronosyncSyncPrefix()
  {
    return m_chronosyncSyncPrefix;
  }

  void
  setChronosyncLsaPrefix(std::string clp)
  {
    m_chronosyncLsaPrefix = clp;
  }

  std::string
  getChronosyncLsaPrefix()
  {
    return m_chronosyncLsaPrefix;
  }

  int
  getInfoInterestInterval()
  {
    return m_infoInterestInterval;
  }

  void
  setInfoInterestInterval(int iii)
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

  int m_interestRetryNumber;
  int m_interestResendTime;
  int m_infoInterestInterval;
  int m_lsaRefreshTime;
  int m_routerDeadInterval;

  int m_maxFacesPerPrefix;
  int m_tunnelType;
  int m_detailedLogging;

  std::string m_certDir;
  int m_debugging;
  std::string m_seqFileDir;

  int m_isHyperbolicCalc;
  double m_corR;
  double m_corTheta;

  std::string m_logFile;
  std::string m_logDir;

};

std::ostream&
operator<<(std::ostream& os, ConfParameter& cfp);

} // namespace nlsr

#endif //CONF_PARAM_HPP
