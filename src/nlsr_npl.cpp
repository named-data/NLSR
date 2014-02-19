#include<iostream>
#include<algorithm>

#include "nlsr_npl.hpp"

namespace nlsr {

using namespace std;

Npl::Npl(){

}

Npl::~Npl(){

}

static bool
nameCompare(string& s1, string& s2){
	return s1==s2;
}

int
Npl::insertIntoNpl(string& name){
	std::list<string >::iterator it = std::find_if( nameList.begin(), 
								nameList.end(),	
   								bind(&nameCompare, _1 , name));

	if( it != nameList.end() ){
		return -1;
	}

	nameList.push_back(name);
	return 0;

}

int 
Npl::removeFromNpl(string& name)
{
	std::list<string >::iterator it = std::find_if( nameList.begin(), 
								nameList.end(),	
   								bind(&nameCompare, _1 , name));

	if( it != nameList.end() ){
		nameList.erase(it);
	}

	return -1;
}

void
Npl::sortNpl()
{
	nameList.sort();
}

void
Npl::printNpl(){
	int i=1;
	for( std::list<string>::iterator it=nameList.begin(); it != nameList.end(); it++){
		cout<<"Name "<<i<<" : "<<(*it)<<endl;
		i++;
	}
}

}//namespace nlsr
