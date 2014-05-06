#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "nlsr.hpp"
#include "lsa.hpp"
#include "name-prefix-list.hpp"
#include "adjacent.hpp"
#include "utility/tokenizer.hpp"


namespace nlsr {

using namespace std;


const ndn::Name
NameLsa::getKey() const
{
  ndn::Name key = m_origRouter;
  key.append("name");
  return key;
}

NameLsa::NameLsa(const ndn::Name& origR, const string& lst, uint32_t lsn,
                 uint32_t lt,
                 NamePrefixList& npl)
{
  m_origRouter = origR;
  m_lsType = lst;
  m_lsSeqNo = lsn;
  m_lifeTime = lt;
  std::list<ndn::Name>& nl = npl.getNameList();
  for (std::list<ndn::Name>::iterator it = nl.begin(); it != nl.end(); it++)
  {
    addName((*it));
  }
}

string
NameLsa::getData()
{
  string nameLsaData;
  nameLsaData = m_origRouter.toUri() + "|" + "name" + "|"
                + boost::lexical_cast<std::string>(m_lsSeqNo) + "|"
                + boost::lexical_cast<std::string>(m_lifeTime);
  nameLsaData += "|";
  nameLsaData += boost::lexical_cast<std::string>(m_npl.getSize());
  std::list<ndn::Name> nl = m_npl.getNameList();
  for (std::list<ndn::Name>::iterator it = nl.begin(); it != nl.end(); it++)
  {
    nameLsaData += "|";
    nameLsaData += (*it).toUri();
  }
  return nameLsaData + "|";
}

bool
NameLsa::initializeFromContent(const std::string& content)
{
  uint32_t numName = 0;
  boost::char_separator<char> sep("|");
  boost::tokenizer<boost::char_separator<char> >tokens(content, sep);
  boost::tokenizer<boost::char_separator<char> >::iterator tok_iter =
                                               tokens.begin();
  m_origRouter = ndn::Name(*tok_iter++);
  if (!(m_origRouter.size() > 0))
  {
    return false;
  }
  try {
    m_lsType = *tok_iter++;
    m_lsSeqNo = boost::lexical_cast<uint32_t>(*tok_iter++);
    m_lifeTime = boost::lexical_cast<uint32_t>(*tok_iter++);
    numName = boost::lexical_cast<uint32_t>(*tok_iter++);
  }
  catch (std::exception& e) {
    return false;
  }
  for (uint32_t i = 0; i < numName; i++)
  {
    string name(*tok_iter++);
    addName(name);
  }
  return true;
}

void
NameLsa::writeLog()
{
}

std::ostream&
operator<<(std::ostream& os, NameLsa& nLsa)
{
  os << "Name Lsa: " << endl;
  os << "  Origination Router: " << nLsa.getOrigRouter() << endl;
  os << "  Ls Type: " << nLsa.getLsType() << endl;
  os << "  Ls Seq No: " << (unsigned int)nLsa.getLsSeqNo() << endl;
  os << "  Ls Lifetime: " << (unsigned int)nLsa.getLifeTime() << endl;
  os << "  Names: " << endl;
  int i = 1;
  std::list<ndn::Name> nl = nLsa.getNpl().getNameList();
  for (std::list<ndn::Name>::iterator it = nl.begin(); it != nl.end(); it++)
  {
    os << "    Name " << i << ": " << (*it) << endl;
  }
  return os;
}



CoordinateLsa::CoordinateLsa(const ndn::Name& origR, const string lst,
                             uint32_t lsn,
                             uint32_t lt
                             , double r, double theta)
{
  m_origRouter = origR;
  m_lsType = lst;
  m_lsSeqNo = lsn;
  m_lifeTime = lt;
  m_corRad = r;
  m_corTheta = theta;
}

const ndn::Name
CoordinateLsa::getKey() const
{
  ndn::Name key = m_origRouter;
  key.append("coordinate");
  return key;
}

bool
CoordinateLsa::isEqualContent(const CoordinateLsa& clsa)
{
  return (std::abs(m_corRad - clsa.getCorRadius()) <
          std::numeric_limits<double>::epsilon()) &&
         (std::abs(m_corTheta - clsa.getCorTheta()) <
          std::numeric_limits<double>::epsilon());
}

string
CoordinateLsa::getData()
{
  string corLsaData;
  corLsaData = m_origRouter.toUri() + "|";
  corLsaData += "coordinate";
  corLsaData += "|";
  corLsaData += (boost::lexical_cast<std::string>(m_lsSeqNo) + "|");
  corLsaData += (boost::lexical_cast<std::string>(m_lifeTime) + "|");
  corLsaData += (boost::lexical_cast<std::string>(m_corRad) + "|");
  corLsaData += (boost::lexical_cast<std::string>(m_corTheta) + "|");
  return corLsaData;
}

bool
CoordinateLsa::initializeFromContent(const std::string& content)
{
  boost::char_separator<char> sep("|");
  boost::tokenizer<boost::char_separator<char> >tokens(content, sep);
  boost::tokenizer<boost::char_separator<char> >::iterator tok_iter =
                                               tokens.begin();
  m_origRouter = ndn::Name(*tok_iter++);
  if (!(m_origRouter.size() > 0))
  {
    return false;
  }
  try {
    m_lsType   = *tok_iter++;
    m_lsSeqNo  = boost::lexical_cast<uint32_t>(*tok_iter++);
    m_lifeTime = boost::lexical_cast<uint32_t>(*tok_iter++);
    m_corRad   = boost::lexical_cast<double>(*tok_iter++);
    m_corTheta = boost::lexical_cast<double>(*tok_iter++);
  }
  catch (std::exception& e) {
    return false;
  }
  return true;
}

std::ostream&
operator<<(std::ostream& os, const CoordinateLsa& cLsa)
{
  os << "Cor Lsa: " << endl;
  os << "  Origination Router: " << cLsa.getOrigRouter() << endl;
  os << "  Ls Type: " << cLsa.getLsType() << endl;
  os << "  Ls Seq No: " << (unsigned int)cLsa.getLsSeqNo() << endl;
  os << "  Ls Lifetime: " << (unsigned int)cLsa.getLifeTime() << endl;
  os << "    Hyperbolic Radius: " << cLsa.getCorRadius() << endl;
  os << "    Hyperbolic Theta: " << cLsa.getCorTheta() << endl;
  return os;
}


AdjLsa::AdjLsa(const ndn::Name& origR, const string& lst, uint32_t lsn,
               uint32_t lt,
               uint32_t nl , AdjacencyList& adl)
{
  m_origRouter = origR;
  m_lsType = lst;
  m_lsSeqNo = lsn;
  m_lifeTime = lt;
  m_noLink = nl;
  std::list<Adjacent> al = adl.getAdjList();
  for (std::list<Adjacent>::iterator it = al.begin(); it != al.end(); it++)
  {
    if ((*it).getStatus() == 1)
    {
      addAdjacent((*it));
    }
  }
}

const ndn::Name
AdjLsa::getKey() const
{
  ndn::Name key = m_origRouter;
  key.append("adjacency");
  return key;
}

bool
AdjLsa::isEqualContent(AdjLsa& alsa)
{
  return m_adl == alsa.getAdl();
}


string
AdjLsa::getData()
{
  string adjLsaData;
  adjLsaData = m_origRouter.toUri() + "|" + "adjacency" + "|"
               + boost::lexical_cast<std::string>(m_lsSeqNo) + "|"
               + boost::lexical_cast<std::string>(m_lifeTime);
  adjLsaData += "|";
  adjLsaData += boost::lexical_cast<std::string>(m_adl.getSize());
  std::list<Adjacent> al = m_adl.getAdjList();
  for (std::list<Adjacent>::iterator it = al.begin(); it != al.end(); it++)
  {
    adjLsaData += "|";
    adjLsaData += (*it).getName().toUri();
    adjLsaData += "|";
    adjLsaData += boost::lexical_cast<std::string>((*it).getConnectingFace());
    adjLsaData += "|";
    adjLsaData += boost::lexical_cast<std::string>((*it).getLinkCost());
  }
  return adjLsaData + "|";
}

bool
AdjLsa::initializeFromContent(const std::string& content)
{
  uint32_t numLink = 0;
  boost::char_separator<char> sep("|");
  boost::tokenizer<boost::char_separator<char> >tokens(content, sep);
  boost::tokenizer<boost::char_separator<char> >::iterator tok_iter =
                                               tokens.begin();
  m_origRouter = ndn::Name(*tok_iter++);
  if (!(m_origRouter.size() > 0))
  {
    return false;
  }
  try {
    m_lsType   = *tok_iter++;
    m_lsSeqNo  = boost::lexical_cast<uint32_t>(*tok_iter++);
    m_lifeTime = boost::lexical_cast<uint32_t>(*tok_iter++);
    numLink    = boost::lexical_cast<uint32_t>(*tok_iter++);
  }
  catch (std::exception& e) {
    return false;
  }
  for (uint32_t i = 0; i < numLink; i++)
  {
    try {
      string adjName(*tok_iter++);
      int connectingFace = boost::lexical_cast<int>(*tok_iter++);
      double linkCost = boost::lexical_cast<double>(*tok_iter++);
      Adjacent adjacent(adjName, connectingFace, linkCost, 0, 0);
      addAdjacent(adjacent);
    }
    catch (std::exception& e) {
      return false;
    }
  }
  return true;
}


void
AdjLsa::addNptEntries(Nlsr& pnlsr)
{
  if (getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
  {
    pnlsr.getNamePrefixTable().addEntry(getOrigRouter(), getOrigRouter());
  }
}


void
AdjLsa::removeNptEntries(Nlsr& pnlsr)
{
  if (getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix())
  {
    pnlsr.getNamePrefixTable().removeEntry(getOrigRouter(), getOrigRouter());
  }
}



std::ostream&
operator<<(std::ostream& os, AdjLsa& aLsa)
{
  os << "Adj Lsa: " << endl;
  os << "  Origination Router: " << aLsa.getOrigRouter() << endl;
  os << "  Ls Type: " << aLsa.getLsType() << endl;
  os << "  Ls Seq No: " << (unsigned int)aLsa.getLsSeqNo() << endl;
  os << "  Ls Lifetime: " << (unsigned int)aLsa.getLifeTime() << endl;
  os << "  No Link: " << (unsigned int)aLsa.getNoLink() << endl;
  os << "  Adjacents: " << endl;
  int i = 1;
  std::list<Adjacent> al = aLsa.getAdl().getAdjList();
  for (std::list<Adjacent>::iterator it = al.begin(); it != al.end(); it++)
  {
    os << "    Adjacent " << i << ": " << endl;
    os << "      Adjacent Name: " << (*it).getName() << endl;
    os << "      Connecting Face: " << (*it).getConnectingFace() << endl;
    os << "      Link Cost: " << (*it).getLinkCost() << endl;
  }
  return os;
}

}//namespace nlsr
