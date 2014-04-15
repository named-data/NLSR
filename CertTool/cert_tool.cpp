#include <ndn-cpp-dev/data.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/random.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/security/certificate-subject-description.hpp>
#include <ndn-cpp-dev/security/signature-sha256-with-rsa.hpp>
#include <ndn-cpp-dev/util/io.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>




namespace
{

    class CertTool: public ndn::KeyChain
    {
        typedef SecPublicInfo::Error InfoError;
        typedef SecTpm::Error TpmError;
    public:
        CertTool()
        {
        }

        std::pair<ndn::shared_ptr<ndn::IdentityCertificate> , bool>
        getCertificate(ndn::Name certificateName)
        {
            try
            {
                ndn::shared_ptr<ndn::IdentityCertificate> cert=
                                ndn::KeyChain::getCertificate(certificateName);
                return std::make_pair(cert , true);
            }
            catch(TpmError& e)
            {
                std::cerr<<"Certificate Not Found"<<std::endl;
                return std::make_pair(
                        ndn::make_shared<ndn::IdentityCertificate>() , false);
            }
        }

        void
        deleteIdentity(const ndn::Name identityName)
        {
           ndn::KeyChain::deleteIdentity(identityName); 
        }

        
        /* Return Certificate Name */
        ndn::Name
        createIdentity(const ndn::Name identityName, const ndn::Name signee, 
                                                            bool isUnSigned=false)
        {
            ndn::KeyChain::deleteIdentity(identityName);
            ndn::KeyChain::addIdentity(identityName);
            ndn::Name keyName;
            try
            {
                keyName = ndn::KeyChain::getDefaultKeyNameForIdentity(identityName);
            }
            catch(InfoError& e)
            {
                keyName = ndn::KeyChain::generateRSAKeyPairAsDefault(identityName, true);
            }
            
            ndn::shared_ptr<ndn::PublicKey> pubKey;
            try
            {
                pubKey = ndn::KeyChain::getPublicKey(keyName);
            }
            catch(InfoError& e)
            {
                return identityName;
            }
            ndn::Name certName;
            try
            {
                certName = ndn::KeyChain::getDefaultCertificateNameForKey(keyName);
            }
            catch(InfoError& e)
            {
                ndn::shared_ptr<ndn::IdentityCertificate> certificate =
                    ndn::make_shared<ndn::IdentityCertificate>();
                ndn::Name certificateName = keyName.getPrefix(-1);
                certificateName.append("KEY").append(
                    keyName.get(-1)).append("ID-CERT").appendVersion();
                certificate->setName(certificateName);
                certificate->setNotBefore(ndn::time::system_clock::now());
                certificate->setNotAfter(ndn::time::system_clock::now() + ndn::time::days(730) /* 2 years*/);
                certificate->setPublicKeyInfo(*pubKey);
                certificate->addSubjectDescription(
                    ndn::CertificateSubjectDescription("2.5.4.41",
                                                       keyName.toUri()));
                certificate->encode();
                if ( !isUnSigned )
                {
                    try
                    {
                        signByIdentity(*certificate,signee);
                    }
                    catch(InfoError& e)
                    {
                        try
                        {
                            ndn::KeyChain::deleteIdentity(identityName);
                        }
                        catch(InfoError& e)
                        {
                        }
                        return identityName;
                    }
                    ndn::KeyChain::addCertificateAsIdentityDefault(*(certificate));
                }
                
                certName=certificate->getName();
            }
            return certName;
        }
        
        template<typename T>
        void 
        signByIdentity(T& packet, const ndn::Name& identityName)
        {
            ndn::KeyChain::signByIdentity(packet,identityName);
        }
        
        bool
        loadCertificate(std::string inputFile)
        {
            ndn::shared_ptr<ndn::IdentityCertificate> cert = 
                    ndn::io::load<ndn::IdentityCertificate>(
                                           inputFile.c_str(), ndn::io::BASE_64);
            
            try
            {
                ndn::KeyChain::deleteCertificate(cert->getName());
                ndn::KeyChain::addCertificateAsIdentityDefault(*(cert));
                return true;
            }
            catch(InfoError& e)
            {
                std::cout << e.what() <<std::endl;
                return false;
            }
        }

    };
}


void
createCertificateAndDump(std::string identity, std::string signee, 
                                                         std::string outputFile)
{
    ndn::Name certName;
    CertTool ct;
     
        if( boost::iequals(signee, "self") || boost::iequals(signee, "unsigned"))
        {
            certName=ct.createIdentity(ndn::Name(identity),ndn::Name(identity));
        }
        else
        {
            certName=ct.createIdentity(ndn::Name(identity),ndn::Name(signee));
        }
    
        std::pair<ndn::shared_ptr<ndn::IdentityCertificate> , bool> cert = 
                                                    ct.getCertificate(certName);
        if( cert.second )
        {
            std::cout<<*(cert.first)<<std::endl;
            std::ofstream outFile(outputFile.c_str());
            ndn::io::save(*(cert.first),outFile,ndn::io::BASE_64);
        }
        else
        {
            std::cerr<<"Certificate not created or signee not found"<<std::endl;
        }
    
}

void
signCertificateAndDump(std::string signee,
                                  std::string inputFile, std::string outputFile)
{
    ndn::shared_ptr<ndn::IdentityCertificate> cert = 
                    ndn::io::load<ndn::IdentityCertificate>(
                                           inputFile.c_str(), ndn::io::BASE_64);
    try
    {
        CertTool ct;
        ct.signByIdentity(*(cert), ndn::Name(signee));
        std::cout<<*(cert)<<std::endl;
        std::ofstream outFile(outputFile.c_str());
        ndn::io::save(*(cert),outFile,ndn::io::BASE_64);
        
    }
    catch(std::exception&  e)
    {
        std::cout << e.what() <<std::endl;
    }
    
}

void
loadCertificate(std::string inputFile)
{
    try
    {
        CertTool ct;
        if (ct.loadCertificate(inputFile) )
        {
            std::cout<<"Certificate Loaded in Key Chain"<<std::endl;
        }
    }
    catch(std::exception&  e)
    {
        std::cout << e.what() <<std::endl;
    }
}

void 
deleteIdentity(std::string identityName)
{
    ndn::Name idName(identityName);
    try
    {
        CertTool ct;
        ct.deleteIdentity(idName);
        
        std::cout<<"Identity Deleted"<<std::endl;
        
    }
    catch(std::exception&  e)
    {
        std::cout << e.what() <<std::endl;
    }
    
}


static int
usage(const std::string& program)
{
    std::cout << "Usage: " << program << " [OPTIONS...]"<<std::endl;
    std::cout << "   Cert Tool options...." << std::endl;
    std::cout << "       -(c|s|l) ,                  Create or Sign or Load identity's certificate" << std::endl;
    std::cout << "       -i ,     --identityName     New/singing identity name" <<std::endl;
    std::cout << "       -I ,     --signeeIdentity   Identity Name of signer or self for self signing or unsigned" <<std::endl;
    std::cout << "       -f ,     --inputFile        Input file name for -s option"<<std::endl;
    std::cout << "       -o ,     --outputFile       Output file name where certificate will be dumped"<<std::endl; 
    std::cout << "       -h ,                        Display this help message" << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    bool isCreate=false;
    bool isSign=false;
    bool isLoad=false;
    bool isDelete=false;
    int operationCount=0;
    std::string programName(argv[0]);
    std::string inputFile;
    std::string outputFile;
    std::string identityName;
    std::string signeeIdentityName;
    int opt;
    while ((opt = getopt(argc, argv, "dcslf:I:i:f:o:h")) != -1)
    {
        switch (opt)
        {
            case 'c':
                isCreate=true;
                operationCount++;
                break;
            case 's':
                isSign=true;
                operationCount++;
                break;
            case 'l':
                isLoad=true;
                operationCount++;
                break;
            case 'd':
                isDelete=true;
                operationCount++;
                break;
            case 'f':
                inputFile=optarg;
                break;
            case 'o':
                outputFile=optarg;
                break;
            case 'i':
                identityName=optarg;
            case 'I':
                signeeIdentityName=optarg;
                break;
            case 'h':
            default:
                usage(programName);
                return EXIT_FAILURE;
        }
    }
    
    if( operationCount > 1)
    {
        std::cerr<<"Can not perform more than one operation at once !"<<std::endl;
        usage(programName);
    }
    
    if ( isCreate )
    {
        if ( identityName.empty() || 
                            signeeIdentityName.empty() || outputFile.empty())
        {
           usage(programName); 
        }
        
        createCertificateAndDump(identityName,signeeIdentityName, outputFile);
    }
    
    if( isSign )
    {
        if ( signeeIdentityName.empty() || 
                                        inputFile.empty() || outputFile.empty())
        {
           usage(programName); 
        }
        
        signCertificateAndDump(signeeIdentityName, inputFile, outputFile);
    }
    
    if( isLoad )
    {
        if ( inputFile.empty() )
        {
            usage(programName);
        }
        
        loadCertificate(inputFile);
        
    }
    
    if( isDelete )
    {
        if ( identityName.empty() )
        {
            usage(programName);
        }
        deleteIdentity(identityName);
    }
    
    return EXIT_SUCCESS;
}

