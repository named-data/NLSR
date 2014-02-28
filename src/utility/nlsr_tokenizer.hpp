#ifndef NLSR_TOKENIZER_HPP
#define NLSR_TOKENIZER_HPP

#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <list>
#include <vector>
#include <ndn-cpp-dev/face.hpp>

namespace nlsr
{

    using namespace std;
    using namespace boost;

    class nlsrTokenizer
    {
    public:
        nlsrTokenizer(const string& inputString)
            : firstToken()
            , restOfTheLine()
            , currentPosition(0)
        {
            seps = " ";
            originalString = inputString;
            makeToken();
        }

        nlsrTokenizer(const string& inputString, const string& separator)
            : firstToken()
            , restOfTheLine()
            , currentPosition(0)
        {
            seps = separator;
            originalString = inputString;
            makeToken();
        }

        string getFirstToken()
        {
            return firstToken;
        }

        string getRestOfLine()
        {
            return restOfTheLine;
        }

        void resetCurrentPosition(uint32_t cp=0)
        {
            if( cp >=0 && cp <= vTokenList.size() )
            {
                currentPosition=cp;
            }
        }

        string getNextToken()
        {
            if(currentPosition >= 0 && currentPosition <= (vTokenList.size()-1))
            {
                return vTokenList[currentPosition++];
            }
            return "";
        }

        uint32_t getTokenNumber()
        {
            return tokenList.size();
        }

        string getToken(int position)
        {
            if( position >=0 && position <vTokenList.size() )
            {
                return vTokenList[position];
            }
            return "";
        }

        int getTokenPosition(string& token);
        string getTokenString(int from , int to);
        string getTokenString(int from);
        bool doesTokenExist(string token);


    private:

        void makeToken();
        void insertToken(const string& token);
        void makeRestOfTheLine();

        string seps;
        string originalString;
        string firstToken;
        string restOfTheLine;
        std::list<string> tokenList;
        std::vector<string> vTokenList;
        uint32_t currentPosition;
    };

}//namespace nlsr
#endif
