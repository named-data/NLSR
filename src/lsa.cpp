/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
 *
 * This file is part of NLSR (Named-data Link State Routing).
 * See AUTHORS.md for complete list of NLSR authors and contributors.
 *
 * NLSR is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

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
#include "logger.hpp"


namespace nlsr {

INIT_LOGGER("Lsa");

using namespace std;

const std::string NameLsa::TYPE_STRING = "name";
const std::string AdjLsa::TYPE_STRING = "adjacency";
const std::string CoordinateLsa::TYPE_STRING = "coordinate";

const ndn::Name
NameLsa::getKey() const
{
  ndn::Name key = m_origRouter;
  key.append(NameLsa::TYPE_STRING);
  return key;
}

NameLsa::NameLsa(const ndn::Name& origR, const string& lst, uint32_t lsn,
                 const ndn::time::system_clock::TimePoint& lt,
                 NamePrefixList& npl)
{
  m_origRouter = origR;
  m_lsType = lst;
  m_lsSeqNo = lsn;
  m_expirationTimePoint = lt;
  std::list<ndn::Name>& nl = npl.getNameList();
  for (std::list<ndn::Name>::iterator it = nl.begin(); it != nl.end(); it++) {
    addName((*it));
  }
}

string
NameLsa::getData()
{
  string nameLsaData;
  nameLsaData = m_origRouter.toUri() + "|" + NameLsa::TYPE_STRING + "|"
                + boost::lexical_cast<std::string>(m_lsSeqNo) + "|"
                + ndn::time::toIsoString(m_expirationTimePoint);
  nameLsaData += "|";
  nameLsaData += boost::lexical_cast<std::string>(m_npl.getSize());
  std::list<ndn::Name> nl = m_npl.getNameList();
  for (std::list<ndn::Name>::iterator it = nl.begin(); it != nl.end(); it++) {
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
  if (!(m_origRouter.size() > 0)) {
    return false;
  }
  try {
    m_lsType = *tok_iter++;
    m_lsSeqNo = boost::lexical_cast<uint32_t>(*tok_iter++);
    m_expirationTimePoint = ndn::time::fromIsoString(*tok_iter++);
    numName = boost::lexical_cast<uint32_t>(*tok_iter++);
  }
  catch (std::exception& e) {
    return false;
  }
  for (uint32_t i = 0; i < numName; i++) {
    ndn::Name name(*tok_iter++);
    addName(name);
  }
  return true;
}

void
NameLsa::writeLog()
{
  _LOG_DEBUG("Name Lsa: ");
  _LOG_DEBUG("  Origination Router: " << m_origRouter);
  _LOG_DEBUG("  Ls Type: " << m_lsType);
  _LOG_DEBUG("  Ls Seq No: " << m_lsSeqNo);
  _LOG_DEBUG("  Ls Lifetime: " << m_expirationTimePoint);
  _LOG_DEBUG("  Names: ");
  int i = 1;
  std::list<ndn::Name> nl = m_npl.getNameList();
  for (std::list<ndn::Name>::iterator it = nl.begin(); it != nl.end(); it++)
  {
    _LOG_DEBUG("    Name " << i << ": " << (*it));
  }
  _LOG_DEBUG("name_lsa_end");
}

CoordinateLsa::CoordinateLsa(const ndn::Name& origR, const string lst,
                             uint32_t lsn,
                             const ndn::time::system_clock::TimePoint& lt,
                             double r, double theta)
{
  m_origRouter = origR;
  m_lsType = lst;
  m_lsSeqNo = lsn;
  m_expirationTimePoint = lt;
  m_corRad = r;
  m_corTheta = theta;
}

const ndn::Name
CoordinateLsa::getKey() const
{
  ndn::Name key = m_origRouter;
  key.append(CoordinateLsa::TYPE_STRING);
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
  corLsaData += CoordinateLsa::TYPE_STRING;
  corLsaData += "|";
  corLsaData += (boost::lexical_cast<std::string>(m_lsSeqNo) + "|");
  corLsaData += (ndn::time::toIsoString(m_expirationTimePoint) + "|");
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
  if (!(m_origRouter.size() > 0)) {
    return false;
  }
  try {
    m_lsType   = *tok_iter++;
    m_lsSeqNo  = boost::lexical_cast<uint32_t>(*tok_iter++);
    m_expirationTimePoint = ndn::time::fromIsoString(*tok_iter++);
    m_corRad   = boost::lexical_cast<double>(*tok_iter++);
    m_corTheta = boost::lexical_cast<double>(*tok_iter++);
  }
  catch (std::exception& e) {
    return false;
  }
  return true;
}

void
CoordinateLsa::writeLog()
{
  _LOG_DEBUG("Cor Lsa: ");
  _LOG_DEBUG("  Origination Router: " << m_origRouter);
  _LOG_DEBUG("  Ls Type: " << m_lsType);
  _LOG_DEBUG("  Ls Seq No: " << m_lsSeqNo);
  _LOG_DEBUG("  Ls Lifetime: " << m_expirationTimePoint);
  _LOG_DEBUG("    Hyperbolic Radius: " << m_corRad);
  _LOG_DEBUG("    Hyperbolic Theta: " << m_corTheta);
}

AdjLsa::AdjLsa(const ndn::Name& origR, const string& lst, uint32_t lsn,
               const ndn::time::system_clock::TimePoint& lt,
               uint32_t nl , AdjacencyList& adl)
{
  m_origRouter = origR;
  m_lsType = lst;
  m_lsSeqNo = lsn;
  m_expirationTimePoint = lt;
  m_noLink = nl;
  std::list<Adjacent> al = adl.getAdjList();
  for (std::list<Adjacent>::iterator it = al.begin(); it != al.end(); it++) {
    if (it->getStatus() == Adjacent::STATUS_ACTIVE) {
      addAdjacent((*it));
    }
  }
}

const ndn::Name
AdjLsa::getKey() const
{
  ndn::Name key = m_origRouter;
  key.append(AdjLsa::TYPE_STRING);
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
  adjLsaData = m_origRouter.toUri() + "|" + AdjLsa::TYPE_STRING + "|"
               + boost::lexical_cast<std::string>(m_lsSeqNo) + "|"
               + ndn::time::toIsoString(m_expirationTimePoint);
  adjLsaData += "|";
  adjLsaData += boost::lexical_cast<std::string>(m_adl.getSize());
  std::list<Adjacent> al = m_adl.getAdjList();
  for (std::list<Adjacent>::iterator it = al.begin(); it != al.end(); it++) {
    adjLsaData += "|";
    adjLsaData += (*it).getName().toUri();
    adjLsaData += "|";
    adjLsaData += (*it).getConnectingFaceUri();
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
  if (!(m_origRouter.size() > 0)) {
    return false;
  }
  try {
    m_lsType   = *tok_iter++;
    m_lsSeqNo  = boost::lexical_cast<uint32_t>(*tok_iter++);
    m_expirationTimePoint = ndn::time::fromIsoString(*tok_iter++);
    numLink    = boost::lexical_cast<uint32_t>(*tok_iter++);
  }
  catch (std::exception& e) {
    return false;
  }
  for (uint32_t i = 0; i < numLink; i++) {
    try {
      ndn::Name adjName(*tok_iter++);
      std::string connectingFaceUri(*tok_iter++);
      double linkCost = boost::lexical_cast<double>(*tok_iter++);
      Adjacent adjacent(adjName, connectingFaceUri, linkCost, Adjacent::STATUS_INACTIVE, 0, 0);
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
  if (getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix()) {
    pnlsr.getNamePrefixTable().addEntry(getOrigRouter(), getOrigRouter());
  }
}


void
AdjLsa::removeNptEntries(Nlsr& pnlsr)
{
  if (getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix()) {
    pnlsr.getNamePrefixTable().removeEntry(getOrigRouter(), getOrigRouter());
  }
}

void
AdjLsa::writeLog()
{
  _LOG_DEBUG(*this);
}

std::ostream&
operator<<(std::ostream& os, const AdjLsa& adjLsa)
{
  os << "Adj Lsa:\n"
     << "  Origination Router: " << adjLsa.getOrigRouter() << "\n"
     << "  Ls Type: " << adjLsa.getLsType() << "\n"
     << "  Ls Seq No: " << adjLsa.getLsSeqNo() << "\n"
     << "  Ls Lifetime: " << adjLsa.getExpirationTimePoint() << "\n"
     << "  Adjacents: \n";

  int adjacencyIndex = 1;

  for (const Adjacent& adjacency : adjLsa) {
  os << "    Adjacent " << adjacencyIndex++ << ":\n"
     << "      Adjacent Name: " << adjacency.getName() << "\n"
     << "      Connecting FaceUri: " << adjacency.getConnectingFaceUri() << "\n"
     << "      Link Cost: " << adjacency.getLinkCost() << "\n";
  }
  os << "adj_lsa_end";

  return os;
}

} // namespace nlsr
