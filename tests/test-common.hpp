/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
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
 *
 *
 **/

#ifndef NLSR_TEST_COMMON_HPP
#define NLSR_TEST_COMMON_HPP

#include "common.hpp"

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/time-unit-test-clock.hpp>

namespace nlsr {
namespace test {

class BaseFixture
{
public:
  BaseFixture()
    : g_scheduler(g_ioService)
  {
  }

protected:
  boost::asio::io_service g_ioService;
  ndn::Scheduler g_scheduler;
};

class UnitTestTimeFixture : public BaseFixture
{
protected:
  UnitTestTimeFixture()
    : steadyClock(make_shared<ndn::time::UnitTestSteadyClock>())
    , systemClock(make_shared<ndn::time::UnitTestSystemClock>())
  {
    ndn::time::setCustomClocks(steadyClock, systemClock);
  }

  ~UnitTestTimeFixture()
  {
    ndn::time::setCustomClocks(nullptr, nullptr);
  }

  void
  advanceClocks(const ndn::time::nanoseconds& tick, size_t nTicks = 1)
  {
    for (size_t i = 0; i < nTicks; ++i) {
      steadyClock->advance(tick);
      systemClock->advance(tick);

      if (g_ioService.stopped()) {
        g_ioService.reset();
      }

      g_ioService.poll();
    }
  }

protected:
  shared_ptr<ndn::time::UnitTestSteadyClock> steadyClock;
  shared_ptr<ndn::time::UnitTestSystemClock> systemClock;
};

} // namespace test
} // namespace nlsr

#endif // NLSR_TEST_COMMON_HPP
