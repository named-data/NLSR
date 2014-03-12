#include<iostream>
#include "nlsr_conf_param.hpp"

namespace nlsr
{

  using namespace std;

  ostream&
  operator << (ostream &os, ConfParameter& cfp)
  {
    os  <<"Router Name: "<< cfp.getRouterName()<<endl;
    os  <<"Site Name: "<< cfp.getSiteName()<<endl;
    os  <<"Network: "<< cfp.getNetwork()<<endl;
    os  <<"Router Prefix: "<< cfp.getRouterPrefix()<<endl;
    os  <<"ChronoSync sync Prifex: "<< cfp.getChronosyncSyncPrefix()<<endl;
    os  <<"Interest Retry number: "<< cfp.getInterestRetryNumber()<<endl;
    os  <<"Interest Resend second: "<< cfp.getInterestResendTime()<<endl;
    os  <<"Info Interest Interval: "<<cfp.getInfoInterestInterval()<<endl;
    os  <<"LSA refresh time: "<< cfp.getLsaRefreshTime()<<endl;
    os  <<"Max Faces Per Prefix: "<< cfp.getMaxFacesPerPrefix()<<endl;
    os  <<"Log Dir: "<< cfp.getLogDir()<<endl;
    os  <<"Detalied logging: "<< cfp.getDetailedLogging()<<endl;
    os  <<"Debugging: "<< cfp.getDebugging()<<endl;
    os  <<"Hyperbolic ROuting: "<< cfp.getIsHyperbolicCalc()<<endl;
    os  <<"Hyp R: "<< cfp.getCorR()<<endl;
    os  <<"Hyp theta: "<< cfp.getCorTheta()<<endl;
    os  <<"Tunnel Type: "<< cfp.getTunnelType()<<endl;
    return os;
  }

} //namespace nlsr
