#ifndef NLSR_FE_HPP
#define NLSR_FE_HPP

#include<list>
#include <iostream>
#include <ndn-cpp-dev/util/scheduler.hpp>

#include "nlsr_nexthop.hpp"
#include "nlsr_nhl.hpp"

namespace nlsr
{

    using namespace std;

    class FibEntry
    {
    public:
        FibEntry()
            : name()
            , timeToRefresh(0)
            , feSeqNo(0)
        {
        }

        FibEntry(string n)
        {
            name=n;
        }

        string getName()
        {
            return name;
        }

        Nhl& getNhl()
        {
            return nhl;
        }

        int getTimeToRefresh()
        {
            return timeToRefresh;
        }

        void setTimeToRefresh(int ttr)
        {
            timeToRefresh=ttr;
        }

        void setFeExpiringEventId(ndn::EventId feid)
        {
            feExpiringEventId=feid;
        }

        ndn::EventId getFeExpiringEventId()
        {
            return feExpiringEventId;
        }

        void setFeSeqNo(int fsn)
        {
            feSeqNo=fsn;
        }

        int getFeSeqNo()
        {
            return feSeqNo;
        }

        bool isEqualNextHops(Nhl &nhlOther);

    private:
        string name;
        int timeToRefresh;
        ndn::EventId feExpiringEventId;
        int feSeqNo;
        Nhl nhl;
    };

    ostream& operator<<(ostream& os, FibEntry& fe);

} //namespace nlsr

#endif
