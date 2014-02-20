#ifndef NLSR_IM_HPP
#define NLSR_IM_HPP

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include <ndn-cpp-dev/util/scheduler.hpp>

namespace nlsr
{

    using namespace ndn;
    using namespace std;

    class Nlsr;

    class interestManager
    {
    public:
        interestManager()
        {
        }
        void processInterest(Nlsr& pnlsr, const ndn::Name &name,
                             const ndn::Interest &interest);
        void processInterestInfo(Nlsr& pnlsr, string& neighbor,
                                 const ndn::Interest &interest);
        void processInterestTimedOut(Nlsr& pnlsr, const ndn::Interest &interest);
        void processInterestTimedOutInfo(Nlsr& pnlsr, string& neighbor,
                                         const ndn::Interest &interest);
        void expressInterest(Nlsr& pnlsr,const string& interestNamePrefix, int scope,
                             int seconds);
        void sendScheduledInfoInterest(Nlsr& pnlsr, int seconds);
        void scheduleInfoInterest(Nlsr& pnlsr, int seconds);

    private:


    };

}//namespace nlsr

#endif
