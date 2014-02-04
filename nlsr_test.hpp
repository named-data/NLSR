#ifndef NLSR_TEST_HPP
#define NLSR_TEST_HPP

#include <iostream>
#include <string>

#include "nlsr_lsdb.hpp"
#include "nlsr_lsa.hpp"
#include "nlsr_adl.hpp"
#include "nlsr_npl.hpp"
#include "nlsr_adjacent.hpp"

using namespace std;

class nlsr;

class nlsrTest
{
public:
	nlsrTest()
	{
	}
	void schedlueAddingLsas(nlsr& pnlsr);
private:
	void secheduledAddNameLsa(nlsr& pnlsr, string router,
																		 string name1, string name2, string name3);
	void secheduledAddCorLsa(nlsr& pnlsr,string router, double r, double angle);

	void scheduledAddAdjacentLsa(nlsr& pnlsr, string router, 
	                                                Adjacent adj1, Adjacent adj2);

};

#endif
