#include<iostream>
#include<cstdlib>

#include <ndn-cpp-dev/security/signature-sha256-with-rsa.hpp>
#include <ndn-cpp-dev/security/identity-certificate.hpp>
#include <ndn-cpp-dev/util/io.hpp>

#include "nlsr.hpp"
#include "nlsr_dm.hpp"
#include "utility/nlsr_tokenizer.hpp"
#include "nlsr_lsdb.hpp"

namespace nlsr
{

    using namespace std;
    using namespace ndn;

    void
    DataManager::processContent(Nlsr& pnlsr, const ndn::Interest &interest,
                                const ndn::Data & data, interestManager& im)
    {
        cout << "I: " << interest.toUri() << endl;
        string dataName(data.getName().toUri());
        //cout << "D: " << dataName << endl;
        //cout << "Data Content: " << dataContent << endl;
        nlsrTokenizer nt(dataName,"/");
        //SignatureSha256WithRsa sig(data.getSignature());
        //ndn::Name keyName=sig.getKeyLocator().getName();
        //cout<<"Key Locator Name: "<<keyName.toUri()<<endl;
        string chkString("info");
        if( nt.doesTokenExist(chkString) )
        {
            string dataContent((char *)data.getContent().value());
            processContentInfo(pnlsr,dataName,dataContent);
        }
        chkString="LSA";
        if( nt.doesTokenExist(chkString) )
        {
            string dataContent((char *)data.getContent().value());
            processContentLsa(pnlsr, dataName, dataContent);
        }
        chkString="keys";
        if( nt.doesTokenExist(chkString) )
        {
            processContentKeys(pnlsr, data);
        }
    }

    void
    DataManager::processContentInfo(Nlsr& pnlsr, string& dataName,
                                    string& dataContent)
    {
        nlsrTokenizer nt(dataName,"/");
        string chkString("info");
        string neighbor=nt.getTokenString(0,nt.getTokenPosition(chkString)-1);
        int oldStatus=pnlsr.getAdl().getStatusOfNeighbor(neighbor);
        int infoIntTimedOutCount=pnlsr.getAdl().getTimedOutInterestCount(neighbor);
        //debugging purpose start
        cout <<"Before Updates: " <<endl;
        cout <<"Neighbor : "<<neighbor<<endl;
        cout<<"Status: "<< oldStatus << endl;
        cout<<"Info Interest Timed out: "<< infoIntTimedOutCount <<endl;
        //debugging purpose end
        pnlsr.getAdl().setStatusOfNeighbor(neighbor,1);
        pnlsr.getAdl().setTimedOutInterestCount(neighbor,0);
        int newStatus=pnlsr.getAdl().getStatusOfNeighbor(neighbor);
        infoIntTimedOutCount=pnlsr.getAdl().getTimedOutInterestCount(neighbor);
        //debugging purpose
        cout <<"After Updates: " <<endl;
        cout <<"Neighbor : "<<neighbor<<endl;
        cout<<"Status: "<< newStatus << endl;
        cout<<"Info Interest Timed out: "<< infoIntTimedOutCount <<endl;
        //debugging purpose end
        if ( ( oldStatus-newStatus)!= 0 ) // change in Adjacency list
        {
            pnlsr.incrementAdjBuildCount();
            /* Need to schedule event for Adjacency LSA building */
            if ( pnlsr.getIsBuildAdjLsaSheduled() == 0 )
            {
                pnlsr.setIsBuildAdjLsaSheduled(1);
                // event here
                pnlsr.getScheduler().scheduleEvent(ndn::time::seconds(5),
                                                   ndn::bind(&Lsdb::scheduledAdjLsaBuild, pnlsr.getLsdb(),
                                                           boost::ref(pnlsr)));
            }
        }
    }

    void
    DataManager::processContentLsa(Nlsr& pnlsr, string& dataName,
                                   string& dataContent)
    {
        nlsrTokenizer nt(dataName,"/");
        string chkString("LSA");
        string origRouter=nt.getTokenString(nt.getTokenPosition(chkString)+1,
                                            nt.getTokenNumber()-4);
        string lsTypeString=nt.getToken(nt.getTokenNumber()-3);
        string lsSeNoString=nt.getToken(nt.getTokenNumber()-2);
        uint32_t interestedLsSeqNo;
        try
        {
            interestedLsSeqNo=boost::lexical_cast<uint32_t>(lsSeNoString);
        }
        catch(std::exception &e)
        {
            return;
        }
        if( lsTypeString == "1" ) //Name Lsa
        {
            processContentNameLsa(pnlsr, origRouter+"/"+lsTypeString,
                                  interestedLsSeqNo, dataContent);
        }
        else if( lsTypeString == "2" ) //Adj Lsa
        {
            processContentAdjLsa(pnlsr, origRouter+"/"+lsTypeString,
                                 interestedLsSeqNo, dataContent);
        }
        else if( lsTypeString == "3" ) //Cor Lsa
        {
            processContentCorLsa(pnlsr, origRouter+"/"+lsTypeString,
                                 interestedLsSeqNo, dataContent);
        }
        else
        {
            cout<<"Unrecognized LSA Type :("<<endl;
        }
    }

    void
    DataManager::processContentNameLsa(Nlsr& pnlsr, string lsaKey,
                                       uint32_t lsSeqNo, string& dataContent)
    {
        if ( pnlsr.getLsdb().isNameLsaNew(lsaKey,lsSeqNo))
        {
            NameLsa nameLsa;
            if( nameLsa.initNameLsaFromContent(dataContent) )
            {
                pnlsr.getLsdb().installNameLsa(pnlsr, nameLsa);
            }
            else
            {
                cout<<"LSA data decoding error :("<<endl;
            }
        }
    }

    void
    DataManager::processContentAdjLsa(Nlsr& pnlsr, string lsaKey,
                                      uint32_t lsSeqNo, string& dataContent)
    {
        if ( pnlsr.getLsdb().isAdjLsaNew(lsaKey,lsSeqNo))
        {
            AdjLsa adjLsa;
            if( adjLsa.initAdjLsaFromContent(dataContent) )
            {
                pnlsr.getLsdb().installAdjLsa(pnlsr, adjLsa);
            }
            else
            {
                cout<<"LSA data decoding error :("<<endl;
            }
        }
    }

    void
    DataManager::processContentCorLsa(Nlsr& pnlsr, string lsaKey,
                                      uint32_t lsSeqNo, string& dataContent)
    {
        if ( pnlsr.getLsdb().isCorLsaNew(lsaKey,lsSeqNo))
        {
            CorLsa corLsa;
            if( corLsa.initCorLsaFromContent(dataContent) )
            {
                pnlsr.getLsdb().installCorLsa(pnlsr, corLsa);
            }
            else
            {
                cout<<"LSA data decoding error :("<<endl;
            }
        }
    }
    
    void 
    DataManager::processContentKeys(Nlsr& pnlsr, const ndn::Data& data)
    {
        std::ofstream outFile("data_received");
        ndn::io::save(data,outFile,ndn::io::NO_ENCODING);
        cout<<" processContentKeys called "<<endl;
        SignatureSha256WithRsa signature(data.getSignature());
        cout<<"D: <<"<<data<<endl;
        cout<<"Key Locator: "<<signature.getKeyLocator().getName().toUri()<<endl;
        ndn::shared_ptr<ndn::IdentityCertificate> cert=ndn::make_shared<ndn::IdentityCertificate>();
        cert->wireDecode(data.getContent().blockFromValue());
        cout<<*(cert)<<endl;
        
        if( pnlsr.getKeyManager().verifySignature(*(cert),cert->getPublicKeyInfo()))
        {
            cout<<"Verified Data"<<endl;
        }
    }
}//namespace nlsr
