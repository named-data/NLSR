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

#define BOOST_TEST_DYN_LINK 1
#define BOOST_TEST_NO_MAIN 1
// #define BOOST_TEST_MODULE StateTests
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 
using boost::test_tools::output_test_stream;

#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "sync-std-name-info.h"
#include "sync-full-state.h"
#include "sync-diff-state.h"

using namespace Sync;
using namespace std;
using namespace boost;

BOOST_AUTO_TEST_SUITE(StateTests)

BOOST_AUTO_TEST_CASE (FullStateTest)
{
  BOOST_CHECK_NO_THROW (FullState ());
  FullState state;
  BOOST_CHECK_EQUAL (state.getLeaves ().size (), 0);

  output_test_stream output;
  output << state.getTimeFromLastUpdate ();
  BOOST_CHECK (output.is_equal ("not-a-date-time", true));

  NameInfoConstPtr name = StdNameInfo::FindOrCreate ("/test/name");
  BOOST_CHECK_NO_THROW (state.update (name, SeqNo (12)));
  BOOST_CHECK_NO_THROW (state.update (name, SeqNo (12)));
  BOOST_CHECK_NO_THROW (state.update (name, SeqNo (12)));
  BOOST_CHECK_EQUAL (state.getLeaves ().size (), 1);
  BOOST_CHECK_EQUAL ((*state.getLeaves ().begin ())->getSeq ().getSeq (), 12);

  BOOST_CHECK_NO_THROW (state.update (name, SeqNo (13)));
  BOOST_CHECK_EQUAL ((*state.getLeaves ().begin ())->getSeq ().getSeq (), 13);

  BOOST_CHECK_NO_THROW (state.remove (name));
  BOOST_CHECK_EQUAL (state.getLeaves ().size (), 0);

  BOOST_CHECK_EQUAL (state.getTimeFromLastUpdate ().total_milliseconds (), 0);
}

BOOST_AUTO_TEST_CASE (DiffStateTest)
{
  BOOST_CHECK_NO_THROW (DiffState ());
  DiffState state;
  BOOST_CHECK_EQUAL (state.getLeaves ().size (), 0);

  NameInfoConstPtr name = StdNameInfo::FindOrCreate ("/test/name");
  BOOST_CHECK_NO_THROW (state.update (name, SeqNo (12)));
  BOOST_CHECK_NO_THROW (state.update (name, SeqNo (12)));
  BOOST_CHECK_NO_THROW (state.update (name, SeqNo (12)));
  BOOST_CHECK_EQUAL (state.getLeaves ().size (), 1);
  BOOST_CHECK_EQUAL ((*state.getLeaves ().begin ())->getSeq ().getSeq (), 12);

  BOOST_CHECK_NO_THROW (state.update (name, SeqNo (13)));
  BOOST_CHECK_EQUAL ((*state.getLeaves ().begin ())->getSeq ().getSeq (), 13);

  BOOST_CHECK_NO_THROW (state.remove (name));
  BOOST_CHECK_EQUAL (state.getLeaves ().size (), 1);
  BOOST_CHECK_EQUAL ((*state.getLeaves ().begin ())->getSeq ().getSeq (), 0);
}

BOOST_AUTO_TEST_CASE (FullStateDigestTest)
{
  FullState state;
  BOOST_CHECK_EQUAL (state.getLeaves ().size (), 0);

  NameInfoConstPtr name3 = StdNameInfo::FindOrCreate ("3");
  NameInfoConstPtr name2 = StdNameInfo::FindOrCreate ("2");
  NameInfoConstPtr name1 = StdNameInfo::FindOrCreate ("1");

  state.update (name1, SeqNo (10));
  DigestConstPtr digest1 = state.getDigest ();

  state.update (name2, SeqNo (12));
  DigestConstPtr digest2 = state.getDigest ();

  BOOST_CHECK (digest1.get () != digest2.get ());
  BOOST_CHECK (!digest1->empty ());
  BOOST_CHECK (!digest2->empty ());

  state.update (name3, SeqNo (8));
  DigestConstPtr digest3 = state.getDigest ();

  BOOST_CHECK (digest1.get () != digest2.get ());
  BOOST_CHECK (digest2.get () != digest3.get ());
  BOOST_CHECK (digest1.get () != digest3.get ());

  BOOST_CHECK (*digest1 != *digest2);
  BOOST_CHECK (*digest2 != *digest3);
  BOOST_CHECK (*digest1 != *digest3);

  // removing elements. Digest should get reverted to digest1
  state.remove (name2);
  state.remove (name3);
  DigestConstPtr digest4 = state.getDigest ();
  BOOST_CHECK (*digest1 == *digest4);

  name2.reset (); // force destructor
  name3.reset (); // force destructor
  name3 = StdNameInfo::FindOrCreate ("3"); // this will enforce different (larger) hashing ID of name
  name2 = StdNameInfo::FindOrCreate ("2"); // this will enforce different (larger) hashing ID of name
  
  // adding in different order
  state.update (name3, SeqNo (8));
  state.update (name2, SeqNo (12));
  DigestConstPtr digest5 = state.getDigest ();
  BOOST_CHECK (*digest5 == *digest3);
}

BOOST_AUTO_TEST_CASE (FullStateXml)
{
  FullState state;

  NameInfoConstPtr name3 = StdNameInfo::FindOrCreate ("3");
  NameInfoConstPtr name2 = StdNameInfo::FindOrCreate ("2");
  NameInfoConstPtr name1 = StdNameInfo::FindOrCreate ("1");

  state.update (name1, SeqNo (10));
  state.update (name2, SeqNo (12));
  state.update (name3, SeqNo (8));  

  string xml1 = "<state>"
    "<item><name>1</name><seq><session>0</session><seqno>10</seqno></seq></item>"
    "<item><name>2</name><seq><session>0</session><seqno>12</seqno></seq></item>"
    "<item><name>3</name><seq><session>0</session><seqno>8</seqno></seq></item>"
    "</state>";
  {
  ostringstream os;
  os << state;
  string s = os.str ();
  // cout << s << endl; 
  erase_all (s, "\n");
  BOOST_CHECK_EQUAL (s, xml1);
  }
  
  state.remove (name2);
  string xml2 = "<state>"
    "<item><name>1</name><seq><session>0</session><seqno>10</seqno></seq></item>"
    "<item><name>3</name><seq><session>0</session><seqno>8</seqno></seq></item>"
    "</state>";
  {
  ostringstream os;
  os << state;
  string s = os.str ();
  erase_all (s, "\n");
  BOOST_CHECK_EQUAL (s, xml2);
  }

  FullState state2;
  istringstream xml1_is (xml1);
  BOOST_CHECK_NO_THROW (xml1_is >> state2);
  {
  ostringstream os;
  os << state2;
  string xml1_test = os.str ();
  erase_all (xml1_test, "\n");
  BOOST_CHECK_EQUAL (xml1_test, xml1);
  }
  
  istringstream xml2_is ("<state><item action=\"remove\"><name>2</name></item></state>");
  BOOST_CHECK_NO_THROW (xml2_is >> state2);
  
  {
  ostringstream os;
  os << state2;
  string xml2_test = os.str ();
  erase_all (xml2_test, "\n");
  BOOST_CHECK_EQUAL (xml2_test, xml2);
  }
}

BOOST_AUTO_TEST_CASE (DiffStateXml)
{
  DiffState state;

  NameInfoConstPtr name3 = StdNameInfo::FindOrCreate ("3");
  NameInfoConstPtr name2 = StdNameInfo::FindOrCreate ("2");
  NameInfoConstPtr name1 = StdNameInfo::FindOrCreate ("1");

  state.update (name1, SeqNo (10));
  state.update (name2, SeqNo (12));
  state.update (name3, SeqNo (8));  

  string xml1 = "<state>"
    "<item action=\"update\"><name>1</name><seq><session>0</session><seqno>10</seqno></seq></item>"
    "<item action=\"update\"><name>2</name><seq><session>0</session><seqno>12</seqno></seq></item>"
    "<item action=\"update\"><name>3</name><seq><session>0</session><seqno>8</seqno></seq></item>"
    "</state>";
  {
  ostringstream os;
  os << state;
  string xml1_test = os.str ();
  erase_all (xml1_test, "\n");
  BOOST_CHECK_EQUAL (xml1_test, xml1);
  }
  
  state.remove (name2);
  string xml2 = "<state>"
    "<item action=\"update\"><name>1</name><seq><session>0</session><seqno>10</seqno></seq></item>"
    "<item action=\"remove\"><name>2</name></item>"
    "<item action=\"update\"><name>3</name><seq><session>0</session><seqno>8</seqno></seq></item>"
    "</state>";
  {
  ostringstream os;
  os << state;
  string xml2_test = os.str ();
  erase_all (xml2_test, "\n");
  BOOST_CHECK_EQUAL (xml2_test, xml2);
  }

  ////////////  ////////////  ////////////  ////////////  ////////////  ////////////
  
  DiffState state2;
  istringstream xml1_is (xml1);
  BOOST_CHECK_NO_THROW (xml1_is >> state2);
  
  {
  ostringstream os;
  os << state2;
  string xml1_test = os.str ();
  erase_all (xml1_test, "\n");
  BOOST_CHECK_EQUAL (xml1_test, xml1);
  }

  istringstream xml2_is ("<state><item action=\"remove\"><name>2</name></item></state>");
  BOOST_CHECK_NO_THROW (xml2_is >> state2);
  
  {
  ostringstream os;
  os << state2;
  string xml2_test = os.str ();
  erase_all (xml2_test, "\n");
  BOOST_CHECK_EQUAL (xml2_test, xml2);
  }

}

BOOST_AUTO_TEST_CASE (DiffStateDiffTest)
{
  DiffStatePtr root = make_shared<DiffState> ();

  DiffStatePtr head = make_shared<DiffState> ();
  root->setNext (head);
  
  head->update (StdNameInfo::FindOrCreate ("3"), SeqNo (1));
  head->remove (StdNameInfo::FindOrCreate ("1"));
  
  DiffStatePtr tail = make_shared<DiffState> ();
  head->setNext (tail);

  tail->update (StdNameInfo::FindOrCreate ("3"), SeqNo (2));  

  {
  ostringstream os;
  os << *root->diff ();
  string diffState = os.str ();
  erase_all (diffState, "\n");
  BOOST_CHECK_EQUAL (diffState,
                     "<state>"
                     "<item action=\"remove\"><name>1</name></item>"
                     "<item action=\"update\"><name>3</name><seq><session>0</session><seqno>2</seqno></seq></item>"
                     "</state>");
  }
}

BOOST_AUTO_TEST_SUITE_END()
