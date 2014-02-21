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

/*
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 
#include <map>
using boost::test_tools::output_test_stream;

#include <boost/make_shared.hpp>

#include "sync-ccnx-wrapper.h"
#include "sync-app-data-fetch.h"
#include "sync-app-data-publish.h"

using namespace Sync;
using namespace std;
using namespace boost;

class TestStructApp {
public:
  map<string, string> data;
  void set(string str1, string str2) {
    data.insert(make_pair(str1, str2));
  }

  void erase(string str1, string str2) {
    data.erase(str1);
  }

  string toString(){
    map<string, string>::iterator it = data.begin(); 
    string str = "";
    for (; it != data.end(); ++it){
      str += "<";
      str += it->first;
      str += "|";
      str += it->second;
      str += ">";
    }
    return str;
  }

};

BOOST_AUTO_TEST_CASE (AppDataPublishAndFetchTest)
{
  TestStructApp foo;
  TestStructApp bar;
	
  string interest = "/april/fool";
  string seq[5] = {"0", "1", "2", "3", "4" };
  string str[5] = {"panda", "express", "tastes", "so", "good"};

  for (int i = 0; i < 5; i++) {
    foo.set(interest + "/" + "0/" + seq[i], str[i]);
  }

  boost::function<void (string, string)> setFunc =
    bind(&TestStructApp::set, &bar, _1, _2);

  shared_ptr<CcnxWrapper> handle(new CcnxWrapper());

  AppDataFetch fetcher(handle, setFunc);
  AppDataPublish publisher(handle);

  for (int i = 1; i <= 5; i++) {
    publisher.publishData(interest, 0, str[i - 1], 5);
  }

  BOOST_CHECK_EQUAL(publisher.getNextSeq(interest, 0), 5);
  BOOST_CHECK_EQUAL(publisher.getRecentData(interest, 0), str[4]);

  fetcher.onUpdate (interest, SeqNo (0,4), SeqNo (0,-1));
  // give time for ccnd to react
  sleep(1);
  BOOST_CHECK_EQUAL(foo.toString(), bar.toString());


  boost::function<void (string, string)> eraseFunc =
    bind(&TestStructApp::erase, &bar, _1, _2);
  fetcher.setDataCallback(eraseFunc);

  fetcher.onUpdate (interest, SeqNo (0,4), SeqNo (0,-1));
  // give time for ccnd to react
  sleep(1);
  TestStructApp poo;

  BOOST_CHECK_EQUAL(poo.toString(), bar.toString());

}
*/
