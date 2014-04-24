#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>

#include "conf-file-processor.hpp"
#include "conf-parameter.hpp"
#include "utility/tokenizer.hpp"
#include "adjacent.hpp"


namespace nlsr {

using namespace std;

int
ConfFileProcessor::processConfFile()
{
  int ret = 0;
  if (!m_confFileName.empty())
  {
    std::ifstream inputFile(m_confFileName.c_str());
    if (inputFile.is_open())
    {
      for (string line; getline(inputFile, line);)
      {
        if (!line.empty())
        {
          if (line[0] != '#' && line[0] != '!')
          {
            ret = processConfCommand(line);
            if (ret == -1)
            {
              break;
            }
          }
        }
      }
    }
    else
    {
      std::cerr << "Configuration file: (" << m_confFileName << ") does not exist :(";
      std::cerr << endl;
      ret = -1;
    }
  }
  return ret;
}


int
ConfFileProcessor::processConfCommand(string command)
{
  int ret = 0;
  Tokenizer nt(command, " ");
  if ((nt.getFirstToken() == "network"))
  {
    ret = processConfCommandNetwork(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "site-name"))
  {
    ret = processConfCommandSiteName(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "root-key-prefix"))
  {
    ret = processConfCommandRootKeyPrefix(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "router-name"))
  {
    ret = processConfCommandRouterName(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "ndnneighbor"))
  {
    ret = processConfCommandNdnNeighbor(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "link-cost"))
  {
    ret = processConfCommandLinkCost(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "ndnname"))
  {
    ret = processConfCommandNdnName(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "interest-retry-num"))
  {
    processConfCommandInterestRetryNumber(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "interest-resend-time"))
  {
    processConfCommandInterestResendTime(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "lsa-refresh-time"))
  {
    processConfCommandLsaRefreshTime(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "max-faces-per-prefix"))
  {
    processConfCommandMaxFacesPerPrefix(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "log-dir"))
  {
    processConfCommandLogDir(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "cert-dir"))
  {
    processConfCommandCertDir(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "detailed-logging"))
  {
    processConfCommandDetailedLogging(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "debugging"))
  {
    processConfCommandDebugging(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "chronosync-sync-prefix"))
  {
    processConfCommandChronosyncSyncPrefix(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "hyperbolic-cordinate"))
  {
    processConfCommandHyperbolicCordinate(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "hyperbolic-routing"))
  {
    processConfCommandIsHyperbolicCalc(nt.getRestOfLine());
  }
  else if ((nt.getFirstToken() == "tunnel-type"))
  {
    processConfCommandTunnelType(nt.getRestOfLine());
  }
  else
  {
    cout << "Wrong configuration Command: " << nt.getFirstToken() << endl;
  }
  return ret;
}

int
ConfFileProcessor::processConfCommandNetwork(string command)
{
  if (command.empty())
  {
    cerr << " Network can not be null or empty :( !" << endl;
    return -1;
  }
  else
  {
    if (command[command.size() - 1] == '/')
    {
      command.erase(command.size() - 1);
    }
    if (command[0] == '/')
    {
      command.erase(0, 1);
    }
    m_nlsr.getConfParameter().setNetwork(command);
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandSiteName(string command)
{
  if (command.empty())
  {
    cerr << "Site name can not be null or empty :( !" << endl;
    return -1;
  }
  else
  {
    if (command[command.size() - 1] == '/')
    {
      command.erase(command.size() - 1);
    }
    if (command[0] == '/')
    {
      command.erase(0, 1);
    }
    m_nlsr.getConfParameter().setSiteName(command);
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandRootKeyPrefix(string command)
{
  if (command.empty())
  {
    cerr << "Root Key Prefix can not be null or empty :( !" << endl;
    return -1;
  }
  else
  {
    if (command[command.size() - 1] == '/')
    {
      command.erase(command.size() - 1);
    }
    if (command[0] == '/')
    {
      command.erase(0, 1);
    }
    m_nlsr.getConfParameter().setRootKeyPrefix(command);
  }
  return 0;
}


int
ConfFileProcessor::processConfCommandRouterName(string command)
{
  if (command.empty())
  {
    cerr << " Router name can not be null or empty :( !" << endl;
    return -1;
  }
  else
  {
    if (command[command.size() - 1] == '/')
    {
      command.erase(command.size() - 1);
    }
    if (command[0] == '/')
    {
      command.erase(0, 1);
    }
    m_nlsr.getConfParameter().setRouterName(command);
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandInterestRetryNumber(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [interest-retry-num n]" << endl;
  }
  else
  {
    int irn;
    stringstream ss(command.c_str());
    ss >> irn;
    if (irn >= 1 && irn <= 5)
    {
      m_nlsr.getConfParameter().setInterestRetryNumber(irn);
    }
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandInterestResendTime(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [interest-resend-time s]" << endl;
  }
  else
  {
    int irt;
    stringstream ss(command.c_str());
    ss >> irt;
    if (irt >= 1 && irt <= 20)
    {
      m_nlsr.getConfParameter().setInterestResendTime(irt);
    }
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandLsaRefreshTime(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [interest-resend-time s]" << endl;
  }
  else
  {
    int lrt;
    stringstream ss(command.c_str());
    ss >> lrt;
    if (lrt >= 240 && lrt <= 7200)
    {
      m_nlsr.getConfParameter().setLsaRefreshTime(lrt);
    }
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandMaxFacesPerPrefix(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [max-faces-per-prefix n]" << endl;
  }
  else
  {
    int mfpp;
    stringstream ss(command.c_str());
    ss >> mfpp;
    if (mfpp >= 0 && mfpp <= 60)
    {
      m_nlsr.getConfParameter().setMaxFacesPerPrefix(mfpp);
    }
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandTunnelType(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [tunnel-type tcp/udp]!" << endl;
  }
  else
  {
    if (command == "tcp" || command == "TCP")
    {
      m_nlsr.getConfParameter().setTunnelType(1);
    }
    else if (command == "udp" || command == "UDP")
    {
      m_nlsr.getConfParameter().setTunnelType(0);
    }
    else
    {
      cerr << " Wrong command format ! [tunnel-type tcp/udp]!" << endl;
    }
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandChronosyncSyncPrefix(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [chronosync-sync-prefix name/prefix]!" << endl;
  }
  else
  {
    m_nlsr.getConfParameter().setChronosyncSyncPrefix(command);
  }
  return 0;
}


int
ConfFileProcessor::processConfCommandLogDir(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [log-dir /path/to/log/dir]!" << endl;
  }
  else
  {
    m_nlsr.getConfParameter().setLogDir(command);
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandCertDir(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [cert-dir /path/to/cert/dir]!" << endl;
  }
  else
  {
    m_nlsr.getConfParameter().setCertDir(command);
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandDebugging(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [debugging on/of]!" << endl;
  }
  else
  {
    if (command == "on" || command == "ON")
    {
      m_nlsr.getConfParameter().setDebugging(1);
    }
    else if (command == "off" || command == "off")
    {
      m_nlsr.getConfParameter().setDebugging(0);
    }
    else
    {
      cerr << " Wrong command format ! [debugging on/off]!" << endl;
    }
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandDetailedLogging(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [detailed-logging on/off]!" << endl;
  }
  else
  {
    if (command == "on" || command == "ON")
    {
      m_nlsr.getConfParameter().setDetailedLogging(1);
    }
    else if (command == "off" || command == "off")
    {
      m_nlsr.getConfParameter().setDetailedLogging(0);
    }
    else
    {
      cerr << " Wrong command format ! [detailed-logging on/off]!" << endl;
    }
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandIsHyperbolicCalc(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [hyperbolic-routing on/off/dry-run]!" << endl;
  }
  else
  {
    if (command == "on" || command == "ON")
    {
      m_nlsr.getConfParameter().setIsHyperbolicCalc(1);
    }
    else if (command == "dry-run" || command == "DRY-RUN")
    {
      m_nlsr.getConfParameter().setIsHyperbolicCalc(2);
    }
    else if (command == "off" || command == "off")
    {
      m_nlsr.getConfParameter().setIsHyperbolicCalc(0);
    }
    else
    {
      cerr << " Wrong command format ! [hyperbolic-routing on/off/dry-run]!" << endl;
    }
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandHyperbolicCordinate(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [hyperbolic-cordinate r 0]!" << endl;
    if (m_nlsr.getConfParameter().getIsHyperbolicCalc() > 0)
    {
      return -1;
    }
  }
  else
  {
    Tokenizer nt(command, " ");
    stringstream ssr(nt.getFirstToken().c_str());
    stringstream sst(nt.getRestOfLine().c_str());
    double r, theta;
    ssr >> r;
    sst >> theta;
    m_nlsr.getConfParameter().setCorR(r);
    m_nlsr.getConfParameter().setCorTheta(theta);
  }
  return 0;
}


int
ConfFileProcessor::processConfCommandNdnNeighbor(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [ndnneighbor /nbr/name/ FaceId]!" << endl;
  }
  else
  {
    Tokenizer nt(command, " ");
    if (nt.getRestOfLine().empty())
    {
      cerr << " Wrong command format ! [ndnneighbor /nbr/name/ FaceId]!" << endl;
      return 0;
    }
    else
    {
      stringstream sst(nt.getRestOfLine().c_str());
      int faceId;
      sst >> faceId;
      Adjacent adj(nt.getFirstToken(), faceId, 0.0, 0, 0);
      m_nlsr.getAdl().insert(adj);
    }
  }
  return 0;
}

int
ConfFileProcessor::processConfCommandNdnName(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [ndnname name/prefix]!" << endl;
  }
  else
  {
    m_nlsr.getNpl().insert(command);
  }
  return 0;
}


int
ConfFileProcessor::processConfCommandLinkCost(string command)
{
  if (command.empty())
  {
    cerr << " Wrong command format ! [link-cost nbr/name cost]!" << endl;
    if (m_nlsr.getConfParameter().getIsHyperbolicCalc() > 0)
    {
      return -1;
    }
  }
  else
  {
    Tokenizer nt(command, " ");
    stringstream sst(nt.getRestOfLine().c_str());
    double cost;
    sst >> cost;
    m_nlsr.getAdl().updateAdjacentLinkCost(nt.getFirstToken(), cost);
  }
  return 0;
}

} //namespace nlsr

