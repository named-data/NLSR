/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
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
 **/

#ifndef NLSR_UTIL_FACE_CONTROLLER_HPP
#define NLSR_UTIL_FACE_CONTROLLER_HPP

#include <ndn-cxx/util/face-uri.hpp>
#include <ndn-cxx/mgmt/nfd/controller.hpp>

namespace nlsr {
namespace util {

class FaceController : boost::noncopyable
{
public:
  typedef ndn::nfd::Controller::CommandSucceedCallback CommandSuccessCallback;
  typedef ndn::nfd::Controller::CommandFailCallback CommandFailureCallback;

  FaceController(boost::asio::io_service& io, ndn::nfd::Controller& controller)
    : m_ioService(io)
    , m_controller(controller)
  {
  }

  void
  createFace(const std::string& request,
             const CommandSuccessCallback& onSuccess,
             const CommandFailureCallback& onFailure);

private:
  void
  createFaceInNfd(const ndn::util::FaceUri& uri,
                  const CommandSuccessCallback& onSuccess,
                  const CommandFailureCallback& onFailure);

  void
  onCanonizeSuccess(const ndn::util::FaceUri& uri,
                    const CommandSuccessCallback& onSuccess,
                    const CommandFailureCallback& onFailure,
                    const ndn::util::FaceUri& request);

  void
  onCanonizeFailure(const std::string& reason,
                    const CommandSuccessCallback& onSuccess,
                    const CommandFailureCallback& onFailure,
                    const ndn::util::FaceUri& request);

private:
  boost::asio::io_service& m_ioService;
  ndn::nfd::Controller& m_controller;

  static const uint32_t CANONIZE_ERROR_CODE = 408;
  static const ndn::time::seconds TIME_ALLOWED_FOR_CANONIZATION;
};

} // namespace util
} // namespace nlsr

#endif // NLSR_UTIL_FACE_CONTROLLER_HPP
