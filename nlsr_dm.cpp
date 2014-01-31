#include<iostream>
#include<cstdlib>



#include "nlsr.hpp"
#include "nlsr_dm.hpp"
#include "nlsr_tokenizer.hpp"

using namespace std;
using namespace ndn;

void
DataManager::processContent(const nlsr& pnlsr, 
                  const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest,
								               const ndn::ptr_lib::shared_ptr<ndn::Data> &data)
{

	cout << "I: " << interest->toUri() << endl;
  	cout << "D: " << data->getName().toUri() << endl;
	cout << "Data Content: " << data->getContent() << endl;

}
