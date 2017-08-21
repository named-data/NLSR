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
 **/

#include "test-common.hpp"

namespace nlsr {
namespace test {

ndn::Data&
signData(ndn::Data& data)
{
  ndn::SignatureSha256WithRsa fakeSignature;
  fakeSignature.setValue(ndn::encoding::makeEmptyBlock(ndn::tlv::SignatureValue));
  data.setSignature(fakeSignature);
  data.wireEncode();

  return data;
}

MockNfdMgmtFixture::MockNfdMgmtFixture()
  : m_face(m_ioService, m_keyChain, {true, true})
{
}

void
MockNfdMgmtFixture::signDatasetReply(ndn::Data& data)
{
  signData(data);
}

void
UnitTestTimeFixture::advanceClocks(const ndn::time::nanoseconds& tick, size_t nTicks)
{
  for (size_t i = 0; i < nTicks; ++i) {
    steadyClock->advance(tick);
    systemClock->advance(tick);

    if (m_ioService.stopped()) {
      m_ioService.reset();
    }

    m_ioService.poll();
  }
}

} // namespace test
} // namespace nlsr
