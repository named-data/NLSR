#include <ndn-cpp-dev/security/signature-sha256-with-rsa.hpp>
#include "nlsr_cse.hpp"

namespace nlsr
{
  std::ostream&
  operator <<(std::ostream& os, const NlsrCertificateStoreEntry& ncse)
  {
    os<<"------Certificate Entry---------------"<<std::endl;
    os<<*(ncse.getCert())<<std::endl;
    ndn::SignatureSha256WithRsa sig(ncse.getCert()->getSignature());
    ndn::Name keyName=sig.getKeyLocator().getName();
    os<<"Signee : "<<keyName.toUri()<<std::endl;
    os<<"Cert Seq Num: "<<ncse.getCertSeqNum()<<std::endl;
    os<<"Is Signer Verified: "<<ncse.getIsSignerVerified()<<std::endl;
    return os;
  }
}
