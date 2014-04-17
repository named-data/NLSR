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
#include <boost/date_time/posix_time/posix_time.hpp>

#include "sync-logging.h"
#include "sync-socket.h"
#include <ndn-cpp-dev/security/validator-null.hpp>

extern "C" {
#include <unistd.h>
}

using namespace Sync;
using namespace std;
using namespace boost;

INIT_LOGGER ("Test.AppSocket");

#define PRINT 
//std::cout << "Line: " << __LINE__ << std::endl;

class TestSocketApp {
public:
  TestSocketApp()
    : sum(0)
  {}

  map<ndn::Name, string> data;
  void set(const ndn::shared_ptr<const ndn::Data>& dataPacket) {
    // _LOG_FUNCTION (this << ", " << str1);
    ndn::Name dataName(dataPacket->getName());
    string str2(reinterpret_cast<const char*>(dataPacket->getContent().value()), dataPacket->getContent().value_size());
    data.insert(make_pair(dataName, str2));
    // cout << str1 << ", " << str2 << endl;
  }

  void set(ndn::Name name, const char * buf, int len) {
    string str2(buf, len);
    data.insert(make_pair(name, str2));
  }
  
  void setNum(const ndn::shared_ptr<const ndn::Data>& dataPacket) {
    int n = dataPacket->getContent().value_size() / 4;
    uint32_t *numbers = new uint32_t [n];
    memcpy(numbers, dataPacket->getContent().value(), dataPacket->getContent().value_size());
    for (int i = 0; i < n; i++) {
      sum += numbers[i];
    }
    delete numbers;

  }

  void setNum(ndn::Name name, const char * buf, int len) {
    int n = len / 4;
    int *numbers = new int [n];
    memcpy(numbers, buf, len);
    for (int i = 0; i < n; i++) {
      sum += numbers[i];
    }
    delete numbers;
  }

  uint32_t sum;

  void fetchAll(const vector<MissingDataInfo> &v, SyncSocket *socket) {
    int n = v.size();

    PRINT

    for (int i = 0; i < n; i++) {
      for(SeqNo s = v[i].low; s <= v[i].high; ++s) {
        //PRINT
        socket->fetchData(v[i].prefix, s, bind(&TestSocketApp::set, this, _1));
      }
    }
  }

  void fetchNumbers(const vector<MissingDataInfo> &v, SyncSocket *socket) {
    int n = v.size();

    PRINT

    // std::cout << "In fetchNumbers. size of v is:  " << n << std::endl;
    for (int i = 0; i < n; i++) {
      // std::cout << "In fetchNumbers. v[i].low is (" <<v[i].low.getSession() <<", " << v[i].low.getSeq() << ") v[i].high is ("<<v[i].high.getSession() <<", " <<v[i].high.getSeq()<<")" << std::endl;
      for(SeqNo s = v[i].low; s <= v[i].high; ++s) {
        PRINT
        socket->fetchData(v[i].prefix, s, bind(&TestSocketApp::setNum, this, _1));
      }
    }
  }

  void pass(const string &prefix) {
  }

  string toString(){
    map<ndn::Name, string>::iterator it = data.begin(); 
    string str = "\n";
    for (; it != data.end(); ++it){
      str += "<";
      str += it->first.toUri();
      str += "|";
      str += it->second;
      str += ">";
      str += "\n";
    }

    return str;
  }

};

class TestSet1{
public:
  TestSet1(ndn::shared_ptr<boost::asio::io_service> ioService)
    : m_validator(new ndn::ValidatorNull())
    , m_face1(new ndn::Face(ioService))
    , m_face2(new ndn::Face(ioService))
    , m_face3(new ndn::Face(ioService))
    , m_p1("/irl.cs.ucla.edu")
    , m_p2("/yakshi.org")
    , m_p3("/google.com")
    , m_syncPrefix("/let/us/sync")
  {}

  void
  createSyncSocket1()
  {
    _LOG_DEBUG ("s1");
    m_s1 = make_shared<SyncSocket>(m_syncPrefix, m_validator, m_face1, 
                                   bind(&TestSocketApp::fetchAll, &m_a1, _1, _2), 
                                   bind(&TestSocketApp::pass, &m_a1, _1));
  }

  void
  createSyncSocket2()
  {
    _LOG_DEBUG ("s2");
    m_s2 = make_shared<SyncSocket>(m_syncPrefix, m_validator, m_face2, 
                                   bind(&TestSocketApp::fetchAll, &m_a2, _1, _2), 
                                   bind(&TestSocketApp::pass, &m_a2, _1));
  }
  
  void
  createSyncSocket3()
  {
    _LOG_DEBUG ("s3");
    m_s3 = make_shared<SyncSocket>(m_syncPrefix, m_validator, m_face3, 
                                   bind(&TestSocketApp::fetchAll, &m_a3, _1, _2), 
                                   bind(&TestSocketApp::pass, &m_a3, _1));
  }

  void
  publishSocket1(uint32_t session, string data)
  {
    _LOG_DEBUG ("s1 publish");
    m_s1->publishData (m_p1, session, data.c_str(), data.size(), 1000); 
  }

  void
  publishSocket2(uint32_t session, string data)
  {
    _LOG_DEBUG ("s2 publish");
    m_s2->publishData (m_p2, session, data.c_str(), data.size(), 1000); 
  }

  void
  publishSocket3(uint32_t session, string data)
  {
    _LOG_DEBUG ("s3 publish");
    m_s3->publishData (m_p3, session, data.c_str(), data.size(), 1000); 
  }

  void
  setSocket1(string suffix, string data)
  {
    _LOG_DEBUG ("a1 set");
    ndn::Name name = m_p1;
    name.append(suffix);
    m_a1.set (name, data.c_str(), data.size()); 
  }

  void
  setSocket2(string suffix, string data)
  {
    _LOG_DEBUG ("a2 set");
    ndn::Name name = m_p2;
    name.append(suffix);
    m_a2.set (name, data.c_str(), data.size()); 
  }

  void
  setSocket3(string suffix, string data)
  {
    _LOG_DEBUG ("a3 set");
    ndn::Name name = m_p3;
    name.append(suffix);
    m_a3.set (name, data.c_str(), data.size()); 
  }

  void
  check(int round)
  { 
    BOOST_CHECK_EQUAL(m_a1.toString(), m_a2.toString());
    BOOST_CHECK_EQUAL(m_a2.toString(), m_a3.toString());
  }

  void
  done()
  {
    m_s1.reset();
    m_s2.reset();
    m_s3.reset();
  }


  TestSocketApp m_a1, m_a2, m_a3;
  ndn::shared_ptr<ndn::ValidatorNull> m_validator;
  ndn::shared_ptr<ndn::Face> m_face1, m_face2, m_face3;
  ndn::Name m_p1, m_p2, m_p3;
  ndn::shared_ptr<SyncSocket> m_s1, m_s2, m_s3;
  ndn::Name m_syncPrefix;
};

class TestSet2{
public:
  TestSet2(ndn::shared_ptr<boost::asio::io_service> ioService)
    : m_validator(new ndn::ValidatorNull())
    , m_face1(new ndn::Face(ioService))
    , m_face2(new ndn::Face(ioService))
    , m_p1("/xiaonei.com")
    , m_p2("/mitbbs.com")
    , m_syncPrefix("/this/is/the/prefix")
  {}

  void
  createSyncSocket1()
  {
    _LOG_DEBUG ("s1");
    m_s1 = make_shared<SyncSocket>(m_syncPrefix, m_validator, m_face1, 
                                   bind(&TestSocketApp::fetchNumbers, &m_a1, _1, _2), 
                                   bind(&TestSocketApp::pass, &m_a1, _1));
  }

  void
  createSyncSocket2()
  {
    _LOG_DEBUG ("s2");
    m_s2 = make_shared<SyncSocket>(m_syncPrefix, m_validator, m_face2, 
                                   bind(&TestSocketApp::fetchNumbers, &m_a2, _1, _2), 
                                   bind(&TestSocketApp::pass, &m_a2, _1));
  }
  
  void
  publishSocket1(uint32_t session, string data)
  {
    _LOG_DEBUG ("s1 publish");
    m_s1->publishData (m_p1, session, data.c_str(), data.size(), 1000); 
  }

  void
  publishSocket2(uint32_t session, string data)
  {
    _LOG_DEBUG ("s2 publish");
    m_s2->publishData (m_p2, session, data.c_str(), data.size(), 1000); 
  }

  void
  setSocket1(const char* ptr, size_t size)
  {
    _LOG_DEBUG ("a1 setNum");
    m_a1.setNum (m_p1, ptr, size); 
  }

  void
  setSocket2(const char* ptr, size_t size)
  {
    _LOG_DEBUG ("a2 setNum");
    m_a2.setNum (m_p2, ptr, size); 
  }

  void
  check(int num)
  { 
    _LOG_DEBUG ("codnum " << num);
    _LOG_DEBUG ("a1 sum " << m_a1.sum);
    _LOG_DEBUG ("a2 sum " << m_a2.sum);

    BOOST_CHECK(m_a1.sum == m_a2.sum && m_a1.sum == num);
  }

  void
  done()
  {
    m_s1.reset();
    m_s2.reset();
  }

  TestSocketApp m_a1, m_a2;
  ndn::shared_ptr<ndn::ValidatorNull> m_validator;
  ndn::shared_ptr<ndn::Face> m_face1, m_face2;
  ndn::Name m_p1, m_p2;
  ndn::shared_ptr<SyncSocket> m_s1, m_s2;
  ndn::Name m_syncPrefix;
};

BOOST_AUTO_TEST_CASE (AppSocketTest1)
{
  INIT_LOGGERS ();

  ndn::shared_ptr<boost::asio::io_service> ioService = ndn::make_shared<boost::asio::io_service>();
  ndn::Scheduler scheduler(*ioService);
  TestSet1 testSet1(ioService);

  scheduler.scheduleEvent(ndn::time::seconds(0.00), ndn::bind(&TestSet1::createSyncSocket1, &testSet1));
  scheduler.scheduleEvent(ndn::time::seconds(0.05), ndn::bind(&TestSet1::createSyncSocket2, &testSet1));
  scheduler.scheduleEvent(ndn::time::seconds(0.10), ndn::bind(&TestSet1::createSyncSocket3, &testSet1));
  string data0 = "Very funny Scotty, now beam down my clothes";
  scheduler.scheduleEvent(ndn::time::seconds(0.15), ndn::bind(&TestSet1::publishSocket1, &testSet1, 0, data0));
  scheduler.scheduleEvent(ndn::time::seconds(1.15), ndn::bind(&TestSet1::setSocket1, &testSet1, "/0/0", data0));
  scheduler.scheduleEvent(ndn::time::seconds(1.16), ndn::bind(&TestSet1::check, &testSet1, 1)); 
  string data1 = "Yes, give me that ketchup";
  string data2 = "Don't look conspicuous, it draws fire";
  scheduler.scheduleEvent(ndn::time::seconds(1.17), ndn::bind(&TestSet1::publishSocket1, &testSet1, 0, data1));
  scheduler.scheduleEvent(ndn::time::seconds(1.18), ndn::bind(&TestSet1::publishSocket1, &testSet1, 0, data2));
  scheduler.scheduleEvent(ndn::time::seconds(2.15), ndn::bind(&TestSet1::setSocket1, &testSet1, "/0/1", data1));
  scheduler.scheduleEvent(ndn::time::seconds(2.16), ndn::bind(&TestSet1::setSocket1, &testSet1, "/0/2", data2));
  scheduler.scheduleEvent(ndn::time::seconds(2.17), ndn::bind(&TestSet1::check, &testSet1, 2));
  string data3 = "You surf the Internet, I surf the real world";
  string data4 = "I got a fortune cookie once that said 'You like Chinese food'";
  string data5 = "Real men wear pink. Why? Because their wives make them";
  scheduler.scheduleEvent(ndn::time::seconds(3.18), ndn::bind(&TestSet1::publishSocket3, &testSet1, 0, data3));
  scheduler.scheduleEvent(ndn::time::seconds(3.20), ndn::bind(&TestSet1::publishSocket2, &testSet1, 0, data4));
  scheduler.scheduleEvent(ndn::time::seconds(3.21), ndn::bind(&TestSet1::publishSocket2, &testSet1, 0, data5));
  scheduler.scheduleEvent(ndn::time::seconds(4.21), ndn::bind(&TestSet1::setSocket3, &testSet1, "/0/0", data3));
  scheduler.scheduleEvent(ndn::time::seconds(4.22), ndn::bind(&TestSet1::setSocket2, &testSet1, "/0/0", data4));
  scheduler.scheduleEvent(ndn::time::seconds(4.23), ndn::bind(&TestSet1::setSocket2, &testSet1, "/0/1", data5));
  scheduler.scheduleEvent(ndn::time::seconds(4.30), ndn::bind(&TestSet1::check, &testSet1, 3));
  // not sure weither this is simultanous data generation from multiple sources
  _LOG_DEBUG ("Simultaneous publishing");
  string data6 = "Shakespeare says: 'Prose before hos.'";
  string data7 = "Pick good people, talent never wears out";
  scheduler.scheduleEvent(ndn::time::seconds(5.50), ndn::bind(&TestSet1::publishSocket1, &testSet1, 0, data6));
  scheduler.scheduleEvent(ndn::time::seconds(5.50), ndn::bind(&TestSet1::publishSocket2, &testSet1, 0, data7));
  scheduler.scheduleEvent(ndn::time::seconds(6.80), ndn::bind(&TestSet1::setSocket1, &testSet1, "/0/3", data6));
  scheduler.scheduleEvent(ndn::time::seconds(6.80), ndn::bind(&TestSet1::setSocket2, &testSet1, "/0/2", data7));
  scheduler.scheduleEvent(ndn::time::seconds(6.90), ndn::bind(&TestSet1::check, &testSet1, 4));
  scheduler.scheduleEvent(ndn::time::seconds(7.00), ndn::bind(&TestSet1::done, &testSet1));

  ioService->run();
}

BOOST_AUTO_TEST_CASE (AppSocketTest2)
{
  ndn::shared_ptr<boost::asio::io_service> ioService = ndn::make_shared<boost::asio::io_service>();
  ndn::Scheduler scheduler(*ioService);
  TestSet2 testSet2(ioService);

  scheduler.scheduleEvent(ndn::time::seconds(0.00), ndn::bind(&TestSet2::createSyncSocket1, &testSet2));
  scheduler.scheduleEvent(ndn::time::seconds(0.05), ndn::bind(&TestSet2::createSyncSocket2, &testSet2));
  uint32_t num[5] = {0, 1, 2, 3, 4};
  string data0((const char *) num, sizeof(num));
  scheduler.scheduleEvent(ndn::time::seconds(0.10), ndn::bind(&TestSet2::publishSocket1, &testSet2, 0, data0));
  scheduler.scheduleEvent(ndn::time::seconds(0.15), ndn::bind(&TestSet2::setSocket1, &testSet2, (const char *) num, sizeof (num)));
  scheduler.scheduleEvent(ndn::time::seconds(1.00), ndn::bind(&TestSet2::check, &testSet2, 10));
  uint32_t newNum[5] = {9, 7, 2, 1, 1};
  string data1((const char *) newNum, sizeof(newNum));
  scheduler.scheduleEvent(ndn::time::seconds(1.10), ndn::bind(&TestSet2::publishSocket2, &testSet2, 0, data1));
  scheduler.scheduleEvent(ndn::time::seconds(1.15), ndn::bind(&TestSet2::setSocket2, &testSet2, (const char *) newNum, sizeof (newNum)));
  scheduler.scheduleEvent(ndn::time::seconds(2.00), ndn::bind(&TestSet2::check, &testSet2, 30));
  scheduler.scheduleEvent(ndn::time::seconds(7.00), ndn::bind(&TestSet2::done, &testSet2));

  ioService->run();
}
