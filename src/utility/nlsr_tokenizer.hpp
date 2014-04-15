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
      : m_firstToken()
      , m_restOfTheLine()
      , m_currentPosition(0)
    {
      m_seps = " ";
      m_originalString = inputString;
      makeToken();
    }

    nlsrTokenizer(const string& inputString, const string& separator)
      : m_firstToken()
      , m_restOfTheLine()
      , m_currentPosition(0)
    {
      m_seps = separator;
      m_originalString = inputString;
      makeToken();
    }

    string getFirstToken()
    {
      return m_firstToken;
    }

    string getRestOfLine()
    {
      return m_restOfTheLine;
    }

    void resetCurrentPosition(uint32_t cp=0)
    {
      if( cp >=0 && cp <= m_vTokenList.size() )
      {
        m_currentPosition=cp;
      }
    }

    string getNextToken()
    {
      if(m_currentPosition >= 0 && m_currentPosition <= (m_vTokenList.size()-1))
      {
        return m_vTokenList[m_currentPosition++];
      }
      return "";
    }

    uint32_t getTokenNumber()
    {
      return m_tokenList.size();
    }

    string getToken(int position)
    {
      if( position >=0 && position <m_vTokenList.size() )
      {
        return m_vTokenList[position];
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

    string m_seps;
    string m_originalString;
    string m_firstToken;
    string m_restOfTheLine;
    std::list<string> m_tokenList;
    std::vector<string> m_vTokenList;
    uint32_t m_currentPosition;
  };

}//namespace nlsr
#endif
