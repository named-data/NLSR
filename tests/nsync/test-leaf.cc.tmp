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

#include <boost/make_shared.hpp>

#include "sync-full-leaf.h"
#include "sync-diff-leaf.h"
#include "sync-std-name-info.h"

using namespace Sync;
using namespace std;
using namespace boost;

BOOST_AUTO_TEST_SUITE(LeafTests)

BOOST_AUTO_TEST_CASE (LeafBase)
{
  NameInfoConstPtr name = StdNameInfo::FindOrCreate ("/test/name");
  BOOST_CHECK (name != 0);

  // find the same name
  BOOST_CHECK (name.get () == StdNameInfo::FindOrCreate ("/test/name").get ());
  BOOST_CHECK_EQUAL (name.use_count (), 1);

  BOOST_CHECK_NO_THROW (DiffLeaf x (name, SeqNo (12)));
  BOOST_CHECK_EQUAL (name.use_count (), 1);
  
  BOOST_CHECK_NO_THROW (DiffLeaf x (name));
  BOOST_CHECK_EQUAL (name.use_count (), 1);

  DiffLeaf updateLeaf (name, SeqNo (12));
  BOOST_CHECK_EQUAL (name.use_count (), 2);

  DiffLeaf removeLeaf (name);
  BOOST_CHECK_EQUAL (name.use_count (), 3);

  BOOST_CHECK_EQUAL (updateLeaf.getOperation (), UPDATE);
  BOOST_CHECK_EQUAL (updateLeaf.getSeq ().getSession (), 0);
  BOOST_CHECK_EQUAL (updateLeaf.getSeq ().getSeq (), 12);
  
  BOOST_CHECK_EQUAL (removeLeaf.getOperation (), REMOVE);
  BOOST_CHECK_EQUAL (removeLeaf.getSeq ().getSession (), 0);
  BOOST_CHECK_EQUAL (removeLeaf.getSeq ().getSeq (), 0);
  
  BOOST_REQUIRE_NO_THROW (FullLeaf x (name, SeqNo (12)));
  FullLeaf fullLeaf (name, SeqNo (12));
  BOOST_CHECK_EQUAL (name.use_count (), 4);
}

BOOST_AUTO_TEST_CASE (LeafDigest)
{
  BOOST_CHECK_EQUAL (StdNameInfo::FindOrCreate ("/test/name").use_count (), 1);
  NameInfoConstPtr name = StdNameInfo::FindOrCreate ("/test/name");
  FullLeaf fullLeaf (name, SeqNo (12));

  // fullLeafDigest = hash ( hash(name), hash (session, seqNo) )
  
  // Digest manualDigest;

  // Digest manualNameDigest;
  // manualNameDigest << "/test/name";
  // manualNameDigest.finalize ();

  // Digest manualSeqNoDigest;
  // manualSeqNoDigest << 0 << 13;
  // manualSeqNoDigest.finalize ();

  // manualDigest << manualNameDigest << manualSeqNoDigest;
  // manualDigest.finalize ();

  // cout << manualDigest << "\n\n";

  output_test_stream output;
  output << fullLeaf.getDigest ();
  // BOOST_CHECK (output.is_equal ("991f8cf6262dfe0f519c63f6e9b92fe69e741a9b", true)); // for sha1
	BOOST_CHECK (output.is_equal ("526d63e6e1f05f97502fd500a1729c4907f3841483ae4561b7e6307c40188f35", true)); // for sha256

  fullLeaf.setSeq (SeqNo (13));
  output << fullLeaf.getDigest ();
  BOOST_CHECK (!output.is_equal ("991f8cf6262dfe0f519c63f6e9b92fe69e741a9b", false));
  // BOOST_CHECK (output.is_equal ("585a8687ab41d5c29f86e5906c8f188ddca816b3", true)); // for sha1
	BOOST_CHECK (output.is_equal ("39fefe65b3e1021776c07d3a9a3023c6c7cdf12724ee7f3a98b813b22f46d5ec", true)); // for sha256
}

BOOST_AUTO_TEST_SUITE_END()
