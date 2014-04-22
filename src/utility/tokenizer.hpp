#ifndef NLSR_TOKENIZER_HPP
#define NLSR_TOKENIZER_HPP

#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <list>
#include <vector>
#include <ndn-cpp-dev/face.hpp>

namespace nlsr {

class Tokenizer
{
public:
  Tokenizer(const std::string& inputString)
    : m_firstToken()
    , m_restOfTheLine()
    , m_currentPosition(0)
  {
    m_seps = " ";
    m_originalString = inputString;
    makeToken();
  }

  Tokenizer(const std::string& inputString, const std::string& separator)
    : m_firstToken()
    , m_restOfTheLine()
    , m_currentPosition(0)
  {
    m_seps = separator;
    m_originalString = inputString;
    makeToken();
  }

  std::string
  getFirstToken()
  {
    return m_firstToken;
  }

  std::string
  getRestOfLine()
  {
    return m_restOfTheLine;
  }

  void
  resetCurrentPosition(uint32_t cp = 0)
  {
    if (cp <= m_vTokenList.size())
    {
      m_currentPosition = cp;
    }
  }

  std::string
  getNextToken()
  {
    if (m_currentPosition <= (m_vTokenList.size() - 1))
    {
      return m_vTokenList[m_currentPosition++];
    }
    return "";
  }

  uint32_t
  getTokenNumber()
  {
    return m_tokenList.size();
  }

  std::string
  getToken(unsigned int position)
  {
    if (position < m_vTokenList.size())
    {
      return m_vTokenList[position];
    }
    return "";
  }

  uint32_t
  getTokenPosition(std::string& token);

  std::string
  getTokenString(uint32_t from , uint32_t to);

  std::string
  getTokenString(uint32_t from);

  bool
  doesTokenExist(std::string token);


private:

  void
  makeToken();

  void
  insertToken(const std::string& token);

  void
  makeRestOfTheLine();

  std::string m_seps;
  std::string m_originalString;
  std::string m_firstToken;
  std::string m_restOfTheLine;
  std::list<std::string> m_tokenList;
  std::vector<std::string> m_vTokenList;
  uint32_t m_currentPosition;
};

}//namespace nlsr
#endif //NLSR_TOKENIZER_HPP
