/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
 *                           Regents of the University of California
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
 *
 **/

#ifndef NLSR_TEST_COMMON_HPP
#define NLSR_TEST_COMMON_HPP

#include "common.hpp"
#include "identity-management-fixture.hpp"

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/time-unit-test-clock.hpp>

#include <ndn-cxx/security/signature-sha256-with-rsa.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>

namespace nlsr {
namespace test {

ndn::Data&
signData(ndn::Data& data);

/** \brief add a fake signature to Data
 */
inline shared_ptr<ndn::Data>
signData(shared_ptr<ndn::Data> data)
{
  signData(*data);
  return data;
}

class BaseFixture : public tests::IdentityManagementFixture
{
public:
  BaseFixture()
    : m_scheduler(m_ioService)
  {
  }

protected:
  boost::asio::io_service m_ioService;
  ndn::Scheduler m_scheduler;
};

class UnitTestTimeFixture : public BaseFixture
{
protected:
  UnitTestTimeFixture()
    : steadyClock(std::make_shared<ndn::time::UnitTestSteadyClock>())
    , systemClock(std::make_shared<ndn::time::UnitTestSystemClock>())
  {
    ndn::time::setCustomClocks(steadyClock, systemClock);
  }

  ~UnitTestTimeFixture()
  {
    ndn::time::setCustomClocks(nullptr, nullptr);
  }

  void
  advanceClocks(const ndn::time::nanoseconds& tick, size_t nTicks = 1);

protected:
  std::shared_ptr<ndn::time::UnitTestSteadyClock> steadyClock;
  std::shared_ptr<ndn::time::UnitTestSystemClock> systemClock;
};

class MockNfdMgmtFixture : public UnitTestTimeFixture
{
public:
  MockNfdMgmtFixture();

  /** \brief send one WireEncodable in reply to StatusDataset request
   *  \param prefix dataset prefix without version and segment
   *  \param payload payload block
   *  \note payload must fit in one Data
   *  \pre Interest for dataset has been expressed, sendDataset has not been invoked
   */
  template<typename T>
  void
  sendDataset(const ndn::Name& prefix, const T& payload)
  {
    BOOST_CONCEPT_ASSERT((ndn::WireEncodable<T>));

    this->sendDatasetReply(prefix, payload.wireEncode());
  }

  /** \brief send two WireEncodables in reply to StatusDataset request
   *  \param prefix dataset prefix without version and segment
   *  \param payload1 first vector item
   *  \param payload2 second vector item
   *  \note all payloads must fit in one Data
   *  \pre Interest for dataset has been expressed, sendDataset has not been invoked
   */
  template<typename T1, typename T2>
  void
  sendDataset(const ndn::Name& prefix, const T1& payload1, const T2& payload2)
  {
    BOOST_CONCEPT_ASSERT((ndn::WireEncodable<T1>));
    BOOST_CONCEPT_ASSERT((ndn::WireEncodable<T2>));

    ndn::encoding::EncodingBuffer buffer;
    payload2.wireEncode(buffer);
    payload1.wireEncode(buffer);

    this->sendDatasetReply(prefix, buffer.buf(), buffer.size());
  }

  /** \brief send a payload in reply to StatusDataset request
   *  \param name dataset prefix without version and segment
   *  \param contentArgs passed to Data::setContent
   */
  template<typename ...ContentArgs>
  void
  sendDatasetReply(ndn::Name name, ContentArgs&&... contentArgs)
  {
    name.appendVersion().appendSegment(0);

    // These warnings assist in debugging when nfdc does not receive StatusDataset.
    // They usually indicate a misspelled prefix or incorrect timing in the test case.
    if (m_face.sentInterests.empty()) {
      BOOST_WARN_MESSAGE(false, "no Interest expressed");
    }
    else {
      BOOST_WARN_MESSAGE(m_face.sentInterests.back().getName().isPrefixOf(name),
                         "last Interest " << m_face.sentInterests.back().getName() <<
                         " cannot be satisfied by this Data " << name);
    }

    auto data = make_shared<ndn::Data>(name);
    data->setFinalBlockId(name[-1]);
    data->setContent(std::forward<ContentArgs>(contentArgs)...);
    this->signDatasetReply(*data);
    m_face.receive(*data);
  }

  virtual void
  signDatasetReply(ndn::Data& data);

public:
  ndn::util::DummyClientFace m_face;
};

} // namespace test
} // namespace nlsr

#endif // NLSR_TEST_COMMON_HPP
