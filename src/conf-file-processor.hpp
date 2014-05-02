#ifndef CONF_FILE_PROCESSOR_HPP
#define CONF_FILE_PROCESSOR_HPP

#include <boost/cstdint.hpp>
#include "nlsr.hpp"

namespace nlsr {

class ConfFileProcessor
{
public:
  ConfFileProcessor(Nlsr& nlsr, const std::string& cfile)
    : m_confFileName(cfile)
    , m_nlsr(nlsr)
  {
  }

  int processConfFile();

private:
  int
  processConfCommand(std::string command);

  int
  processConfCommandNetwork(std::string command);

  int
  processConfCommandSiteName(std::string command);

  int
  processConfCommandRootKeyPrefix(std::string command);

  int
  processConfCommandRouterName(std::string command);

  int
  processConfCommandInterestRetryNumber(std::string command);

  int
  processConfCommandInterestResendTime(std::string command);

  int
  processConfCommandLsaRefreshTime(std::string command);

  int
  processConfCommandMaxFacesPerPrefix(std::string command);

  int
  processConfCommandTunnelType(std::string command);

  int
  processConfCommandChronosyncSyncPrefix(std::string command);

  int
  processConfCommandLogDir(std::string command);

  int
  processConfCommandCertDir(std::string command);

  int
  processConfCommandDebugging(std::string command);

  int
  processConfCommandDetailedLogging(std::string command);

  int
  processConfCommandIsHyperbolicCalc(std::string command);

  int
  processConfCommandHyperbolicCordinate(std::string command);

  int
  processConfCommandNdnNeighbor(std::string command);

  int
  processConfCommandNdnName(std::string command);

  int
  processConfCommandLinkCost(std::string command);


private:
  std::string m_confFileName;
  Nlsr& m_nlsr;
};

} //namespace nlsr
#endif //CONF_FILE_PROCESSOR_HPP
