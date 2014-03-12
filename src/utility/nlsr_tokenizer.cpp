#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <algorithm>

#include "nlsr_tokenizer.hpp"

namespace nlsr
{

  using namespace std;
  using namespace boost;

  void
  nlsrTokenizer::makeToken()
  {
    char_separator<char> sep(seps.c_str());
    tokenizer< char_separator<char> >tokens(originalString, sep);
    tokenizer< char_separator<char> >::iterator tok_iter = tokens.begin();
    for ( ; tok_iter != tokens.end(); ++tok_iter)
    {
      string oneToken(*tok_iter);
      trim(oneToken);
      if(!oneToken.empty())
      {
        insertToken(oneToken);
      }
    }
    firstToken=vTokenList[0];
    makeRestOfTheLine();
  }

  void
  nlsrTokenizer::insertToken(const string& token)
  {
    tokenList.push_back(token);
    vTokenList.push_back(token);
  }

  int
  nlsrTokenizer::getTokenPosition(string& token)
  {
    int pos=-1;
    int i=0;
    for(std::list<string>::iterator it=tokenList.begin();
        it!=tokenList.end(); it++)
    {
      if( (*it) == token )
      {
        break;
      }
      i++;
    }
    if( i < tokenList.size() )
    {
      pos=i;
    }
    return pos;
  }

  string
  nlsrTokenizer::getTokenString(int from , int to)
  {
    string returnString="";
    if((from>=0 && to<tokenList.size()) &&
        (to>=from && to <tokenList.size()))
    {
      for(int i=from; i<=to; i++)
      {
        returnString+=seps;
        returnString+=vTokenList[i];
      }
    }
    trim(returnString);
    return returnString;
  }

  string
  nlsrTokenizer::getTokenString(int from)
  {
    return getTokenString(from,tokenList.size()-1);
  }

  static bool
  tokenCompare(string& s1, string& s2)
  {
    return s1==s2;
  }

  void
  nlsrTokenizer::makeRestOfTheLine()
  {
    restOfTheLine=getTokenString(1);
  }

  bool
  nlsrTokenizer::doesTokenExist(string token)
  {
    std::list<string >::iterator it = std::find_if( tokenList.begin(),
                                      tokenList.end(),
                                      bind(&tokenCompare, _1 , token));
    if( it != tokenList.end() )
    {
      return true;
    }
    return false;
  }

}//namespace nlsr
