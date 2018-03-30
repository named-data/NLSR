/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  Regents of the University of California,
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

#ifndef NLSR_PUBLISHER_SEGMENT_PUBLISHER_HPP
#define NLSR_PUBLISHER_SEGMENT_PUBLISHER_HPP

#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/security/key-chain.hpp>

namespace nlsr {

/*! \brief provides a publisher of Status Dataset or other segmented octet stream
    \sa https://redmine.named-data.net/projects/nfd/wiki/StatusDataset
 */
template <class FaceBase>
class SegmentPublisher : boost::noncopyable
{
public:
  SegmentPublisher(FaceBase& face,
                   ndn::KeyChain& keyChain,
                   const ndn::security::SigningInfo& signingInfo,
                   const ndn::time::milliseconds& freshnessPeriod = getDefaultFreshness())
    : m_face(face)
    , m_keyChain(keyChain)
    , m_signingInfo(signingInfo)
    , m_freshnessPeriod(freshnessPeriod)
  {
  }

  virtual
  ~SegmentPublisher()
  {
  }

  /*! \brief Define the max segment size as half the max NDN packet size.
   */
  static size_t
  getMaxSegmentSize()
  {
    static const size_t MAX_SEGMENT_SIZE = ndn::MAX_NDN_PACKET_SIZE >> 1;
    return MAX_SEGMENT_SIZE;
  }

  static constexpr ndn::time::milliseconds
  getDefaultFreshness()
  {
    return ndn::time::milliseconds(1000);
  }

  /*! \brief Publish data under the provided prefix
   *
   * Processes whatever is provided from SegmentPublisher::generate,
   * by breaking it into segments of MAX_SEGMENT_SIZE and sending each
   * one individually. The last segment is distinguished by having the
   * final block ID set to a timestamp.
   */
  void
  publish(const ndn::Name& prefix)
  {
    ndn::EncodingBuffer buffer;
    generate(buffer);

    const uint8_t* rawBuffer = buffer.buf();
    const uint8_t* segmentBegin = rawBuffer;
    const uint8_t* end = rawBuffer + buffer.size();

    ndn::Name segmentPrefix(prefix);
    segmentPrefix.appendVersion();

    uint64_t segmentNo = 0;
    do {
      const uint8_t* segmentEnd = segmentBegin + getMaxSegmentSize();
      if (segmentEnd > end) {
        segmentEnd = end;
      }

      ndn::Name segmentName(segmentPrefix);
      segmentName.appendSegment(segmentNo);

      std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>(segmentName);
      data->setContent(segmentBegin, segmentEnd - segmentBegin);
      data->setFreshnessPeriod(m_freshnessPeriod);

      segmentBegin = segmentEnd;
      if (segmentBegin >= end) {
        data->setFinalBlock(segmentName[-1]);
      }

      publishSegment(data);
      ++segmentNo;
    } while (segmentBegin < end);
  }

protected:
  /*! \brief In a derived class, write the octets into outBuffer.
   */
  virtual size_t
  generate(ndn::EncodingBuffer& outBuffer) = 0;

private:
  /*! \brief Helper function to sign and put data on a Face.
   */
  void
  publishSegment(std::shared_ptr<ndn::Data>& data)
  {
    m_keyChain.sign(*data, m_signingInfo);
    m_face.put(*data);
  }

private:
  FaceBase& m_face;
  ndn::KeyChain& m_keyChain;
  const ndn::security::SigningInfo& m_signingInfo;
  const ndn::time::milliseconds m_freshnessPeriod;
};

} // namespace nlsr

#endif // NLSR_PUBLISHER_SEGMENT_PUBLISHER_HPP
