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
 **/

#include "face-controller.hpp"

#include "common.hpp"
#include "logger.hpp"

namespace nlsr {
namespace util {

INIT_LOGGER("FaceController");

using ndn::util::FaceUri;

const ndn::time::seconds FaceController::TIME_ALLOWED_FOR_CANONIZATION = ndn::time::seconds(4);

void
FaceController::createFace(const std::string& request,
                           const CommandSuccessCallback& onSuccess,
                           const CommandFailureCallback& onFailure)
{
  FaceUri uri(request);

  _LOG_TRACE("Converting " << uri << " to canonical form");
  uri.canonize(bind(&FaceController::onCanonizeSuccess, this, _1, onSuccess, onFailure, uri),
               bind(&FaceController::onCanonizeFailure, this, _1, onSuccess, onFailure, uri),
               m_ioService, TIME_ALLOWED_FOR_CANONIZATION);
}

void
FaceController::createFaceInNfd(const FaceUri& uri,
                                const CommandSuccessCallback& onSuccess,
                                const CommandFailureCallback& onFailure)
{
  ndn::nfd::ControlParameters faceParameters;
  faceParameters.setUri(uri.toString());

  _LOG_DEBUG("Creating Face in NFD with face-uri: " << uri);
  m_controller.start<ndn::nfd::FaceCreateCommand>(faceParameters, onSuccess, onFailure);
}

void
FaceController::onCanonizeSuccess(const FaceUri& uri,
                                  const CommandSuccessCallback& onSuccess,
                                  const CommandFailureCallback& onFailure,
                                  const FaceUri& request)
{
  _LOG_DEBUG("Converted " << request << " to canonical form: " << uri);

  createFaceInNfd(uri, onSuccess, onFailure);
}

void
FaceController::onCanonizeFailure(const std::string& reason,
                                  const CommandSuccessCallback& onSuccess,
                                  const CommandFailureCallback& onFailure,
                                  const FaceUri& request)
{
  _LOG_WARN("Could not convert " << request << " to canonical form: " << reason);
  onFailure(CANONIZE_ERROR_CODE, "Could not canonize face-uri: " + request.toString());
}

} // namespace util
} // namespace nlsr
