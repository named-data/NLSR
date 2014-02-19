#ifndef NLSR_TOKENIZER_HPP
#define NLSR_TOKENIZER_HPP

#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <list>
#include <ndn-cpp-dev/face.hpp>

namespace nlsr {

using namespace std;
using namespace boost;

class nlsrTokenizer{
	public:
		nlsrTokenizer(const string& inputString)
			:firstToken(),
			 restOfTheLine()	
		{
			seps = " ";
			originalString = inputString;
			makeToken();
		}

		nlsrTokenizer(const string& inputString, const string& separator)
			:firstToken(),
			 restOfTheLine()	
		{
			seps = separator;
			originalString = inputString;
			makeToken();
		}
		
		string getFirstToken(){
			return firstToken;
		}

		string getRestOfLine(){
			return restOfTheLine;
		}

		int getTokenPosition(string& token);
		string getTokenString(int from , int to);
		string getTokenString(int from);
		bool doesTokenExist(string token);

	private:

		void makeToken();
		void insertToken(const string& token);
	
		string seps;
		string originalString;
		string firstToken;
		string restOfTheLine;
		std::list<string> tokenList;
};

}//namespace nlsr
#endif
