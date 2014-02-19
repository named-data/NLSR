#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <algorithm>

#include "nlsr_tokenizer.hpp"

namespace nlsr {

using namespace std;
using namespace boost;

void 
nlsrTokenizer::makeToken(){
	char_separator<char> sep(seps.c_str());
	tokenizer< char_separator<char> >tokens(originalString, sep);
	tokenizer< char_separator<char> >::iterator tok_iter = tokens.begin();
	
	string ft(*tok_iter);
	firstToken=ft;
	++tok_iter;

	for ( ;tok_iter != tokens.end(); ++tok_iter){
		string oneToken(*tok_iter);
		this->insertToken(oneToken);
		restOfTheLine+=oneToken;
		restOfTheLine+=seps;
  	}

	trim(restOfTheLine);
}

void 
nlsrTokenizer::insertToken(const string& token){
	tokenList.push_back(token);
}

int 
nlsrTokenizer::getTokenPosition(string& token){
	int pos=-1;
	int i=1;

	for(std::list<string>::iterator it=tokenList.begin();it!=tokenList.end();it++){
		if( (*it) == token ){
			break;
		}
		i++;
	}

	if( i < tokenList.size() ){
		pos=i;
	}

	return pos;
}

string 
nlsrTokenizer::getTokenString(int from , int to){
	string returnString;
	if ( from >=0 && to < tokenList.size()){
		int i=0;
		for(std::list<string>::iterator it=tokenList.begin();
													it!=tokenList.end();it++){
			i++;
			if( i >= from && i<= to ){
				string oneToken((*it));
				returnString+=seps;
				returnString+=oneToken;
				
			}
			
		}
	}

	trim(returnString);
	return returnString;
}

string 
nlsrTokenizer::getTokenString(int from){
	string returnString;
	if ( from >=0 && from < tokenList.size()){
		int i=0;
		for(std::list<string>::iterator it=tokenList.begin();
													it!=tokenList.end();it++){
			i++;
			if( i >= from){
				string oneToken((*it));
				returnString+=seps;
				returnString+=oneToken;
				
			}
			
		}
	}

	trim(returnString);
	return returnString;
}

static bool
tokenCompare(string& s1, string& s2){
	return s1==s2;
}

bool
nlsrTokenizer::doesTokenExist(string token){
	std::list<string >::iterator it = std::find_if( tokenList.begin(), 
								tokenList.end(),	
   								bind(&tokenCompare, _1 , token));

	if( it != tokenList.end() ){
		return true;
	}

	return false;
}

}//namespace nlsr
