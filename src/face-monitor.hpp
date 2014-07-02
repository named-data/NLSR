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
#ifndef FACE_MONITOR_HPP
#define FACE_MONITOR_HPP

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/management/nfd-face-event-notification.hpp>

namespace nlsr {

class FaceMonitor
{
public:
  typedef ndn::function<void(const ndn::nfd::FaceEventNotification&)> NotificationCallback;
  typedef ndn::function<void()> TimeoutCallback;

  FaceMonitor(boost::asio::io_service& ioService, const NotificationCallback& notificationCallBack)
    : m_face(ioService)
    , m_notificationCallBack(notificationCallBack)
  {
  }

  ~FaceMonitor()
  {
  }

  void
  startNotification();

private:
  void
  onTimeout();

  void
  onData(const ndn::Data& data);

private:
  ndn::Face m_face;
  NotificationCallback m_notificationCallBack;
  uint64_t m_lastSequence;
};

} // namespace nlsr

 #endif //FACE_MONITOR_HPP
