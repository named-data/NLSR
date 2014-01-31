#ifndef CONF_PROCESSOR_HPP
#define CONF_PROCESSOR_HPP

#include "nlsr.hpp" 

using namespace std;

class ConfFileProcessor{
	public:
		ConfFileProcessor()
			:confFileName()
		{
		}
		ConfFileProcessor(const string& cfile){ 
			confFileName=cfile;
		}

		int processConfFile(nlsr& pnlsr);
		int processConfCommand(nlsr& pnlsr, string command);
		int processConfCommandNetwork(nlsr& pnlsr, string command);
		int processConfCommandSiteName(nlsr& pnlsr, string command);
		int processConfCommandRouterName(nlsr& pnlsr, string command);
		int processConfCommandInterestRetryNumber(nlsr& pnlsr, string command);
		int processConfCommandInterestResendTime(nlsr& pnlsr, string command);
		int processConfCommandLsaRefreshTime(nlsr& pnlsr, string command);
		int processConfCommandMaxFacesPerPrefix(nlsr& pnlsr, string command);
		int processConfCommandTunnelType(nlsr& pnlsr, string command);

		int processConfCommandChronosyncSyncPrefix(nlsr& pnlsr, string command);
		int processConfCommandLogDir(nlsr& pnlsr, string command);
		int processConfCommandDebugging(nlsr& pnlsr, string command);
		int processConfCommandDetailedLogging(nlsr& pnlsr, string command);
		int processConfCommandIsHyperbolicCalc(nlsr& pnlsr, string command);

		int processConfCommandHyperbolicCordinate(nlsr& pnlsr, string command);

		int processConfCommandNdnNeighbor(nlsr& pnlsr, string command);
		int processConfCommandNdnName(nlsr& pnlsr, string command);
		int processConfCommandLinkCost(nlsr& pnlsr, string command);
		

	private:
		string confFileName;
};

#endif
