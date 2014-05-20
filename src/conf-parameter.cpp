#include "conf-parameter.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("ConfParameter");

void
ConfParameter::writeLog()
{
  _LOG_DEBUG("Router Name: " << m_routerName);
  _LOG_DEBUG("Site Name: " << m_siteName);
  _LOG_DEBUG("Network: " << m_network);
  _LOG_DEBUG("Router Prefix: " << m_routerPrefix);
  _LOG_DEBUG("ChronoSync sync Prifex: " << m_chronosyncPrefix);
  _LOG_DEBUG("ChronoSync LSA prefix: " << m_lsaPrefix);
  _LOG_DEBUG("Interest Retry number: " << m_interestRetryNumber);
  _LOG_DEBUG("Interest Resend second: " << m_interestResendTime);
  _LOG_DEBUG("Info Interest Interval: " << m_infoInterestInterval);
  _LOG_DEBUG("LSA refresh time: " << m_lsaRefreshTime);
  _LOG_DEBUG("Max Faces Per Prefix: " << m_maxFacesPerPrefix);
  _LOG_DEBUG("Hyperbolic ROuting: " << m_hyperbolicState);
  _LOG_DEBUG("Hyp R: " << m_corR);
  _LOG_DEBUG("Hyp theta: " << m_corTheta);
  _LOG_DEBUG("Log Directory: " << m_logDir);
  _LOG_DEBUG("Seq Directory: " << m_seqFileDir);
}

} // namespace nlsr
