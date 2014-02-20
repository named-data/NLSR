#include<iostream>
#include<cstdlib>

#include "nlsr.hpp"
#include "nlsr_dm.hpp"
#include "nlsr_tokenizer.hpp"
#include "nlsr_lsdb.hpp"

namespace nlsr
{

    using namespace std;
    using namespace ndn;

    void
    DataManager::processContent(Nlsr& pnlsr, const ndn::Interest &interest,
                                const ndn::Data & data)
    {
        cout << "I: " << interest.toUri() << endl;
        string dataName(data.getName().toUri());
        string dataContent((char *)data.getContent().value());
        cout << "D: " << dataName << endl;
        cout << "Data Content: " << dataContent << endl;
        nlsrTokenizer nt(dataName,"/");
        string chkString("info");
        if( nt.doesTokenExist(chkString) )
        {
            processContentInfo(pnlsr,dataName,dataContent);
        }
    }

    void
    DataManager::processContentInfo(Nlsr& pnlsr, string& dataName,
                                    string& dataContent)
    {
        nlsrTokenizer nt(dataName,"/");
        string chkString("info");
        string neighbor="/" + nt.getFirstToken()
                        +nt.getTokenString(0,nt.getTokenPosition(chkString)-1);
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

}//namespace nlsr
