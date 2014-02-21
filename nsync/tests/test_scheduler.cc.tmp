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
#include <map>
using boost::test_tools::output_test_stream;

#include <boost/make_shared.hpp>
#include "sync-scheduler.h"
#include "sync-logic.h"

using namespace Sync;
using namespace std;
using namespace boost;



// void funcUpdate (const std::string &, const SeqNo &newSeq, const SeqNo &oldSeq)
// {
//   cout << "funcUpdate\n";
// }

// void funcRemove (const std::string &)
// {
//   cout << "funcRemove\n";
// }

enum SCHEDULE_LABELS
  {
    TEST_LABEL,
    ANOTHER_LABEL
  };

struct SchedulerFixture
{
  SchedulerFixture ()
    : counter (0)
  {}

  int counter;
  
  Scheduler *scheduler;

  void everySecond ()
  {
    // cout << "." << flush;
    counter ++;

    if (counter < 9)
    scheduler->schedule (boost::posix_time::milliseconds (100),
                         boost::bind (&SchedulerFixture::everySecond, this),
                         TEST_LABEL);
  }

  void setCounterFive ()
  {
    counter = 5;
  }

  void setCounterThree ()
  {
    counter = 3;
  }
};


#ifdef _DEBUG

BOOST_FIXTURE_TEST_SUITE (SchedulerTestSuite, SchedulerFixture)

BOOST_AUTO_TEST_CASE (BasicTest)
{
  BOOST_CHECK_NO_THROW (scheduler = new Scheduler ());

  scheduler->schedule (posix_time::milliseconds (100),
                       bind (&SchedulerFixture::everySecond, this),
                       TEST_LABEL);

  sleep (1);
  // cout << counter << endl;
  BOOST_CHECK_EQUAL (counter, 9); // generally, should be 9

  scheduler->schedule (posix_time::seconds (2),
		       bind (&SchedulerFixture::setCounterFive, this),
                       TEST_LABEL);

  this_thread::sleep (posix_time::milliseconds (400)); // just in case

  scheduler->schedule (posix_time::milliseconds (600),
		       bind (&SchedulerFixture::setCounterThree, this),
                       TEST_LABEL);

  this_thread::sleep (posix_time::milliseconds (500));
  BOOST_CHECK_EQUAL (counter, 9); // still 9
  
  this_thread::sleep (posix_time::milliseconds (200));
  BOOST_CHECK_EQUAL (counter, 3);

  this_thread::sleep (posix_time::milliseconds (1000));
  BOOST_CHECK_EQUAL (counter, 5);

  scheduler->schedule (posix_time::milliseconds (100),
		       bind (&SchedulerFixture::setCounterThree, this),
                       ANOTHER_LABEL);
  this_thread::sleep (posix_time::milliseconds (50));
  scheduler->cancel (ANOTHER_LABEL);
  this_thread::sleep (posix_time::milliseconds (150));
  BOOST_CHECK_EQUAL (counter, 5);
  
  BOOST_CHECK_NO_THROW (delete scheduler);
}

BOOST_AUTO_TEST_SUITE_END ()


void funcUpdate( const std::string &/*prefix*/, const SeqNo &/*newSeq*/, const SeqNo &/*oldSeq*/ )
{
}

void funcPass( const std::vector<MissingDataInfo> &v)
{
}

void funcRemove( const std::string &/*prefix*/ )
{
}

BOOST_AUTO_TEST_CASE (SyncLogicSchedulerTest)
{  
  SyncLogic *logic = 0;
  BOOST_CHECK_NO_THROW (logic = new SyncLogic ("/prefix", funcPass, funcRemove));
  this_thread::sleep (posix_time::milliseconds (100));

  Scheduler &scheduler = logic->getScheduler ();
  BOOST_CHECK_EQUAL (scheduler.getEventsSize (), 1);

  BOOST_CHECK_NO_THROW (logic->respondSyncInterest ("/prefix/e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e")); 
  BOOST_CHECK_EQUAL (scheduler.getEventsSize (), 2);

  this_thread::sleep (posix_time::milliseconds (100)); // max waiting time
  BOOST_CHECK_EQUAL (scheduler.getEventsSize (), 1);

  BOOST_CHECK_NO_THROW (logic->respondSyncInterest ("/prefix/e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e")); 
  BOOST_CHECK_NO_THROW (logic->respondSyncInterest ("/prefix/e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e")); 
  BOOST_CHECK_NO_THROW (logic->respondSyncInterest ("/prefix/e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e")); 
  BOOST_CHECK_NO_THROW (logic->respondSyncInterest ("/prefix/e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e"));
  BOOST_CHECK_EQUAL (scheduler.getEventsSize (), 5);  

  this_thread::sleep (posix_time::milliseconds (19)); // min waiting time is 20
  BOOST_CHECK_EQUAL (scheduler.getEventsSize (), 5);  

  this_thread::sleep (posix_time::milliseconds (100)); // max waiting time
  BOOST_CHECK_EQUAL (scheduler.getEventsSize (), 1);
  
  BOOST_CHECK_NO_THROW (delete logic);
}

#endif
