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
      : chronosyncSyncPrefix("ndn/nlsr/sync")
      , chronosyncLsaPrefix("/ndn/nlsr/LSA")
      , rootKeyPrefix("/ndn/keys")
      , isStrictHierchicalKeyCheck(0)
      , interestRetryNumber(3)
      , interestResendTime(5)
      , infoInterestInterval(60)
      , lsaRefreshTime(1800)
      , routerDeadInterval(3600)
      , maxFacesPerPrefix(0)
      , tunnelType(0)
      , detailedLogging(0)
      , certDir()
      , debugging(0)
      , isHyperbolicCalc(0)
      , seqFileDir()
      , corR(0)
      , corTheta(0)
    {}

    void setRouterName(const string& rn)
    {
      routerName=rn;
    }

    string getRouterName()
    {
      return routerName;
    }

    void setSiteName(const string& sn)
    {
      siteName=sn;
    }

    string getSiteName()
    {
      return siteName;
    }

    void setNetwork(const string& nn)
    {
      network=nn;
    }

    string getNetwork()
    {
      return network;
    }

    void buildRouterPrefix()
    {
      routerPrefix="/"+network+"/"+siteName+"/"+routerName;
    }

    string getRouterPrefix()
    {
      return routerPrefix;
    }

    string getRootKeyPrefix()
    {
      return rootKeyPrefix;
    }

    void setRootKeyPrefix(string rkp)
    {
      rootKeyPrefix=rkp;
    }

    void setInterestRetryNumber(int irn)
    {
      interestRetryNumber=irn;
    }

    int getInterestRetryNumber()
    {
      return interestRetryNumber;
    }

    void setInterestResendTime(int irt)
    {
      interestResendTime=irt;
    }

    int getInterestResendTime()
    {
      return interestResendTime;
    }

    void setLsaRefreshTime(int lrt)
    {
      lsaRefreshTime=lrt;
      routerDeadInterval=2*lsaRefreshTime;
    }

    int getLsaRefreshTime()
    {
      return lsaRefreshTime;
    }

    void setRouterDeadInterval(int rdt)
    {
      routerDeadInterval=rdt;
    }

    long int getRouterDeadInterval()
    {
      return routerDeadInterval;
    }

    void setMaxFacesPerPrefix(int mfpp)
    {
      maxFacesPerPrefix=mfpp;
    }

    int getMaxFacesPerPrefix()
    {
      return maxFacesPerPrefix;
    }

    void setLogDir(string ld)
    {
      logDir=ld;
    }

    string getLogDir()
    {
      return logDir;
    }

    void setCertDir(std::string cd)
    {
      certDir=cd;
    }

    std::string getCertDir()
    {
      return certDir;
    }

    void setSeqFileDir(string ssfd)
    {
      seqFileDir=ssfd;
    }

    string getSeqFileDir()
    {
      return seqFileDir;
    }

    void setDetailedLogging(int dl)
    {
      detailedLogging=dl;
    }

    int getDetailedLogging()
    {
      return detailedLogging;
    }

    void setDebugging(int d)
    {
      debugging=d;
    }

    int getDebugging()
    {
      return debugging;
    }

    void setIsHyperbolicCalc(int ihc)
    {
      isHyperbolicCalc=ihc;
    }

    int getIsHyperbolicCalc()
    {
      return isHyperbolicCalc;
    }

    void setCorR(double cr)
    {
      corR=cr;
    }

    double getCorR()
    {
      return corR;
    }

    void setCorTheta(double ct)
    {
      corTheta=ct;
    }

    double getCorTheta()
    {
      return corTheta;
    }

    void setTunnelType(int tt)
    {
      tunnelType=tt;
    }

    int getTunnelType()
    {
      return tunnelType;
    }

    void setChronosyncSyncPrefix(const string& csp)
    {
      chronosyncSyncPrefix=csp;
    }

    string getChronosyncSyncPrefix()
    {
      return chronosyncSyncPrefix;
    }

    void setChronosyncLsaPrefix(string clp)
    {
      chronosyncLsaPrefix=clp;
    }

    string getChronosyncLsaPrefix()
    {
      return chronosyncLsaPrefix;
    }

    int getInfoInterestInterval()
    {
      return infoInterestInterval;
    }

    void setInfoInterestInterval(int iii)
    {
      infoInterestInterval=iii;
    }

  private:
    string routerName;
    string siteName;
    string network;

    string routerPrefix;
    string lsaRouterPrefix;

    string chronosyncSyncPrefix;
    string chronosyncLsaPrefix;

    string rootKeyPrefix;

    int interestRetryNumber;
    int interestResendTime;
    int infoInterestInterval;
    int lsaRefreshTime;
    int routerDeadInterval;

    int maxFacesPerPrefix;
    string logDir;
    string certDir;
    string seqFileDir;
    string logFile;
    int detailedLogging;
    int debugging;

    int isHyperbolicCalc;
    double corR;
    double corTheta;

    int tunnelType;
    int isStrictHierchicalKeyCheck;

  };

  std::ostream&
  operator << (std::ostream &os, ConfParameter &cfp);

} // namespace nlsr

#endif
