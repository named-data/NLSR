#include <cstdlib>
#include <string>
#include <sstream>
#include <cstdio>

#include "nlsr.hpp"


namespace nlsr {

using namespace ndn;
using namespace std;

void
Nlsr::registrationFailed(const ndn::Name& name)
{
  std::cerr << "ERROR: Failed to register prefix in local hub's daemon" << endl;
  throw Error("Error: Prefix registration failed");
}


void
Nlsr::setInfoInterestFilter()
{
  ndn::Name name(m_confParam.getRouterPrefix());
  getNlsrFace().setInterestFilter(name,
                                  ndn::bind(&HelloProtocol::processInterest,
                                            &m_helloProtocol, _1, _2),
                                  ndn::bind(&Nlsr::registrationFailed, this, _1));
}

void
Nlsr::setLsaInterestFilter()
{
  // ndn::Name name(m_confParam.getChronosyncLsaPrefix() +
  //                m_confParam.getRouterPrefix());
  ndn::Name name = m_confParam.getChronosyncLsaPrefix();
  name.append(m_confParam.getRouterPrefix());
  getNlsrFace().setInterestFilter(name,
                                  ndn::bind(&Lsdb::processInterest,
                                            &m_nlsrLsdb, _1, _2),
                                  ndn::bind(&Nlsr::registrationFailed, this, _1));
}

void
Nlsr::initialize()
{
  m_confParam.buildRouterPrefix();
  m_nlsrLsdb.setLsaRefreshTime(m_confParam.getLsaRefreshTime());
  m_nlsrLsdb.setThisRouterPrefix(m_confParam.getRouterPrefix().toUri());
  m_fib.setEntryRefreshTime(2 * m_confParam.getLsaRefreshTime());
  m_sequencingManager.setSeqFileName(m_confParam.getSeqFileDir());
  m_sequencingManager.initiateSeqNoFromFile();
  /* debugging purpose start */
  cout << m_confParam;
  m_adjacencyList.print();
  m_namePrefixList.print();
  /* debugging purpose end */
  m_nlsrLsdb.buildAndInstallOwnNameLsa();
  m_nlsrLsdb.buildAndInstallOwnCoordinateLsa();
  setInfoInterestFilter();
  setLsaInterestFilter();
  m_syncLogicHandler.setSyncPrefix(m_confParam.getChronosyncSyncPrefix().toUri());
  m_syncLogicHandler.createSyncSocket(boost::ref(*this));
  //m_interestManager.scheduleInfoInterest(10);
  m_helloProtocol.scheduleInterest(10);
}

void
Nlsr::startEventLoop()
{
  m_nlsrFace.processEvents();
}

void
Nlsr::usage(const string& progname)
{
  cout << "Usage: " << progname << " [OPTIONS...]" << endl;
  cout << "   NDN routing...." << endl;
  cout << "       -d, --daemon        Run in daemon mode" << endl;
  cout << "       -f, --config_file   Specify configuration file name" << endl;
  cout << "       -p, --api_port      port where api client will connect" << endl;
  cout << "       -h, --help          Display this help message" << endl;
}


} // namespace nlsr
