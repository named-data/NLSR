#include "nlsr_cert_store.hpp"

namespace nlsr
{
    static bool
    nlsrCertificateStoreEntryCompare(NlsrCertificateStoreEntry& ncse1,
                                               NlsrCertificateStoreEntry& ncse2)
    
    {
        return ncse1.getCert()->getName().toUri() == 
                                          ncse2.getCert()->getName().toUri() ;
    }
    
    static bool
    nlsrCertificateStoreEntryCompareByName(NlsrCertificateStoreEntry& ncse1,
                                               std::string compCertName)
    
    {
        return ncse1.getCert()->getName().toUri() == compCertName ;
    }
    
    bool
    NlsrCertificateStore::addCertificate(NlsrCertificateStoreEntry & ncse)
    {
        std::list<NlsrCertificateStoreEntry>::iterator it = 
                               std::find_if( certTable.begin(), certTable.end(),
                             bind(&nlsrCertificateStoreEntryCompare, _1, ncse));
        if(it == certTable.end())
        {
            certTable.push_back(ncse);
            return true;
        }
        
        if( it !=  certTable.end() )
        {
            if ( (*it).getCertSeqNum() < ncse.getCertSeqNum() )
            {
                certTable.erase(it);
                certTable.push_back(ncse);
                return true;
            }
        }
        
        return false;
    }
    
    bool
    NlsrCertificateStore::addCertificate(
        ndn::shared_ptr<ndn::IdentityCertificate> pcert, uint32_t csn, bool isv)
    {
        NlsrCertificateStoreEntry ncse(pcert, csn, isv);
        return addCertificate(ncse);
    }
    
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
    NlsrCertificateStore::getCertificateFromStore(const std::string certName)
    {
        std::list<NlsrCertificateStoreEntry>::iterator it = 
                               std::find_if( certTable.begin(), certTable.end(),
                   bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
        if(it == certTable.end())
        {
            ndn::shared_ptr<ndn::IdentityCertificate> cert=
                                   ndn::make_shared<ndn::IdentityCertificate>();
                                   
            return std::make_pair(cert,false);
        }
        
        return std::make_pair((*it).getCert(),true);
    }
    
    std::pair<ndn::shared_ptr<ndn::IdentityCertificate>, bool>
    NlsrCertificateStore::getCertificateFromStore(
                                    const std::string certName, int checkSeqNum)
    {
        std::list<NlsrCertificateStoreEntry>::iterator it = 
                               std::find_if( certTable.begin(), certTable.end(),
                   bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
        if(it == certTable.end())
        {
            ndn::shared_ptr<ndn::IdentityCertificate> cert=
                                   ndn::make_shared<ndn::IdentityCertificate>();
                                   
            return std::make_pair(cert,false);
        }
        else
        {
            if( (*it).getCertSeqNum() == checkSeqNum )
            {
                return std::make_pair((*it).getCert(),true);
            }
        }
           
        return std::make_pair((*it).getCert(),false);
        
    }
    
    bool 
    NlsrCertificateStore::isCertificateNewInStore(const std::string certName,
                                                            int checkSeqNo)
    {
        std::list<NlsrCertificateStoreEntry>::iterator it = 
                               std::find_if( certTable.begin(), certTable.end(),
                   bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
        if(it != certTable.end())
        {
            return (*it).getCertSeqNum() < checkSeqNo ;
        }
        
        return true;
        
    }
    
    bool
    NlsrCertificateStore::removeCertificateFromStroe(const std::string certName)
    {
        std::list<NlsrCertificateStoreEntry>::iterator it = 
                               std::find_if( certTable.begin(), certTable.end(),
                   bind(&nlsrCertificateStoreEntryCompareByName, _1, certName));
        if(it != certTable.end())
        {
            certTable.erase(it);
            return true;
        }
        
        return false;
    }
    
    void 
    NlsrCertificateStore::printCertStore()
    {
        std::list<NlsrCertificateStoreEntry>::iterator it;
        for(it=certTable.begin(); it!=certTable.end(); ++it)
        {
            std::cout<<(*it)<<std::endl;
        }
        
    }
}
