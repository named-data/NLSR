/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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

#include "sequencing-manager.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestSequencingManager)

BOOST_AUTO_TEST_CASE(SequencingManagerBasic)
{
  SequencingManager sm1(120, 121, 122);

  SequencingManager sm2(sm1.getCombinedSeqNo());

  BOOST_CHECK_EQUAL(sm2.getNameLsaSeq(), (uint32_t)120);

  BOOST_CHECK_EQUAL(sm2.getAdjLsaSeq(), (uint32_t)121);

  BOOST_CHECK_EQUAL(sm2.getCorLsaSeq(), (uint32_t)122);
}

BOOST_AUTO_TEST_CASE(BitMask)
{
  uint64_t nameLsaSeqNoMax = 0xFFFFFF;
  uint64_t corLsaSeqNoMax = 0xFFFFF;
  uint64_t adjLsaSeqNoMax = 0xFFFFF;

  uint64_t seqNo = (nameLsaSeqNoMax << 40) | (corLsaSeqNoMax << 20) | adjLsaSeqNoMax;
  SequencingManager manager(seqNo);

  BOOST_CHECK_EQUAL(manager.getNameLsaSeq(), nameLsaSeqNoMax);
  BOOST_CHECK_EQUAL(manager.getCorLsaSeq(), corLsaSeqNoMax);
  BOOST_CHECK_EQUAL(manager.getAdjLsaSeq(), adjLsaSeqNoMax);
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
