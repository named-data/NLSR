/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 
using boost::test_tools::output_test_stream;

#include "sync-digest.h"
#include <iostream>
#include <sstream>

using namespace Sync;
using namespace Sync::Error;
using namespace std;
using namespace boost;

BOOST_AUTO_TEST_SUITE(DigestTests)

BOOST_AUTO_TEST_CASE (BasicTest)
{
  Digest d0;
  BOOST_REQUIRE (d0.empty ());
}

BOOST_AUTO_TEST_CASE (DigestGenerationTest)
{
  Digest d1;
  BOOST_CHECK_NO_THROW (d1 << "1\n");

  // without explicit finalizing, Digest will not be complete and printing out will cause assert
  BOOST_CHECK (d1.empty ());

  // fix hash
  d1.finalize ();
  
  BOOST_CHECK_NO_THROW (d1.getHash ());
  BOOST_CHECK (!d1.empty ());
  BOOST_CHECK (d1 == d1);

  BOOST_CHECK_THROW (d1 << "2", DigestCalculationError);
  
  output_test_stream output;
  BOOST_CHECK_NO_THROW (output << d1);
  // BOOST_CHECK (output.check_length (40, false) );
  // BOOST_CHECK (output.is_equal ("e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e", true)); // for sha1
	BOOST_CHECK (output.check_length (64, false) );
	BOOST_CHECK (output.is_equal ("4355a46b19d348dc2f57c046f8ef63d4538ebb936000f3c9ee954a27460dd865", true)); // for sha256
}

BOOST_AUTO_TEST_CASE (DigestComparison)
{
  Digest d1;
  BOOST_CHECK_NO_THROW (d1 << "1\n");
  // BOOST_CHECK_THROW (d1 == d1, DigestCalculationError);
  BOOST_CHECK_NO_THROW (d1.finalize ());
  BOOST_CHECK (d1 == d1);
  
  Digest d2;
  BOOST_CHECK_NO_THROW (d2 << "2\n");
  BOOST_CHECK_NO_THROW (d2.finalize ());
  BOOST_CHECK (d1 != d2);
  
  Digest d3;
  // istringstream is (string ("e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e")); // real sha-1 for "1\n"
	istringstream is (string ("4355a46b19d348dc2f57c046f8ef63d4538ebb936000f3c9ee954a27460dd865")); // real sha256 for "1\n"
  BOOST_CHECK_NO_THROW (is >> d3);
  BOOST_CHECK (!d3.empty ());
  BOOST_CHECK (d3 == d1);
  BOOST_CHECK (d3 != d2);

  istringstream is2 (string ("25fa44f2b31c1fb553b6021e7360d07d5d91ff5e")); // some fake hash
  BOOST_CHECK_THROW (is2 >> d3, DigestCalculationError); // >> can be used only once

  Digest d4;
  BOOST_CHECK_THROW (is2 >> d4, DigestCalculationError); // is2 is now empty. empty >> is not allowed

  istringstream is3 (string ("25fa44f2b31c1fb553b6021e7360d07d5d91ff5e")); // some fake hash
  BOOST_CHECK_NO_THROW (is3 >> d4);
  
  BOOST_CHECK (d4 != d1);
  BOOST_CHECK (d4 != d2);
  BOOST_CHECK (d4 != d3);
}

BOOST_AUTO_TEST_SUITE_END()
