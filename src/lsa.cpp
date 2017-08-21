/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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

#include "lsa.hpp"
#include "nlsr.hpp"
#include "name-prefix-list.hpp"
#include "adjacent.hpp"
#include "logger.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <boost/algorithm/string.hpp>

namespace nlsr {

INIT_LOGGER("Lsa");

std::string
Lsa::getData() const
{
  std::ostringstream os;
  os << m_origRouter << "|" << getType() << "|" << m_lsSeqNo << "|"
     << ndn::time::toIsoString(m_expirationTimePoint) << "|";
  return os.str();
}

const ndn::Name
Lsa::getKey() const
{
  return ndn::Name(m_origRouter).append(std::to_string(getType()));
}

bool
Lsa::deserializeCommon(boost::tokenizer<boost::char_separator<char>>::iterator& iterator)
{
  m_origRouter = ndn::Name(*iterator++);
  if (m_origRouter.size() <= 0)
    return false;
  if (*iterator++ != std::to_string(getType()))
    return false;
  m_lsSeqNo = boost::lexical_cast<uint32_t>(*iterator++);
  m_expirationTimePoint = ndn::time::fromIsoString(*iterator++);
  return true;
}

NameLsa::NameLsa(const ndn::Name& origR, uint32_t lsn,
                 const ndn::time::system_clock::TimePoint& lt,
                 NamePrefixList& npl)
{
  m_origRouter = origR;
  m_lsSeqNo = lsn;
  m_expirationTimePoint = lt;
  for (const auto& name : npl.getNames()) {
    addName(name);
  }
}

std::string
NameLsa::serialize() const
{
  std::ostringstream os;
  os << getData() << m_npl.size();
  for (const auto& name : m_npl.getNames()) {
    os << "|" << name;
  }
  os << "|";
  return os.str();
}

bool
NameLsa::deserialize(const std::string& content) noexcept
{
  uint32_t numName = 0;
  boost::char_separator<char> sep("|");
  boost::tokenizer<boost::char_separator<char> >tokens(content, sep);
  boost::tokenizer<boost::char_separator<char> >::iterator tok_iter =
                                               tokens.begin();

  try {
    if (!deserializeCommon(tok_iter))
      return false;
    numName = boost::lexical_cast<uint32_t>(*tok_iter++);
    for (uint32_t i = 0; i < numName; i++) {
      ndn::Name name(*tok_iter++);
      addName(name);
    }
  }
  catch (const std::exception& e) {
    NLSR_LOG_ERROR("Could not deserialize from content: " << e.what());
    return false;
  }
  return true;
}

bool
NameLsa::isEqualContent(const NameLsa& other) const
{
  return m_npl == other.getNpl();
}

void
NameLsa::writeLog() const
{
  NLSR_LOG_DEBUG(*this);
}

CoordinateLsa::CoordinateLsa(const ndn::Name& origR, uint32_t lsn,
                             const ndn::time::system_clock::TimePoint& lt,
                             double r, std::vector<double> theta)
{
  m_origRouter = origR;
  m_lsSeqNo = lsn;
  m_expirationTimePoint = lt;
  m_corRad = r;
  m_angles = theta;
}

bool
CoordinateLsa::isEqualContent(const CoordinateLsa& clsa)
{
  if (clsa.getCorTheta().size() != m_angles.size()) {
    return false;
  }

  std::vector<double> m_angles2 = clsa.getCorTheta();
  for (unsigned int i = 0; i < clsa.getCorTheta().size(); i++) {
    if (std::abs(m_angles[i] - m_angles2[i]) > std::numeric_limits<double>::epsilon()) {
      return false;
    }
  }

  return (std::abs(m_corRad - clsa.getCorRadius()) <
          std::numeric_limits<double>::epsilon());
}

std::string
CoordinateLsa::serialize() const
{
  std::ostringstream os;
  os << getData() << m_corRad << "|" << m_angles.size() << "|";
  for (const auto& angle: m_angles) {
    os << angle << "|";
  }
  return os.str();
}

bool
CoordinateLsa::deserialize(const std::string& content) noexcept
{
  boost::char_separator<char> sep("|");
  boost::tokenizer<boost::char_separator<char> >tokens(content, sep);
  boost::tokenizer<boost::char_separator<char> >::iterator tok_iter =
                                               tokens.begin();

  try {
    if (!deserializeCommon(tok_iter))
      return false;
    m_corRad = boost::lexical_cast<double>(*tok_iter++);
    int numAngles = boost::lexical_cast<uint32_t>(*tok_iter++);
    for (int i = 0; i < numAngles; i++) {
      m_angles.push_back(boost::lexical_cast<double>(*tok_iter++));
    }
  }
  catch (const std::exception& e) {
    NLSR_LOG_ERROR("Could not deserialize from content: " << e.what());
    return false;
  }
  return true;
}

void
CoordinateLsa::writeLog() const
{
  NLSR_LOG_DEBUG(*this);
}

AdjLsa::AdjLsa(const ndn::Name& origR, uint32_t lsn,
               const ndn::time::system_clock::TimePoint& lt,
               uint32_t nl , AdjacencyList& adl)
{
  m_origRouter = origR;
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

bool
AdjLsa::isEqualContent(AdjLsa& alsa)
{
  return m_adl == alsa.getAdl();
}

std::string
AdjLsa::serialize() const
{
  std::ostringstream os;
  os << getData() << m_adl.size();
  for (const auto& adjacent : m_adl.getAdjList()) {
    os << "|" << adjacent.getName() << "|" << adjacent.getFaceUri()
       << "|" << adjacent.getLinkCost();
  }
  os << "|";
  return os.str();
}

bool
AdjLsa::deserialize(const std::string& content) noexcept
{
  uint32_t numLink = 0;
  boost::char_separator<char> sep("|");
  boost::tokenizer<boost::char_separator<char> >tokens(content, sep);
  boost::tokenizer<boost::char_separator<char> >::iterator tok_iter =
                                               tokens.begin();

  try {
    if (!deserializeCommon(tok_iter))
      return false;
    numLink = boost::lexical_cast<uint32_t>(*tok_iter++);
    for (uint32_t i = 0; i < numLink; i++) {
      ndn::Name adjName(*tok_iter++);
      std::string connectingFaceUri(*tok_iter++);
      double linkCost = boost::lexical_cast<double>(*tok_iter++);
      Adjacent adjacent(adjName, ndn::FaceUri(connectingFaceUri), linkCost,
                        Adjacent::STATUS_INACTIVE, 0, 0);
      addAdjacent(adjacent);
    }
  }
  catch (const std::exception& e) {
    NLSR_LOG_ERROR("Could not deserialize from content: " << e.what());
    return false;
  }
  return true;
}

void
AdjLsa::addNptEntries(Nlsr& pnlsr)
{
  // Only add NPT entries if this is an adj LSA from another router.
  if (getOrigRouter() != pnlsr.getConfParameter().getRouterPrefix()) {
    // Pass the originating router as both the name to register and
    // where it came from.
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
AdjLsa::writeLog() const
{
  NLSR_LOG_DEBUG(*this);
}

std::ostream&
operator<<(std::ostream& os, const AdjLsa& lsa)
{
  os << lsa.toString();
  os << "-Adjacents:";

  int adjacencyIndex = 1;

  for (const Adjacent& adjacency : lsa.m_adl) {
  os << "--Adjacent" << adjacencyIndex++ << ":\n"
     << "---Adjacent Name: " << adjacency.getName() << "\n"
     << "---Connecting FaceUri: " << adjacency.getFaceUri() << "\n"
     << "---Link Cost: " << adjacency.getLinkCost() << "\n";
  }
  os << "adj_lsa_end";

  return os;
}

std::ostream&
operator<<(std::ostream& os, const CoordinateLsa& lsa)
{
  os << lsa.toString();
  os << "--Hyperbolic Radius: " << lsa.m_corRad << "\n";
  int i = 0;
  for (const auto& value : lsa.m_angles) {
    os << "---Hyperbolic Theta: " << i++ << ": " << value << "\n";
  }
  os << "cor_lsa_end";

  return os;
}

std::ostream&
operator<<(std::ostream& os, const NameLsa& lsa)
{
  os << lsa.toString();
  os << "--Names:\n";
  int i = 0;
  auto names = lsa.m_npl.getNames();
  for (const auto& name : names) {
    os << "---Name " << i++ << ": " << name << "\n";
  }
  os << "name_lsa_end";

  return os;
}

std::ostream&
operator<<(std::ostream& os, const Lsa::Type& type)
{
  os << std::to_string(type);
  return os;
}

std::istream&
operator>>(std::istream& is, Lsa::Type& type)
{
  std::string typeString;
  is >> typeString;
  if (typeString == "ADJACENCY") {
    type = Lsa::Type::ADJACENCY;
  }
  else if (typeString == "COORDINATE") {
    type = Lsa::Type::COORDINATE;
  }
  else if (typeString == "NAME") {
    type = Lsa::Type::NAME;
  }
  else {
    type = Lsa::Type::BASE;
  }
  return is;
}

std::string
Lsa::toString() const
{
  std::ostringstream os;
  os << "LSA of type " << getType() << ":\n-Origin Router: " << getOrigRouter()
     << "\n-Sequence Number: " << getLsSeqNo() << "\n-Expiration Point: "
     << getExpirationTimePoint() << "\n";
  return os.str();
}

} // namespace nlsr

namespace std {
std::string
to_string(const nlsr::Lsa::Type& type)
{
  switch (type) {
  case nlsr::Lsa::Type::ADJACENCY:
     return "ADJACENCY";
  case nlsr::Lsa::Type::COORDINATE:
    return "COORDINATE";
  case nlsr::Lsa::Type::NAME:
    return "NAME";
  case nlsr::Lsa::Type::MOCK:
    return "MOCK";
  default:
    return "BASE";
  }
}

} // namespace std
