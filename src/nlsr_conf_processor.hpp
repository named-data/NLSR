#ifndef CONF_PROCESSOR_HPP
#define CONF_PROCESSOR_HPP

#include "nlsr.hpp"

namespace nlsr
{

  using namespace std;

  class ConfFileProcessor
  {
  public:
    ConfFileProcessor()
      :m_confFileName()
    {
    }
    ConfFileProcessor(const string& cfile)
    {
      m_confFileName=cfile;
    }

    int processConfFile(Nlsr& pnlsr);

  private:
    int processConfCommand(Nlsr& pnlsr, string command);
    int processConfCommandNetwork(Nlsr& pnlsr, string command);
    int processConfCommandSiteName(Nlsr& pnlsr, string command);
    int processConfCommandRootKeyPrefix(Nlsr& pnlsr, string command);
    int processConfCommandRouterName(Nlsr& pnlsr, string command);
    int processConfCommandInterestRetryNumber(Nlsr& pnlsr, string command);
    int processConfCommandInterestResendTime(Nlsr& pnlsr, string command);
    int processConfCommandLsaRefreshTime(Nlsr& pnlsr, string command);
    int processConfCommandMaxFacesPerPrefix(Nlsr& pnlsr, string command);
    int processConfCommandTunnelType(Nlsr& pnlsr, string command);

    int processConfCommandChronosyncSyncPrefix(Nlsr& pnlsr, string command);
    int processConfCommandLogDir(Nlsr& pnlsr, string command);
    int processConfCommandCertDir(Nlsr& pnlsr, string command);
    int processConfCommandDebugging(Nlsr& pnlsr, string command);
    int processConfCommandDetailedLogging(Nlsr& pnlsr, string command);
    int processConfCommandIsHyperbolicCalc(Nlsr& pnlsr, string command);

    int processConfCommandHyperbolicCordinate(Nlsr& pnlsr, string command);

    int processConfCommandNdnNeighbor(Nlsr& pnlsr, string command);
    int processConfCommandNdnName(Nlsr& pnlsr, string command);
    int processConfCommandLinkCost(Nlsr& pnlsr, string command);


  private:
    string m_confFileName;
  };

} //namespace nlsr
#endif
