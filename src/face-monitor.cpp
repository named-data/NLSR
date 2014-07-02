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
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#include "face-monitor.hpp"
#include "logger.hpp"


namespace nlsr {

INIT_LOGGER("FaceMonitor");

void
FaceMonitor::startNotification()
{
  ndn::Interest interest("/localhost/nfd/faces/events");
  interest
    .setMustBeFresh(true)
    .setChildSelector(1)
    .setInterestLifetime(ndn::time::seconds(60));

  _LOG_DEBUG("FaceMonitor::startNotification sending interest: " << interest);

  m_face.expressInterest(interest,
                         ndn::bind(&nlsr::FaceMonitor::onData, this, _2),
                         ndn::bind(&nlsr::FaceMonitor::onTimeout, this));
}

void
FaceMonitor::onTimeout()
{
  ndn::Interest interest("/localhost/nfd/faces/events");
  interest
    .setMustBeFresh(true)
    .setChildSelector(1)
    .setInterestLifetime(ndn::time::seconds(60));

  _LOG_DEBUG("FaceMonitor::onTimeout sending interest: " << interest);

  m_face.expressInterest(interest,
                         ndn::bind(&nlsr::FaceMonitor::onData, this, _2),
                         ndn::bind(&nlsr::FaceMonitor::onTimeout, this));
}
void
FaceMonitor::onData(const ndn::Data& data)
{
  m_lastSequence = data.getName().get(-1).toSegment();
  ndn::nfd::FaceEventNotification notification(data.getContent().blockFromValue());
  m_notificationCallBack(notification);

  ndn::Name nextNotification("/localhost/nfd/faces/events");
  nextNotification.appendSegment(m_lastSequence + 1);

  ndn::Interest interest(nextNotification);
  interest.setInterestLifetime(ndn::time::seconds(60));

  _LOG_DEBUG("FaceMonitor::onData Interest sent: " <<  interest);

  m_face.expressInterest(interest,
                         ndn::bind(&nlsr::FaceMonitor::onData, this, _2),
                         ndn::bind(&nlsr::FaceMonitor::onTimeout, this));
}


} // namespace nlsr
