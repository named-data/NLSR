/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
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

#include "tlv/tlv-nlsr.hpp"

#include "../boost-test.hpp"

#include <ndn-cxx/encoding/estimator.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {
namespace tlv {
namespace test {

BOOST_AUTO_TEST_SUITE(TlvTestNlsr)

BOOST_AUTO_TEST_CASE(TestDouble)
{
  ndn::Block block = ndn::encoding::makeNonNegativeIntegerBlock(0x01, 1);
  BOOST_CHECK_THROW(ndn::tlv::nlsr::readDouble(block), ndn::tlv::Error);

  double value = 1.65;
  uint32_t type = 0x251;

  ndn::encoding::EncodingEstimator estimator;
  size_t totalLength = ndn::tlv::nlsr::prependDouble(estimator, type, value);

  ndn::encoding::EncodingBuffer encoder(totalLength, 0);
  ndn::tlv::nlsr::prependDouble(encoder, type, value);

  BOOST_CHECK_CLOSE(value, ndn::tlv::nlsr::readDouble(encoder.block()), 0.001);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace tlv
} // namespace nlsr
