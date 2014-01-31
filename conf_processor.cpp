#include<iostream>
#include<fstream>
#include<string>
#include<cstdlib>
#include <sstream>

#include "conf_processor.hpp"
#include "conf_param.hpp"
#include "nlsr_tokenizer.hpp"
#include "adjacent.hpp"


using namespace std;

int 
ConfFileProcessor::processConfFile(nlsr& pnlsr){
	int ret=0;

	if ( !confFileName.empty()){
		std::ifstream inputFile(confFileName.c_str());
		for( string line; getline( inputFile, line ); ){
    			if (!line.empty() ){
				if(line[0]!= '#' && line[0]!='!'){
					ret=processConfCommand(pnlsr, line);	
					if( ret == -1 ){
						break;
					}
				}
			}
		}
	}

	return ret;
}


int 
ConfFileProcessor::processConfCommand(nlsr& pnlsr, string command){
	int ret=0;
	nlsrTokenizer nt(command," ");
	if( (nt.getFirstToken() == "network")){
		ret=processConfCommandNetwork(pnlsr,nt.getRestOfLine());		
    }
	else if( (nt.getFirstToken() == "site-name")){
     	ret=processConfCommandSiteName(pnlsr,nt.getRestOfLine());   		
    }
	else if ( (nt.getFirstToken() == "router-name")){
		ret=processConfCommandRouterName(pnlsr,nt.getRestOfLine());
	}
	else if( (nt.getFirstToken() == "ndnneighbor") ){
        ret=processConfCommandNdnNeighbor(pnlsr, nt.getRestOfLine());    
    }
	else if( (nt.getFirstToken() == "link-cost")){
      	ret=processConfCommandLinkCost(pnlsr, nt.getRestOfLine());          
 	} 
	else if( (nt.getFirstToken() == "ndnname") ){
 		ret=processConfCommandNdnName(pnlsr, nt.getRestOfLine());
	}
	else if( (nt.getFirstToken() == "interest-retry-num")){
		processConfCommandInterestRetryNumber(pnlsr,nt.getRestOfLine());
	}
	else if( (nt.getFirstToken() == "interest-resend-time")){
    		processConfCommandInterestResendTime(pnlsr,nt.getRestOfLine());
    }
	else if( (nt.getFirstToken() == "lsa-refresh-time")){
    		processConfCommandLsaRefreshTime(pnlsr,nt.getRestOfLine());    		
	}
	else if( (nt.getFirstToken() == "max-faces-per-prefix")){
    		processConfCommandMaxFacesPerPrefix(pnlsr,nt.getRestOfLine());	
	}
	else if( (nt.getFirstToken() == "logdir")){
    		processConfCommandLogDir(pnlsr,nt.getRestOfLine());
	}
	else if( (nt.getFirstToken() == "detailed-logging") ){
    		processConfCommandDetailedLogging(pnlsr,nt.getRestOfLine());
    }
    else if( (nt.getFirstToken() == "debugging") ){
    		processConfCommandDebugging(pnlsr,nt.getRestOfLine());    		
    }
    else if( (nt.getFirstToken() == "chronosync-sync-prefix") ){
    		processConfCommandChronosyncSyncPrefix(pnlsr,nt.getRestOfLine());    		
    }
    else if( (nt.getFirstToken() == "hyperbolic-cordinate") ){
     	processConfCommandHyperbolicCordinate(pnlsr,nt.getRestOfLine());   		
    }
    else if( (nt.getFirstToken() == "hyperbolic-routing")){
    		processConfCommandIsHyperbolicCalc(pnlsr,nt.getRestOfLine());    		
    }
    else if( (nt.getFirstToken() == "tunnel-type")){
     	processConfCommandTunnelType(pnlsr,nt.getRestOfLine());   		
    }
    else {
    		cout << "Wrong configuration Command: "<< nt.getFirstToken()<<endl;
    }

	return ret;
}

int
ConfFileProcessor::processConfCommandNetwork(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Network can not be null or empty :( !"<<endl;
		return -1;
	}else{
		if(command[command.size()-1] == '/' ){
			command.erase(command.size() - 1);
		}
		if(command[0] == '/' ){
			command.erase(0,1);
		}
		pnlsr.confParam.setNetwork(command);
	}
	return 0;
}

int 
ConfFileProcessor::processConfCommandSiteName(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<"Site name can not be null or empty :( !"<<endl;
		return -1;
	}else{
		if(command[command.size()-1] == '/' ){
			command.erase(command.size() - 1);
		}
		if(command[0] == '/' ){
			command.erase(0,1);
		}
		pnlsr.confParam.setSiteName(command);
	}
	return 0;
}

int 
ConfFileProcessor::processConfCommandRouterName(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Router name can not be null or empty :( !"<<endl;
		return -1;
	}else{
		if(command[command.size()-1] == '/' ){
			command.erase(command.size() - 1);
		}
		if(command[0] == '/' ){
			command.erase(0,1);
		}
		pnlsr.confParam.setRouterName(command);
	}
	return 0;
}

int 
ConfFileProcessor::processConfCommandInterestRetryNumber(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [interest-retry-num n]"<<endl;
	}else{
		int irn;
		stringstream ss(command.c_str());
		ss>>irn;
		if ( irn >=1 && irn <=5){
			pnlsr.confParam.setInterestRetryNumber(irn);
		}
	}
	return 0;
}

int 
ConfFileProcessor::processConfCommandInterestResendTime(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [interest-resend-time s]"<<endl;
	}else{
		int irt;
		stringstream ss(command.c_str());
		ss>>irt;
		if( irt>=1 && irt <=20){
			pnlsr.confParam.setInterestResendTime(irt);
		}
	}
	return 0;
}

int 
ConfFileProcessor::processConfCommandLsaRefreshTime(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [interest-resend-time s]"<<endl;
	}else{
		int lrt;
		stringstream ss(command.c_str());
		ss>>lrt;
		if ( lrt>= 240 && lrt<=7200){
			pnlsr.confParam.setLsaRefreshTime(lrt);
		}
	}
	return 0;
}

int 
ConfFileProcessor::processConfCommandMaxFacesPerPrefix(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [max-faces-per-prefix n]"<<endl;
	}else{
		int mfpp;
		stringstream ss(command.c_str());
		ss>>mfpp;
		if ( mfpp>=0 && mfpp<=60){
			pnlsr.confParam.setMaxFacesPerPrefix(mfpp);
		}
	}
	return 0;
}

int
ConfFileProcessor::processConfCommandTunnelType(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [tunnel-type tcp/udp]!"<<endl;
	}else{
		if(command == "tcp" || command == "TCP" ){
			pnlsr.confParam.setTunnelType(1);
		}
		else if(command == "udp" || command == "UDP"){
			pnlsr.confParam.setTunnelType(0);
		}else{
			cerr <<" Wrong command format ! [tunnel-type tcp/udp]!"<<endl;
		}
	}
	return 0;
}

int
ConfFileProcessor::processConfCommandChronosyncSyncPrefix(nlsr& pnlsr, 
																string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [chronosync-sync-prefix name/prefix]!"<<endl;
	}else{
		pnlsr.confParam.setChronosyncSyncPrefix(command);
	}
	return 0;
}


int
ConfFileProcessor::processConfCommandLogDir(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [log-dir /path/to/log/dir]!"<<endl;
	}else{
		pnlsr.confParam.setLogDir(command);
	}
	return 0;
}

int
ConfFileProcessor::processConfCommandDebugging(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [debugging on/of]!"<<endl;
	}else{
		if(command == "on" || command == "ON" ){
			pnlsr.confParam.setDebugging(1);
		}
		else if(command == "off" || command == "off"){
			pnlsr.confParam.setDebugging(0);
		}else{
			cerr <<" Wrong command format ! [debugging on/off]!"<<endl;
		}
	}
	return 0;
}

int
ConfFileProcessor::processConfCommandDetailedLogging(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [detailed-logging on/off]!"<<endl;
	}else{
		if(command == "on" || command == "ON" ){
			pnlsr.confParam.setDetailedLogging(1);
		}
		else if(command == "off" || command == "off"){
			pnlsr.confParam.setDetailedLogging(0);
		}else{
			cerr <<" Wrong command format ! [detailed-logging on/off]!"<<endl;
		}
	}
	return 0;
}

int
ConfFileProcessor::processConfCommandIsHyperbolicCalc(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [hyperbolic-routing on/off/dry-run]!"<<endl;
	}else{
		if(command == "on" || command == "ON" ){
			pnlsr.confParam.setIsHyperbolicCalc(1);
		}
		else if(command == "dry-run" || command == "DRY-RUN"){
			pnlsr.confParam.setIsHyperbolicCalc(2);
		}
		else if(command == "off" || command == "off"){
			pnlsr.confParam.setIsHyperbolicCalc(0);
		}else{
			cerr <<" Wrong command format ! [hyperbolic-routing on/off/dry-run]!"<<endl;
		}
	}
	return 0;
}

int
ConfFileProcessor::processConfCommandHyperbolicCordinate(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [hyperbolic-cordinate r 0]!"<<endl;
		if (pnlsr.confParam.getIsHyperbolicCalc() > 0 ){
			return -1;
		}
	}else{
		nlsrTokenizer nt(command," ");
		stringstream ssr(nt.getFirstToken().c_str());
		stringstream sst(nt.getRestOfLine().c_str());
		
		double r,theta;
		ssr>>r;
		sst>>theta;
		
		pnlsr.confParam.setCorR(r);
		pnlsr.confParam.setCorTheta(theta);
	}
	return 0;
}


int 
ConfFileProcessor::processConfCommandNdnNeighbor(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [ndnneighbor /nbr/name/]!"<<endl;
	}else{
		nlsrTokenizer nt(command," ");
		Adjacent adj(nt.getFirstToken(),0,0.0,0,0);
		pnlsr.adl.insert(adj);
	}
	return 0;	
}

int 
ConfFileProcessor::processConfCommandNdnName(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [ndnname name/prefix]!"<<endl;
	}else{
		pnlsr.npl.insertIntoNpl(command);
	}
	return 0;
}


int 
ConfFileProcessor::processConfCommandLinkCost(nlsr& pnlsr, string command){
	if(command.empty() ){
		cerr <<" Wrong command format ! [link-cost nbr/name cost]!"<<endl;
		if (pnlsr.confParam.getIsHyperbolicCalc() > 0 ){
			return -1;
		}
	}else{
		nlsrTokenizer nt(command," ");
		stringstream sst(nt.getRestOfLine().c_str());
		
		double cost;
		sst>>cost;
		
		pnlsr.adl.updateAdjacentLinkCost(nt.getFirstToken(),cost);
	}
	return 0;
}

