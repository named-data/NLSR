#include <cstdlib>
#include <string>
#include <sstream>
#include <cstdio>
#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>


#include "nlsr.hpp"


namespace nlsr {

using namespace ndn;
using namespace std;

void
Nlsr::registrationFailed(const ndn::Name& name)
{
  cerr << "ERROR: Failed to register prefix in local hub's daemon" << endl;
  getNlsrFace()->shutdown();
}


void
Nlsr::setInterestFilterNlsr(const string& name)
{
  getNlsrFace()->setInterestFilter(name,
                                   func_lib::bind(&InterestManager::processInterest, &m_im,
                                                  boost::ref(*this), _1, _2),
                                   func_lib::bind(&Nlsr::registrationFailed, this, _1));
}

void
Nlsr::initialize()
{
  m_confParam.buildRouterPrefix();
  m_nlsrLsdb.setLsaRefreshTime(m_confParam.getLsaRefreshTime());
  m_nlsrLsdb.setThisRouterPrefix(m_confParam.getRouterPrefix());
  m_fib.setEntryRefreshTime(2 * m_confParam.getLsaRefreshTime());
  if (!m_km.initialize(m_confParam))
  {
    std::cerr << "Can not initiate/load certificate" << endl;
  }
  m_sm.setSeqFileName(m_confParam.getSeqFileDir());
  m_sm.initiateSeqNoFromFile();
  /* debugging purpose start */
  cout <<	m_confParam;
  m_adl.printAdl();
  m_npl.print();
  /* debugging purpose end */
  m_nlsrLsdb.buildAndInstallOwnNameLsa(boost::ref(*this));
  m_nlsrLsdb.buildAndInstallOwnCorLsa(boost::ref(*this));
  setInterestFilterNlsr(m_confParam.getRouterPrefix());
  setInterestFilterNlsr(m_confParam.getChronosyncLsaPrefix() +
                        m_confParam.getRouterPrefix());
  setInterestFilterNlsr(m_confParam.getRootKeyPrefix());
  m_slh.setSyncPrefix(m_confParam.getChronosyncSyncPrefix());
  m_slh.createSyncSocket(boost::ref(*this));
  m_slh.publishKeyUpdate(m_km);
  m_im.scheduleInfoInterest(boost::ref(*this), 10);
}

void
Nlsr::startEventLoop()
{
  m_io->run();
}

int
Nlsr::usage(const string& progname)
{
  cout << "Usage: " << progname << " [OPTIONS...]" << endl;
  cout << "   NDN routing...." << endl;
  cout << "       -d, --daemon        Run in daemon mode" << endl;
  cout << "       -f, --config_file   Specify configuration file name" << endl;
  cout << "       -p, --api_port      port where api client will connect" << endl;
  cout << "       -h, --help          Display this help message" << endl;
  exit(EXIT_FAILURE);
}


} // namespace nlsr
