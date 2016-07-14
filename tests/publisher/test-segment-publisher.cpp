/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "publisher/segment-publisher.hpp"

#include "../boost-test.hpp"

#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>

#include <boost/mpl/int.hpp>
#include <boost/mpl/vector.hpp>

namespace nlsr {
namespace tests {

template<int64_t N=10000>
class TestSegmentPublisher : public SegmentPublisher<ndn::util::DummyClientFace>
{
public:
  TestSegmentPublisher(ndn::util::DummyClientFace& face,
                       ndn::KeyChain& keyChain,
                       const ndn::time::milliseconds freshnessPeriod)
    : SegmentPublisher(face, keyChain, freshnessPeriod)
    , m_totalPayloadLength(0)
  {

  }

  virtual
  ~TestSegmentPublisher()
  {
  }

  uint16_t
  getLimit() const
  {
    return N;
  }

  size_t
  getTotalPayloadLength() const
  {
    return m_totalPayloadLength;
  }

protected:

  virtual size_t
  generate(ndn::EncodingBuffer& outBuffer)
  {
    size_t totalLength = 0;
    for (int64_t i = 0; i < N; i++)
      {
        totalLength += prependNonNegativeIntegerBlock(outBuffer, ndn::tlv::Content, i);
      }
    m_totalPayloadLength += totalLength;
    return totalLength;
  }

protected:
  size_t m_totalPayloadLength;
};

template<int64_t N>
class SegmentPublisherFixture
{
public:
  SegmentPublisherFixture()
    : m_face(std::make_shared<ndn::util::DummyClientFace>())
    , m_expectedFreshnessPeriod(ndn::time::milliseconds(111))
    , m_publisher(*m_face, m_keyChain, m_expectedFreshnessPeriod)
    , m_publishingPrefix("/localhost/nfd/SegmentPublisherFixture")
  {
  }

  void
  validate(const ndn::Data& data)
  {
    BOOST_CHECK_EQUAL(data.getFreshnessPeriod(), m_expectedFreshnessPeriod);

    ndn::Block payload = data.getContent();

    m_buffer.appendByteArray(payload.value(), payload.value_size());

    // uint64_t segmentNo = data.getName()[-1].toSegment();
    if (data.getFinalBlockId() != data.getName()[-1])
      {
        return;
      }

    // wrap data in a single Content TLV for easy parsing
    m_buffer.prependVarNumber(m_buffer.size());
    m_buffer.prependVarNumber(ndn::tlv::Content);

    BOOST_TEST_CHECKPOINT("creating parser");
    ndn::Block parser(m_buffer.buf(), m_buffer.size());
    BOOST_TEST_CHECKPOINT("parsing aggregated response");
    parser.parse();

    BOOST_REQUIRE_EQUAL(parser.elements_size(), m_publisher.getLimit());

    uint64_t expectedNo = m_publisher.getLimit() - 1;
    for (ndn::Block::element_const_iterator i = parser.elements_begin();
         i != parser.elements_end();
         ++i)
      {
        uint64_t number = readNonNegativeInteger(*i);
        BOOST_REQUIRE_EQUAL(number, expectedNo);
        --expectedNo;
      }
  }

protected:
  ndn::shared_ptr<ndn::util::DummyClientFace> m_face;
  const ndn::time::milliseconds m_expectedFreshnessPeriod;
  TestSegmentPublisher<N> m_publisher;
  ndn::EncodingBuffer m_buffer;
  ndn::KeyChain m_keyChain;
  const ndn::Name m_publishingPrefix;
};

using boost::mpl::int_;
typedef boost::mpl::vector<int_<10000>, int_<100>, int_<10>, int_<0> > DatasetSizes;

BOOST_AUTO_TEST_SUITE(PublisherTestSegmentPublisher)

BOOST_FIXTURE_TEST_CASE_TEMPLATE(Generate, T, DatasetSizes, SegmentPublisherFixture<T::value>)
{
  this->m_publisher.publish(this->m_publishingPrefix);
  this->m_face->processEvents();

  size_t nSegments = this->m_publisher.getTotalPayloadLength() /
                     this->m_publisher.getMaxSegmentSize();
  if (this->m_publisher.getTotalPayloadLength() % this->m_publisher.getMaxSegmentSize() != 0 ||
      nSegments == 0)
    ++nSegments;

  BOOST_CHECK_EQUAL(this->m_face->sentData.size(), nSegments);
  for (const ndn::Data& data : this->m_face->sentData) {
    this->validate(data);
  }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace nlsr
