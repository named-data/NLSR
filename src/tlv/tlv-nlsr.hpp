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

#ifndef NLSR_TLV_NLSR_HPP
#define NLSR_TLV_NLSR_HPP

#include <ndn-cxx/encoding/tlv.hpp>

namespace ndn  {
namespace tlv  {
namespace nlsr {

// LSDB DataSet
enum {
  LsaInfo          = 128,
  OriginRouter     = 129,
  SequenceNumber   = 130,
  AdjacencyLsa     = 131,
  Adjacency        = 132,
  CoordinateLsa    = 133,
  Double           = 134,
  HyperbolicRadius = 135,
  HyperbolicAngle  = 136,
  NameLsa          = 137,
  LsdbStatus       = 138,
  ExpirationPeriod = 139,
  Cost             = 140,
  Uri              = 141
};

} // namespace nlsr
} // namespace tlv
} // namespace ndn

#endif // NLSR_TLV_NLSR_HPP
