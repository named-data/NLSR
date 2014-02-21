#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/encoding/block.hpp>
#include "nlsr_km.hpp"

namespace nlsr
{
    void
    KeyManager::initKeyManager(ConfParameter &cp)
    {
        ndn::Name identityName(cp.getRouterPrefix()+"/nlsr");
        kChain.deleteIdentity(identityName);
        ndn::Name certName = kChain.createIdentity(identityName);
        cout<<"Certificate Name: "<<certName.toUri()<<endl;
        ndn::Name keyName=
            ndn::IdentityCertificate::certificateNameToPublicKeyName(certName);
        cout<<"Key Name: "<<keyName.toUri()<<endl;
    }

}



