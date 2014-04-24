#ifndef CONF_PROCESSOR_HPP
#define CONF_PROCESSOR_HPP

#include "nlsr.hpp"

namespace nlsr {

class ConfFileProcessor
{
public:
  ConfFileProcessor(Nlsr& nlsr, const string& cfile)
    : m_confFileName(cfile)
    , m_nlsr(nlsr)
  {
  }

  int processConfFile();

private:
  int
  processConfCommand(string command);

  int
  processConfCommandNetwork(string command);

  int
  processConfCommandSiteName(string command);

  int
  processConfCommandRootKeyPrefix(string command);

  int
  processConfCommandRouterName(string command);

  int
  processConfCommandInterestRetryNumber(string command);

  int
  processConfCommandInterestResendTime(string command);

  int
  processConfCommandLsaRefreshTime(string command);

  int
  processConfCommandMaxFacesPerPrefix(string command);

  int
  processConfCommandTunnelType(string command);

  int
  processConfCommandChronosyncSyncPrefix(string command);

  int
  processConfCommandLogDir(string command);

  int
  processConfCommandCertDir(string command);

  int
  processConfCommandDebugging(string command);

  int
  processConfCommandDetailedLogging(string command);

  int
  processConfCommandIsHyperbolicCalc(string command);

  int
  processConfCommandHyperbolicCordinate(string command);

  int
  processConfCommandNdnNeighbor(string command);

  int
  processConfCommandNdnName(string command);

  int
  processConfCommandLinkCost(string command);


private:
  string m_confFileName;
  Nlsr& m_nlsr;
};

} //namespace nlsr
#endif //CONF_PROCESSOR_HPP
