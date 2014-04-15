#ifndef CONF_PARAM_HPP
#define CONF_PARAM_HPP

#include<iostream>

namespace nlsr
{

  using namespace std;

  class ConfParameter
  {

  public:
    ConfParameter()
      : m_chronosyncSyncPrefix("ndn/nlsr/sync")
      , m_chronosyncLsaPrefix("/ndn/nlsr/LSA")
      , m_rootKeyPrefix("/ndn/keys")
      , isStrictHierchicalKeyCheck(0)
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
      , isHyperbolicCalc(0)
      , m_seqFileDir()
      , m_corR(0)
      , m_corTheta(0)
    {}

    void setRouterName(const string& rn)
    {
      m_routerName=rn;
    }

    string getRouterName()
    {
      return m_routerName;
    }

    void setSiteName(const string& sn)
    {
      m_siteName=sn;
    }

    string getSiteName()
    {
      return m_siteName;
    }

    void setNetwork(const string& nn)
    {
      m_network=nn;
    }

    string getNetwork()
    {
      return m_network;
    }

    void buildRouterPrefix()
    {
      m_routerPrefix="/"+m_network+"/"+m_siteName+"/"+m_routerName;
    }

    string getRouterPrefix()
    {
      return m_routerPrefix;
    }

    string getRootKeyPrefix()
    {
      return m_rootKeyPrefix;
    }

    void setRootKeyPrefix(string rkp)
    {
      m_rootKeyPrefix=rkp;
    }

    void setInterestRetryNumber(int irn)
    {
      m_interestRetryNumber=irn;
    }

    int getInterestRetryNumber()
    {
      return m_interestRetryNumber;
    }

    void setInterestResendTime(int irt)
    {
      m_interestResendTime=irt;
    }

    int getInterestResendTime()
    {
      return m_interestResendTime;
    }

    void setLsaRefreshTime(int lrt)
    {
      m_lsaRefreshTime=lrt;
      m_routerDeadInterval=2*m_lsaRefreshTime;
    }

    int getLsaRefreshTime()
    {
      return m_lsaRefreshTime;
    }

    void setRouterDeadInterval(int rdt)
    {
      m_routerDeadInterval=rdt;
    }

    long int getRouterDeadInterval()
    {
      return m_routerDeadInterval;
    }

    void setMaxFacesPerPrefix(int mfpp)
    {
      m_maxFacesPerPrefix=mfpp;
    }

    int getMaxFacesPerPrefix()
    {
      return m_maxFacesPerPrefix;
    }

    void setLogDir(string ld)
    {
      m_logDir=ld;
    }

    string getLogDir()
    {
      return m_logDir;
    }

    void setCertDir(std::string cd)
    {
      m_certDir=cd;
    }

    std::string getCertDir()
    {
      return m_certDir;
    }

    void setSeqFileDir(string ssfd)
    {
      m_seqFileDir=ssfd;
    }

    string getSeqFileDir()
    {
      return m_seqFileDir;
    }

    void setDetailedLogging(int dl)
    {
      m_detailedLogging=dl;
    }

    int getDetailedLogging()
    {
      return m_detailedLogging;
    }

    void setDebugging(int d)
    {
      m_debugging=d;
    }

    int getDebugging()
    {
      return m_debugging;
    }

    void setIsHyperbolicCalc(bool ihc)
    {
      isHyperbolicCalc=ihc;
    }

    bool getIsHyperbolicCalc()
    {
      return isHyperbolicCalc;
    }

    void setCorR(double cr)
    {
      m_corR=cr;
    }

    double getCorR()
    {
      return m_corR;
    }

    void setCorTheta(double ct)
    {
      m_corTheta=ct;
    }

    double getCorTheta()
    {
      return m_corTheta;
    }

    void setTunnelType(int tt)
    {
      m_tunnelType=tt;
    }

    int getTunnelType()
    {
      return m_tunnelType;
    }

    void setChronosyncSyncPrefix(const string& csp)
    {
      m_chronosyncSyncPrefix=csp;
    }

    string getChronosyncSyncPrefix()
    {
      return m_chronosyncSyncPrefix;
    }

    void setChronosyncLsaPrefix(string clp)
    {
      m_chronosyncLsaPrefix=clp;
    }

    string getChronosyncLsaPrefix()
    {
      return m_chronosyncLsaPrefix;
    }

    int getInfoInterestInterval()
    {
      return m_infoInterestInterval;
    }

    void setInfoInterestInterval(int iii)
    {
      m_infoInterestInterval=iii;
    }

  private:
    string m_routerName;
    string m_siteName;
    string m_network;

    string m_routerPrefix;
    string m_lsaRouterPrefix;

    string m_chronosyncSyncPrefix;
    string m_chronosyncLsaPrefix;

    string m_rootKeyPrefix;

    int m_interestRetryNumber;
    int m_interestResendTime;
    int m_infoInterestInterval;
    int m_lsaRefreshTime;
    int m_routerDeadInterval;

    int m_maxFacesPerPrefix;
    string m_logDir;
    string m_certDir;
    string m_seqFileDir;
    string m_logFile;
    int m_detailedLogging;
    int m_debugging;

    bool isHyperbolicCalc;
    double m_corR;
    double m_corTheta;

    int m_tunnelType;
    bool isStrictHierchicalKeyCheck;

  };

  std::ostream&
  operator << (std::ostream& os, ConfParameter& cfp);

} // namespace nlsr

#endif
