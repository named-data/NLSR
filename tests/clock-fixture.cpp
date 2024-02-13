/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
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
 */

#include "tests/clock-fixture.hpp"

namespace nlsr::tests {

ClockFixture::ClockFixture()
  : m_steadyClock(std::make_shared<time::UnitTestSteadyClock>())
  , m_systemClock(std::make_shared<time::UnitTestSystemClock>())
{
  time::setCustomClocks(m_steadyClock, m_systemClock);
}

ClockFixture::~ClockFixture()
{
  time::setCustomClocks(nullptr, nullptr);
}

void
ClockFixture::advanceClocks(time::nanoseconds tick, time::nanoseconds total)
{
  BOOST_ASSERT(tick > time::nanoseconds::zero());
  BOOST_ASSERT(total >= time::nanoseconds::zero());

  while (total > time::nanoseconds::zero()) {
    auto t = std::min(tick, total);
    m_steadyClock->advance(t);
    m_systemClock->advance(t);
    total -= t;

    afterTick();
  }
}

} // namespace nlsr::tests
