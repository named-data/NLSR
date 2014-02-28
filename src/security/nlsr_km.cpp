#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/encoding/block.hpp>
#include "nlsr_sm.hpp"
#include "nlsr_km.hpp"

namespace nlsr
{
    void
    KeyManager::initKeyManager(ConfParameter &cp)
    {
        string processIdentityName(cp.getRootKeyPrefix());
        processIdentityName += "/";
        processIdentityName += cp.getSiteName();
        processIdentityName += "/";
        processIdentityName += "R.Start";
        processIdentityName += "/";
        processIdentityName += cp.getRouterName();
        processIdentityName += "/";
        processIdentityName += "nlsr";
        cout<<"Proces Identity Name: "<<processIdentityName<<endl;
        ndn::Name identityName(processIdentityName);
        routerIdentity=identityName;
        ndn::KeyChain::deleteIdentity(routerIdentity);
        routerCertName = ndn::KeyChain::createIdentity(routerIdentity);
        cout<<"Certificate Name: "<<routerCertName.toUri()<<endl;
        routerKeyName=
            ndn::IdentityCertificate::certificateNameToPublicKeyName(routerCertName);
        cout<<"Key Name: "<<routerKeyName.toUri()<<endl;
        initCertSeqFromFile(cp.getSeqFileDir());
    }

    ndn::Name
    KeyManager::getRouterCertName()
    {
        return routerCertName;
    }

    uint32_t
    KeyManager::getCertSeqNo()
    {
        return certSeqNo;
    }

    void
    KeyManager::setCerSeqNo(uint32_t csn)
    {
        certSeqNo=csn;
    }

    void
    KeyManager::initCertSeqFromFile(string certSeqFileDir)
    {
        certSeqFileNameWithPath=certSeqFileDir;
        if( certSeqFileNameWithPath.empty() )
        {
            SequencingManager sm;
            certSeqFileNameWithPath=sm.getUserHomeDirectory();
        }
        certSeqFileNameWithPath += "/nlsrCertSeqNo.txt";
        cout<<"Key Seq File Name: "<< certSeqFileNameWithPath<<endl;
        std::ifstream inputFile(certSeqFileNameWithPath.c_str(),ios::binary);
        if ( inputFile.good() )
        {
            inputFile>>certSeqNo;
            certSeqNo++;
        }
        else
        {
            certSeqNo=1;
        }
        writeCertSeqToFile();
    }

    void
    KeyManager::writeCertSeqToFile()
    {
        std::ofstream outputFile(certSeqFileNameWithPath.c_str(),ios::binary);
        outputFile<<certSeqNo;
        outputFile.close();
    }
}



