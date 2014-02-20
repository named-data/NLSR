#include<iostream>
#include<fstream>
#include<string>
#include<cstdlib>
#include <sstream>

#include "nlsr_conf_processor.hpp"
#include "nlsr_conf_param.hpp"
#include "nlsr_tokenizer.hpp"
#include "nlsr_adjacent.hpp"


namespace nlsr
{

    using namespace std;

    int
    ConfFileProcessor::processConfFile(Nlsr& pnlsr)
    {
        int ret=0;
        if ( !confFileName.empty())
        {
            std::ifstream inputFile(confFileName.c_str());
            if ( inputFile.is_open())
            {
                for( string line; getline( inputFile, line ); )
                {
                    if (!line.empty() )
                    {
                        if(line[0]!= '#' && line[0]!='!')
                        {
                            ret=processConfCommand(pnlsr, line);
                            if( ret == -1 )
                            {
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                std::cerr <<"Configuration file: ("<<confFileName<<") does not exist :(";
                std::cerr <<endl;
                ret=-1;
            }
        }
        return ret;
    }


    int
    ConfFileProcessor::processConfCommand(Nlsr& pnlsr, string command)
    {
        int ret=0;
        nlsrTokenizer nt(command," ");
        if( (nt.getFirstToken() == "network"))
        {
            ret=processConfCommandNetwork(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "site-name"))
        {
            ret=processConfCommandSiteName(pnlsr,nt.getRestOfLine());
        }
        else if ( (nt.getFirstToken() == "router-name"))
        {
            ret=processConfCommandRouterName(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "ndnneighbor") )
        {
            ret=processConfCommandNdnNeighbor(pnlsr, nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "link-cost"))
        {
            ret=processConfCommandLinkCost(pnlsr, nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "ndnname") )
        {
            ret=processConfCommandNdnName(pnlsr, nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "interest-retry-num"))
        {
            processConfCommandInterestRetryNumber(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "interest-resend-time"))
        {
            processConfCommandInterestResendTime(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "lsa-refresh-time"))
        {
            processConfCommandLsaRefreshTime(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "max-faces-per-prefix"))
        {
            processConfCommandMaxFacesPerPrefix(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "logdir"))
        {
            processConfCommandLogDir(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "detailed-logging") )
        {
            processConfCommandDetailedLogging(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "debugging") )
        {
            processConfCommandDebugging(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "chronosync-sync-prefix") )
        {
            processConfCommandChronosyncSyncPrefix(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "hyperbolic-cordinate") )
        {
            processConfCommandHyperbolicCordinate(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "hyperbolic-routing"))
        {
            processConfCommandIsHyperbolicCalc(pnlsr,nt.getRestOfLine());
        }
        else if( (nt.getFirstToken() == "tunnel-type"))
        {
            processConfCommandTunnelType(pnlsr,nt.getRestOfLine());
        }
        else
        {
            cout << "Wrong configuration Command: "<< nt.getFirstToken()<<endl;
        }
        return ret;
    }

    int
    ConfFileProcessor::processConfCommandNetwork(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<" Network can not be null or empty :( !"<<endl;
            return -1;
        }
        else
        {
            if(command[command.size()-1] == '/' )
            {
                command.erase(command.size() - 1);
            }
            if(command[0] == '/' )
            {
                command.erase(0,1);
            }
            pnlsr.getConfParameter().setNetwork(command);
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandSiteName(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<"Site name can not be null or empty :( !"<<endl;
            return -1;
        }
        else
        {
            if(command[command.size()-1] == '/' )
            {
                command.erase(command.size() - 1);
            }
            if(command[0] == '/' )
            {
                command.erase(0,1);
            }
            pnlsr.getConfParameter().setSiteName(command);
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandRouterName(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<" Router name can not be null or empty :( !"<<endl;
            return -1;
        }
        else
        {
            if(command[command.size()-1] == '/' )
            {
                command.erase(command.size() - 1);
            }
            if(command[0] == '/' )
            {
                command.erase(0,1);
            }
            pnlsr.getConfParameter().setRouterName(command);
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandInterestRetryNumber(Nlsr& pnlsr,
            string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [interest-retry-num n]"<<endl;
        }
        else
        {
            int irn;
            stringstream ss(command.c_str());
            ss>>irn;
            if ( irn >=1 && irn <=5)
            {
                pnlsr.getConfParameter().setInterestRetryNumber(irn);
            }
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandInterestResendTime(Nlsr& pnlsr,
            string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [interest-resend-time s]"<<endl;
        }
        else
        {
            int irt;
            stringstream ss(command.c_str());
            ss>>irt;
            if( irt>=1 && irt <=20)
            {
                pnlsr.getConfParameter().setInterestResendTime(irt);
            }
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandLsaRefreshTime(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [interest-resend-time s]"<<endl;
        }
        else
        {
            int lrt;
            stringstream ss(command.c_str());
            ss>>lrt;
            if ( lrt>= 240 && lrt<=7200)
            {
                pnlsr.getConfParameter().setLsaRefreshTime(lrt);
            }
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandMaxFacesPerPrefix(Nlsr& pnlsr,
            string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [max-faces-per-prefix n]"<<endl;
        }
        else
        {
            int mfpp;
            stringstream ss(command.c_str());
            ss>>mfpp;
            if ( mfpp>=0 && mfpp<=60)
            {
                pnlsr.getConfParameter().setMaxFacesPerPrefix(mfpp);
            }
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandTunnelType(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [tunnel-type tcp/udp]!"<<endl;
        }
        else
        {
            if(command == "tcp" || command == "TCP" )
            {
                pnlsr.getConfParameter().setTunnelType(1);
            }
            else if(command == "udp" || command == "UDP")
            {
                pnlsr.getConfParameter().setTunnelType(0);
            }
            else
            {
                cerr <<" Wrong command format ! [tunnel-type tcp/udp]!"<<endl;
            }
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandChronosyncSyncPrefix(Nlsr& pnlsr,
            string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [chronosync-sync-prefix name/prefix]!"<<endl;
        }
        else
        {
            pnlsr.getConfParameter().setChronosyncSyncPrefix(command);
        }
        return 0;
    }


    int
    ConfFileProcessor::processConfCommandLogDir(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [log-dir /path/to/log/dir]!"<<endl;
        }
        else
        {
            pnlsr.getConfParameter().setLogDir(command);
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandDebugging(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [debugging on/of]!"<<endl;
        }
        else
        {
            if(command == "on" || command == "ON" )
            {
                pnlsr.getConfParameter().setDebugging(1);
            }
            else if(command == "off" || command == "off")
            {
                pnlsr.getConfParameter().setDebugging(0);
            }
            else
            {
                cerr <<" Wrong command format ! [debugging on/off]!"<<endl;
            }
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandDetailedLogging(Nlsr& pnlsr,
            string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [detailed-logging on/off]!"<<endl;
        }
        else
        {
            if(command == "on" || command == "ON" )
            {
                pnlsr.getConfParameter().setDetailedLogging(1);
            }
            else if(command == "off" || command == "off")
            {
                pnlsr.getConfParameter().setDetailedLogging(0);
            }
            else
            {
                cerr <<" Wrong command format ! [detailed-logging on/off]!"<<endl;
            }
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandIsHyperbolicCalc(Nlsr& pnlsr,
            string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [hyperbolic-routing on/off/dry-run]!"<<endl;
        }
        else
        {
            if(command == "on" || command == "ON" )
            {
                pnlsr.getConfParameter().setIsHyperbolicCalc(1);
            }
            else if(command == "dry-run" || command == "DRY-RUN")
            {
                pnlsr.getConfParameter().setIsHyperbolicCalc(2);
            }
            else if(command == "off" || command == "off")
            {
                pnlsr.getConfParameter().setIsHyperbolicCalc(0);
            }
            else
            {
                cerr <<" Wrong command format ! [hyperbolic-routing on/off/dry-run]!"<<endl;
            }
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandHyperbolicCordinate(Nlsr& pnlsr,
            string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [hyperbolic-cordinate r 0]!"<<endl;
            if (pnlsr.getConfParameter().getIsHyperbolicCalc() > 0 )
            {
                return -1;
            }
        }
        else
        {
            nlsrTokenizer nt(command," ");
            stringstream ssr(nt.getFirstToken().c_str());
            stringstream sst(nt.getRestOfLine().c_str());
            double r,theta;
            ssr>>r;
            sst>>theta;
            pnlsr.getConfParameter().setCorR(r);
            pnlsr.getConfParameter().setCorTheta(theta);
        }
        return 0;
    }


    int
    ConfFileProcessor::processConfCommandNdnNeighbor(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [ndnneighbor /nbr/name/ FaceId]!"<<endl;
        }
        else
        {
            nlsrTokenizer nt(command," ");
            if( nt.getRestOfLine().empty())
            {
                cerr <<" Wrong command format ! [ndnneighbor /nbr/name/ FaceId]!"<<endl;
                return 0;
            }
            else
            {
                stringstream sst(nt.getRestOfLine().c_str());
                int faceId;
                sst>>faceId;
                Adjacent adj(nt.getFirstToken(),faceId,0.0,0,0);
                pnlsr.getAdl().insert(adj);
            }
        }
        return 0;
    }

    int
    ConfFileProcessor::processConfCommandNdnName(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [ndnname name/prefix]!"<<endl;
        }
        else
        {
            pnlsr.getNpl().insertIntoNpl(command);
        }
        return 0;
    }


    int
    ConfFileProcessor::processConfCommandLinkCost(Nlsr& pnlsr, string command)
    {
        if(command.empty() )
        {
            cerr <<" Wrong command format ! [link-cost nbr/name cost]!"<<endl;
            if (pnlsr.getConfParameter().getIsHyperbolicCalc() > 0 )
            {
                return -1;
            }
        }
        else
        {
            nlsrTokenizer nt(command," ");
            stringstream sst(nt.getRestOfLine().c_str());
            double cost;
            sst>>cost;
            pnlsr.getAdl().updateAdjacentLinkCost(nt.getFirstToken(),cost);
        }
        return 0;
    }

} //namespace nlsr

