/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  The University of Memphis,
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
 */

#ifndef NLSR_LSA_HPP
#define NLSR_LSA_HPP

#include "name-prefix-list.hpp"
#include "adjacent.hpp"
#include "adjacency-list.hpp"

#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/time.hpp>
#include <boost/tokenizer.hpp>

namespace nlsr {

class Lsa
{
public:
  enum class Type {
    ADJACENCY,
    COORDINATE,
    NAME,
    BASE,
    MOCK
  };

  virtual
  ~Lsa() = default;

  virtual Type
  getType() const
  {
    return Type::BASE;
  }

  void
  setLsSeqNo(uint32_t lsn)
  {
    m_lsSeqNo = lsn;
  }

  uint32_t
  getLsSeqNo() const
  {
    return m_lsSeqNo;
  }

  const ndn::Name&
  getOrigRouter() const
  {
    return m_origRouter;
  }

  void
  setOrigRouter(const ndn::Name& org)
  {
    m_origRouter = org;
  }

  const ndn::time::system_clock::TimePoint&
  getExpirationTimePoint() const
  {
    return m_expirationTimePoint;
  }

  void
  setExpirationTimePoint(const ndn::time::system_clock::TimePoint& lt)
  {
    m_expirationTimePoint = lt;
  }

  void
  setExpiringEventId(ndn::scheduler::EventId eid)
  {
    m_expiringEventId = std::move(eid);
  }

  ndn::scheduler::EventId
  getExpiringEventId() const
  {
    return m_expiringEventId;
  }

  /*! \brief Return the data that this LSA represents.
   */
  virtual std::string
  serialize() const = 0;

  /*! \brief Gets the key for this LSA.

    Format is: \<router name\>/\<LSA type>\
   */
  const ndn::Name
  getKey() const;

  /*! \brief Populate this LSA with content from the string "content".
    \param content The string containing a valid serialization of LSA content.

    This method populates "this" LSA with data from the string.
   */
  virtual bool
  deserialize(const std::string& content) noexcept = 0;

  virtual void
  writeLog() const = 0;

protected:
  /*! Get data common to all LSA types.

    This method should be called by all LSA classes in their
    serialize() method.
   */
  std::string
  getData() const;

  /*! Print data common to all LSA types.
   */
  std::string
  toString() const;

  bool
  deserializeCommon(boost::tokenizer<boost::char_separator<char>>::iterator& iterator);

protected:
  ndn::Name m_origRouter;
  uint32_t m_lsSeqNo = 0;
  ndn::time::system_clock::TimePoint m_expirationTimePoint;
  ndn::scheduler::EventId m_expiringEventId;
};

class NameLsa : public Lsa
{
public:
  NameLsa() = default;

  NameLsa(const ndn::Name& origR, uint32_t lsn,
          const ndn::time::system_clock::TimePoint& lt,
          NamePrefixList& npl);

  Lsa::Type
  getType() const override
  {
    return Lsa::Type::NAME;
  }

  NamePrefixList&
  getNpl()
  {
    return m_npl;
  }

  const NamePrefixList&
  getNpl() const
  {
    return m_npl;
  }

  void
  addName(const ndn::Name& name)
  {
    m_npl.insert(name);
  }

  void
  removeName(const ndn::Name& name)
  {
    m_npl.remove(name);
  }

  /*! \brief Initializes this LSA object with content's data.

    \param content The data (e.g. name prefixes) to initialize this LSA with.

    This function initializes this object to represent the data
    contained in content. The format for this is the same as for
    getData(); getData() returns data of this format, in other words.
   */
  bool
  deserialize(const std::string& content) noexcept override;

  bool
  isEqualContent(const NameLsa& other) const;

  void
  writeLog() const override;

  /*! \brief Returns the data that this name LSA has.

    Format is: \<original router
    prefix\>|name|\<seq. no.\>|\<exp. time\>|\<prefix 1\>|\<prefix
    2\>|...|\<prefix n\>|
   */
  std::string
  serialize() const override;

private:
  NamePrefixList m_npl;

  friend std::ostream&
  operator<<(std::ostream& os, const NameLsa& lsa);
};

class AdjLsa : public Lsa
{
public:
  typedef AdjacencyList::const_iterator const_iterator;

  AdjLsa() = default;

  AdjLsa(const ndn::Name& origR, uint32_t lsn,
         const ndn::time::system_clock::TimePoint& lt,
         uint32_t nl , AdjacencyList& adl);

  Lsa::Type
  getType() const override
  {
    return Lsa::Type::ADJACENCY;
  }

  AdjacencyList&
  getAdl()
  {
    return m_adl;
  }

  const AdjacencyList&
  getAdl() const
  {
    return m_adl;
  }

  void
  addAdjacent(Adjacent adj)
  {
    m_adl.insert(adj);
  }

  /*! \brief Initializes this adj. LSA from the supplied content.

    \param content The content that this LSA is to have, formatted
    according to getData().
   */
  bool
  deserialize(const std::string& content) noexcept override;

  uint32_t
  getNoLink()
  {
    return m_noLink;
  }

  bool
  isEqualContent(const AdjLsa& alsa) const;

  void
  writeLog() const override;

  const_iterator
  begin() const
  {
    return m_adl.begin();
  }

  const_iterator
  end() const
  {
    return m_adl.end();
  }

  /*! \brief Returns the data this adjacency LSA has.

    The format is: \<original
    router\>|adjacency|\<seq. no.\>|\<exp. time\>|\<size\>|\<adjacency prefix
    1\>|\<face uri 1\>|\<cost 1\>|...|\<adjacency prefix n\>|\<face uri
    n\>|\<cost n\>|
   */
  std::string
  serialize() const override;

private:
  uint32_t m_noLink;
  AdjacencyList m_adl;

  friend std::ostream&
  operator<<(std::ostream& os, const AdjLsa& lsa);
};

class CoordinateLsa : public Lsa
{
public:
  CoordinateLsa() = default;

  CoordinateLsa(const ndn::Name& origR, uint32_t lsn,
                const ndn::time::system_clock::TimePoint& lt,
                double r, std::vector<double> theta);

  Lsa::Type
  getType() const override
  {
    return Lsa::Type::COORDINATE;
  }

  /*! \brief Initializes this coordinate LSA with the data in content.

    \param content The string content that is used to build the LSA.

    This function initializes this LSA object to represent the data
    specified by the parameter. The format that it is expecting is the
    same as for getData();
  */
  bool
  deserialize(const std::string& content) noexcept override;

  double
  getCorRadius() const
  {
    return m_corRad;
  }

  void
  setCorRadius(double cr)
  {
    m_corRad = cr;
  }

  const std::vector<double>
  getCorTheta() const
  {
    return m_angles;
  }

  void
  setCorTheta(std::vector<double> ct)
  {
    m_angles = ct;
  }

  bool
  isEqualContent(const CoordinateLsa& clsa) const;

  void
  writeLog() const override;

  /*! \brief Returns the data that this coordinate LSA represents.

    The format is: \<original
    router\>|coordinate|\<seq. no.\>|\<exp. time\>|\<radians\>|\<theta\>|
  */
  std::string
  serialize() const override;

private:
  double m_corRad = 0.0;
  std::vector<double> m_angles;

  friend std::ostream&
  operator<<(std::ostream& os, const CoordinateLsa& lsa);
};

std::ostream&
operator<<(std::ostream& os, const AdjLsa& lsa);

std::ostream&
operator<<(std::ostream& os, const CoordinateLsa& lsa);

std::ostream&
operator<<(std::ostream& os, const NameLsa& lsa);

std::ostream&
operator<<(std::ostream& os, const Lsa::Type& type);

std::istream&
operator>>(std::istream& is, Lsa::Type& type);

} // namespace nlsr

namespace std {
std::string
to_string(const nlsr::Lsa::Type& type);
} // namespace std

#endif // NLSR_LSA_HPP
