#ifndef NLSR_TEST_HPP
#define NLSR_TEST_HPP

#include <iostream>
#include <string>

#include "nlsr_lsdb.hpp"
#include "nlsr_lsa.hpp"
#include "nlsr_adl.hpp"
#include "nlsr_npl.hpp"
#include "nlsr_adjacent.hpp"

namespace nlsr
{

    using namespace std;

    class Nlsr;

    class NlsrTest
    {
    public:
        NlsrTest()
        {
        }
        void schedlueAddingLsas(Nlsr& pnlsr);
    private:
        void secheduledAddNameLsa(Nlsr& pnlsr, string router,
                                  string name1, string name2, string name3);
        void secheduledAddCorLsa(Nlsr& pnlsr,string router, double r, double angle);

        void scheduledAddAdjacentLsa(Nlsr& pnlsr, string router,
                                     Adjacent adj1, Adjacent adj2);

    };

} //namespace nlsr
#endif
