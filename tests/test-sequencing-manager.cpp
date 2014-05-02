/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

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

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
